# operating-systems-EC440
This repository contains the homework assignments for EC440 Operating Systems, Boston University Fall 2018.

Project 1 Description:
The goal of this project is to implement a basic shell written in C and running on Linux. 
It should be able to execute commands, redirect the standard input/output of commands to files, pipe the output of commands to other commands, and carry out commands in the background.

Project 2 Description:
The main goal for this project is to develop a basic thread system for Linux. Threads are independent units of execution that run (virtually) in parallel in the address space of a single process. As a result, they share the same heap memory, open files (file descriptors), process identifier, etc. Each thread has its own context, which consists of a set of CPU registers and a stack. The thread subsystem provides a set of library functions that applications may use to create, start and terminate threads, and manipulate them in various ways.

The most well-known and widespread standard that specifies a set of interfaces for multi-threading programming on Unix-style operating systems is called POSIX threads (or pthreads). Recall that pthreads merely prescribes the interface of the threading functionality. The implementation then either implement user-mode threads, take advantage of kernel-mode threads (if provided by the operating system), or mix the two approaches. In this project, a small subset of the pthread API is implemented exclusively in user-mode.
