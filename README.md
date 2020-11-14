hteeml
======
Capture a program's output as HTML.

Usage
-----
**hteeml** *program* *argument* [*...*]

Runs the given program, captures it stdout and sterr, and combines them
into an HTML document where stderr is marked up red.

Installation
------------
Should work on any Unix with kqueue support, e.g. BSD and macOS but not
Linux.

   make
   sudo make install
   sudo make uninstall

Author
------
Sijmen J. Mulder (<ik@sjmulder.nl>)
