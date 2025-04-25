# Pipes

There is a command `$ ls | wc -l`. In order to execute the above command, the shell creates two processes, executing
ls and wc, respectively. Two processes are connected to the pipe so that the writing process (ls) has its standard output (file descriptor 1) joined to the write end of the pipe, while the reading process (wc) has its standard input (file descriptor 0) joined to the read end of the pipe.

A pipe is a byte stream. 
When we say that a pipe is a byte stream, we mean that there is no concept of
messages or message boundaries when using a pipe. The process reading from a pipe can read blocks of data of any size, regardless of the size of blocks written by the writing process. Furthermore, the data passes through the pipe sequentially — bytes are read from a pipe in exactly the order they were written. It is not possible
to randomly access the data in a pipe using lseek().

Reading from a pipe
Attempts to read from a pipe that is currently empty block until at least one byte has been written to the pipe. If the write end of a pipe is closed, then a process reading from the pipe will see end-of-file (i.e., read() returns 0) once it has read all remaining data in the pipe.

Pipes are unidirectional
Data can travel only in one direction through a pipe. One end of the pipe is used for writing, and the other end is used for reading. Write to pfd[1] → Read from pfd[0].

Writes of up to PIPE_BUF bytes are guaranteed to be atomic
If multiple processes are writing to a single pipe, then it is guaranteed that their data won’t be intermingled if they write no more than PIPE_BUF bytes at a time. When writing blocks of data larger than PIPE_BUF bytes to a pipe, the kernel may transfer the data in multiple smaller pieces, appending further data as the reader removes bytes from the pipe. When there is only a single process writing to a pipe (the usual case), this doesn’t matter.

Pipes have a limited capacity
A pipe is simply a buffer maintained in kernel memory. This buffer has a maximum capacity. Once a pipe is full, further writes to the pipe block until the reader removes some data from the pipe.

## Creating the pipe

A successful call to pipe() returns two open file descriptors in the array filedes: one for the read end of the pipe ( filedes[0]) and one for the write end ( filedes[1]).

As with any file descriptor, we can use the read() and write() system calls to perform I/O on the pipe. Once written to the write end of a pipe, data is immediately available to be read from the read end. A read() from a pipe obtains the lesser of the number of bytes requested and the number of bytes currently available in the pipe (but blocks if the pipe is empty).

To connect two processes using a pipe, we follow the pipe() call with a call to fork(). During a fork(), the child process inherits copies of its parent’s file descriptors.

While it is possible for the parent and child to both read from and write to the pipe, this is not usual. Therefore, immediately after the fork(), one process closes its descriptor for the write end of the pipe, and the other closes its descriptor for the read end. For example, if the parent is to send data to the child, then it would close its read descriptor for the pipe, filedes[0], while the child would close its write descriptor for the pipe, filedes[1].

One reason that it is not usual to have both the parent and child reading from a single pipe is that if two processes try to simultaneously read from a pipe, we can’t be sure which process will be the first to succeed—the two processes race for data.Preventing such races would require the use of some synchronization mechanism.

How to manage this safely?

    ✅ Use atomic writes: keep each write ≤ PIPE_BUF (typically 4096 bytes on Linux).
    ✅ Use synchronization: a mutex or semaphore to let only one process write at a time.
    ✅ Use other IPC: message queues or sockets if you want message boundaries and concurrency built in.

## Closing unused pipe file descriptors

The process reading from the pipe closes its write descriptor for the pipe. If the reading process doesn’t close the write end of the pipe, then, after the other process closes its write descriptor, the reader won’t see end-of-file, even after it has read all data from the pipe. Instead, a read() would block waiting for data, because the kernel knows that there is still at least one write descriptor open for the pipe.

The writing process closes its read descriptor for the pipe for a different reason.
When a process tries to write to a pipe for which no process has an open read descriptor (if the reading process never starts, or terminates unexpectedly), the kernel sends the SIGPIPE signal to the writing process. By default, this signal kills a process. A process can instead arrange to catch or ignore this signal, in which case the write() on the pipe fails with the error EPIPE (broken pipe). Receiving the SIGPIPE signal or getting the EPIPE error is a useful indication about the status of the pipe, and this is why unused read descriptors for the pipe should be closed. Broken pipe. If you use pipes or FIFOs, you have to design your application so that one process opens the pipe for reading before another starts writing. 

If the writing process doesn’t close the read end of the pipe, then, even after the other process closes the read end of the pipe, the writing process will still be able to write to the pipe. Eventually, the writing process will fill the pipe, and a further attempt to write will block indefinitely.
One final reason for closing unused file descriptors is that it is only after all file descriptors in all processes that refer to a pipe are closed that the pipe is destroyed and its resources released for reuse by other processes. At this point, any unread data in the pipe is lost.