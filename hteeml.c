#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <sysexits.h>
#include <err.h>

#define MIN(a,b) ((a)<(b)?(a):(b))

static int outpipe[2], errpipe[2];
static int nclosed;

static void
print_html(char *data, size_t len, FILE *f)
{
	static char buf[4096];
	size_t i, buflen=0;

	for (i=0; i < len; i++) {
		if (buflen+5 >= sizeof(buf)) {
			fwrite(buf, 1, buflen, f);
			buflen = 0;
		}
		switch (data[i]) {
		case '&':
			memcpy(&buf[buflen], "&amp;", 5);
			buflen += 5;
			break;
		case '<':
			memcpy(&buf[buflen], "&lt;", 4);
			buflen += 4;
			break;
		case '>':
			memcpy(&buf[buflen], "&gt;", 4);
			buflen += 4;
			break;
		default:
			buf[buflen++] = data[i];
			break;
		}
	}

	fwrite(buf, 1, buflen, f);
}

static void
handle(struct kevent *event)
{
	static char buf[4096];
	static int in_span=0;
	int is_err;
	ssize_t nread, nleft, ntoread;

	if (event->flags & EV_ERROR)
		errc(1, event->data, NULL);
	if (event->flags & EV_EOF) {
		close((int)event->ident);
		nclosed++;
		return;
	}

	is_err = (int)event->ident == errpipe[0];
	if (is_err && !in_span) {
		printf("<span class=e>");
		in_span = 1;
	} else if (!is_err && in_span) {
		printf("</span>");
		in_span = 0;
	}

	nleft = (ssize_t)event->data;
	while (nleft) {
		ntoread = MIN(nleft, (ssize_t)sizeof(buf));
		nread = read(event->ident, buf, ntoread);
		if (nread == -1)
			err(1, "read(%lu)", event->ident);
		print_html(buf, (size_t)nread, stdout);
		nleft -= nread;
	}
}

int
main(int argc, char **argv)
{
	int kq, n, i;
	struct kevent events[2];
	char **arg;

	if (argc < 2) {
		fputs("usage: hteeml <program> <argument> ...\n",
		    stderr);
		exit(EX_USAGE);
	}

	if (pipe(outpipe) == -1 || pipe(errpipe) == -1)
		err(1, "pipe");

	switch (fork()) {
	case -1:
		err(1, "fork");
	case 0:
		dup2(outpipe[1], STDOUT_FILENO);
		dup2(errpipe[1], STDERR_FILENO);
		close(outpipe[0]);
		close(errpipe[0]);
		execvp(argv[1], &argv[1]);
		err(1, "exec(%s)", argv[1]);
	}

	close(outpipe[1]);
	close(errpipe[1]);

	fputs("<!DOCTYPE html>"
	    "<meta charset=utf-8>"
	    "<meta name=generator content=hteeml>"
	    "<meta name=color-scheme content=\"light dark\">"
	    "<title>", stdout);
	print_html(argv[1], strlen(argv[1]), stdout);
	for (arg = &argv[2]; *arg; arg++) {
		putchar(' ');
		print_html(*arg, strlen(*arg), stdout);
	}
	printf("</title>"
	    "<style>.e{color:red}</style>"
	    "<pre>");

	EV_SET(&events[0], outpipe[0], EVFILT_READ, EV_ADD, 0, 0, 0);
	EV_SET(&events[1], errpipe[0], EVFILT_READ, EV_ADD, 0, 0, 0);

	if ((kq = kqueue()) == -1)
		err(1, "kqueue");
	if (kevent(kq, events, 2, NULL, 0, NULL) == -1)
		err(1, "kevent setup");

	while (nclosed < 2) {
		if ((n = kevent(kq, NULL, 0, events, 2, NULL)) == -1)
			err(1, "kevent");
		for (i=0; i<n; i++)
			handle(&events[i]);
	}

	return 0;
}

