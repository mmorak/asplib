# asplib
Libraries related to ASPTOOLS (Tool collection for Answer Set Programming)

- **liblp**: library for handling logic programs in Smodels format and clauses in DIMACS format

Some basic utilities related to the formats in question:

- **len**: Calculating size parameters
- **lplist**: Printing ground programs in symbolic form
- **lpstrip**: Removing unnecessary (hidden) atoms from program

Compilation requirements:

- *GNU make* (Debian package make) tested with GNU make v. 4.2.1
- *gcc* (Debian package gcc) tested with gcc v. 9.3.0
- *libtool* (Debiand package libtool-bin) tested with libtool v. 2.4.6

Compilation instructions (please check and adapt Makefiles first):

```
$ cd liblp
$ make
$ make install
$ make finish
$ cd ..
$ cd utils
$ make
$ make install
$ cd ..
```
