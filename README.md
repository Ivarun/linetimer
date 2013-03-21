linetimer
=========

linetimer is a program made to time lines of input read from stdin, and output those lines on stdout with timing information on the left margin. It has several options, available by passing the -h option to the program. This program is somewhat similar to the "ts" utility in moreutils (http://joeyh.name/code/moreutils/).

Installation
------------

linetimer is intended to comply with C99 and POSIX (IEEE Std 1003.1-2008), and should therefore hopefully compile on most of the Unix-like operating systems, and under Cygwin on Windows.

Installation is handled with cmake. Essentially:
mkdir my/build/directory
cd my/build/directory
cmake -DCMAKE_BUILD_TYPE=Release /path/to/linetimer
make

linetimer should now be built. It can also be installed to the default prefix/bin by running make install, but the only thing that is needed is to move the linetimer executable to the path somewhere.

Usage
-----

The accompanying "test" program simply prints some lines that take a specified amount of time as a way to test linetimer. I.e., after building the programs use something like:
    ./test | ./linetimer
To ensure decent timing results it may be necessary to set the buffering mode for the program that is being timed. This can be achieved by using the stdbuf utility, which is part of the GNU coreutils. For example if you want to time the output of the test program properly then you would run:	
    stdbuf -o 0 ./test | ./linetimer
Whether or not this is necessary for good results depends on your system. stdout is usually line buffered when connected to a tty, but when it is being piped to linetimer it may have more aggressive buffering behaviour. Since the test program never flushes stdout explicitly it can be used to test whether it is necessary to alter the buffering behaviour or not.

