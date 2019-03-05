Portable Toolchain (PTC)
========================

Toolchain for native application development. Currently includes a C compiler.

Building on Ubuntu 16.04 LTS and above
--------------------------------------

### Requirements
| Dependency       | Version            | Comments                                                                             |
|------------------|--------------------|--------------------------------------------------------------------------------------|
| Canonical Ubuntu | 16.04 LTS or later |                                                                                      |
| GNU Make         | 4.1                |                                                                                      |
| GNU GCC/G++      | 8.1.0              | For 16.04 LTS users, you must install GCC 8.x from a Personal Package Archive (PPA). |
```
apt-get install make
apt-get install gcc
apt-get install g++
```

### Optional Packages
| Package          | Version |
|------------------|---------|
| GNU GDB          | 7.11.1  |
| Valgrind         | 3.11.0  |
| VIM              | 7.4     |
```
apt-get install gdb
apt-get install valgrind
apt-get install vim
```

### Building compiler
```
cd compiler
make
```
