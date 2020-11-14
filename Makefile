DESTDIR?=
PREFIX?=	/usr/local
MANPREFIX?=	${PREFIX}/man

CFLAGS+=	-Wall -Wextra

all: hteeml

clean:
	rm -f hteeml

install: all
	install -d ${DESTDIR}${PREFIX}/bin \
	           ${DESTDIR}${MANPREFIX}/man1/
	install -m755 hteeml   ${DESTDIR}${PREFIX}/bin/
	install -m644 hteeml.1 ${DESTDIR}${MANPREFIX}/man1/

uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/hteeml \
	      ${DESTDIR}${MANPREFIX}/man1/hteeml.1

.PHONY: all clean install uninstall
