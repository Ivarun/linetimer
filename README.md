linetimer
=========

linetimer is a program that reads lines from stdin and outputs those lines on stdout with timing information on the left margin. It has several options, view them by passing the -h option to the program. This program is somewhat similar to the "ts" utility in moreutils (http://joeyh.name/code/moreutils/).

Installation
------------

linetimer is intended to comply with C99 and POSIX (IEEE Std 1003.1-2008), and should therefore hopefully compile on most of the Unix-like operating systems, and under Cygwin on Windows.

Installation is handled with cmake. Essentially:
* mkdir my/build/directory
* cd my/build/directory
* cmake -DCMAKE_BUILD_TYPE=Release /path/to/linetimer
* make

linetimer should now be built. It can also be installed to the default prefix/bin by running make install, but the only thing that is needed is to copy the linetimer executable to the path somewhere.

Usage
-----

The accompanying "test" program simply prints some lines that take a specified amount of time as a way to test linetimer. I.e., after building the programs use something like:

    ./test | ./linetimer

To ensure decent timing results it may be necessary to set the buffering mode for the program that is being timed. This can be achieved by using the stdbuf utility, which is part of the GNU coreutils. For example if you want to time the output of the test program properly then you would run:	

    stdbuf -o 0 ./test | ./linetimer

Whether or not this is necessary for good results depends on your system. stdout is usually line buffered when connected to a tty, but when it is being piped to linetimer it may have more aggressive buffering behaviour. Since the test program never flushes stdout explicitly it can be used to test whether it is necessary to alter the buffering behaviour or not.

Examples
--------

Running the accompanying test program (using stdbuf to disable buffering) should look something like this:

    > stdbuf -o 0 ./test | linetimer
    0.0080|A line first...
    0.0000|
    0.0000|
    0.0010|1 ms: done
    0.0011|1 ms: done
    0.0011|1 ms: done
    0.0011|1 ms: done
    0.0011|1 ms: done
    0.0000|
    0.1001|100 ms: done
    0.1001|100 ms: done
    0.1001|100 ms: done
    0.1001|100 ms: done
    0.1001|100 ms: done
    0.0000|
    1.5001|1.5 s: done
    1.5411|1.5 s (probably more because of printing). This line should wrap appropr
          |iately: xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
          |xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
          |xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
          |xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
          |xxxxxxxxxxxxxxxxxxxx
    5.5553|5.555 s: done

Or have you ever wondered how long it takes the du utility to find the size of each directory under /usr? I sure have!

    > # My version of du seems to flush stdout appropriately, so there is no need to use stdbuf here.
    > du -s /usr/* 2>/dev/null | linetimer
    0.1432|174704  /usr/bin
    0.0000|0       /usr/games
    0.4419|22956   /usr/include
    1.4700|208860  /usr/lib
    1.1107|325428  /usr/lib64
    0.0329|51016   /usr/local
    0.0372|41860   /usr/sbin
    5.9955|463948  /usr/share
    0.0016|0       /usr/src
    0.0000|4       /usr/tmp
    0.0002|0       /usr/X11R6
    0.0003|28      /usr/x86_64-suse-linux
