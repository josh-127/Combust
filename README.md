Portable Toolchain (PTC)
========================

Toolchain for native application development. Currently includes a C compiler.

Building on Windows 10
----------------------

### Requirements
| Dependency                   | Version       | Comments                       |
|------------------------------|---------------|--------------------------------|
| Microsoft Windows 10         | 1809 or later | Home edition is not supported. |
| Microsoft Visual Studio 2019 | 16.x.x        |                                |

### Visual Studio 2019 Workloads
- Desktop development with C++

### Building compiler
`TODO: Include command-line instructions.`

### Testing compiler
`TODO: Include command-line instructions.`

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

### Testing compiler
```
cd compiler
make check
```
