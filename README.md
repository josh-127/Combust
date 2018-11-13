Portable Toolchain (PTC)
========================

Toolchain for native application development. Currently includes a C compiler.

Building
--------

### Installing Requirements
| Dependency       | Version  |
|------------------|----------|
| CentOS           | 7.5.1804 |
| GNU Make         | 3.82     |
| GNU GCC          | 4.8.5    |
```
yum install make
yum install gcc
```

### Optional Packages
| Package          | Version |
|------------------|---------|
| GNU GDB          | 7.6.1   |
| Valgrind         | 3.13.0  |
| VIM              | 7.4     |
```
yum install gdb
yum install valgrind
yum install vim-minimal
```

### Building PTC
```
make
```
