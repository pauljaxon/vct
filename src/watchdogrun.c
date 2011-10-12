//==========================================================================
//==========================================================================
// WATCHDOGRUN.C
//==========================================================================
//==========================================================================
/*
This file is part of Victor: a SPARK VC Translator and Prover Driver.

Copyright (C) 2009, 2010 University of Edinburgh

Author(s): Paul Jackson, Altran Praxis, AdaCore

Victor is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or (at
your option) any later version.

Victor is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

A copy of the GNU General Public License V3 can be found in file
LICENSE.txt and online at http://www.gnu.org/licenses/.
*/

/*   

Usage:

  watchdogrun outfile errfile timeout cmd arg1 ...  argn

Runs in subprocess:  cmd arg1 ... argn  1>  outfile 2> errfile.

If no output from command for timeout period, cmd run is killed with SIGTERM.

watchdogrun monitors stdout of subprocess by piping it through main process.

*/     

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <assert.h>
#include <err.h> // for err, errx functions.

// Define MACRO discussed in libc docs, but missing from included files.
// Defined in unistd.h when __USE_GNU is defined.
// __USE_GNU is defined in features.h when _GNU_SOURCE set by gcc.
// _GNU_SOURCE is is an `enable all gnu options' flag which doesn't
// help with creating portable code.

// Use of {} within an expression is not ISO C.

#ifndef TEMP_FAILURE_RETRY
#define TEMP_FAILURE_RETRY(expression) \
    ({ \
        long int _result; \
        do _result = (long int) (expression); \
        while (_result == -1L && errno == EINTR); \
        _result; \
    })
#endif      

void process_term_status(int status) {
    if (WIFEXITED(status)) {
        int exit_status = WEXITSTATUS(status);
        /*
        fprintf(stderr,
                "watchdogrun: seen normal exit status %i from child\n",
                exit_status);
        */
        exit(exit_status);
    }
    else if (WIFSIGNALED(status)) {
        int term_signal = WTERMSIG(status);
        /*
        fprintf(stderr,
                "watchdogrun: child terminated with signal %i\n",
                term_signal);
        */
        exit(term_signal + 128);  // Adding 128 is bash behaviour.
    }
    else {
        fprintf(stderr,
                "watchdogrun: seen unexpected status 0x%x from wait\n",
                status);
        exit(EXIT_FAILURE);
    }
}

int
input_timeout (int filedes, double time)
{
  fd_set set;
  struct timeval timeout;
     
  /* Initialize the file descriptor set. */
  FD_ZERO (&set);
  FD_SET (filedes, &set);


  /* Initialize the timeout data structure. */
  int sec = (int) time;  // Conversion to int truncates.
  int usec = (int) ((time - sec) * 1e6);

  timeout.tv_sec = sec;
  timeout.tv_usec = usec;
     
  /* select returns 0 if timeout, 1 if input available, -1 if error. */
  return TEMP_FAILURE_RETRY (select (FD_SETSIZE,
                                     &set, NULL, NULL,
                                     &timeout));
}

int
main (int argc, char* argv[]) {

    if (argc < 5) {
        fprintf(stderr,
                "Usage: watchdogrun outfile errfile timeout cmd arg1 ...  argn\n");
        exit(EXIT_FAILURE);
    }

    char* outfilename = argv[1];
    char* errfilename = argv[2];
    double timeout = atof(argv[3]);
    char* cmd = argv[4];

    char** cmdargv = &(argv[4]);

    // File descriptor numbers for pipe.
    int pipefd[2];  // pipfd[0] is number for read end descriptor
                    // pipfd[1] is number for write end descriptor
    pid_t cpid;     // Child process id
    char buf;


    if (pipe(pipefd) == -1) {
        perror("pipe creation failed");
        exit(EXIT_FAILURE);
    }
    int pipe_input_fdnum = pipefd[1];
    int pipe_output_fdnum = pipefd[0];

    cpid = fork(); 
    if (cpid == -1) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }

    if (cpid == 0) {
        // CHILD PROCESS CODE

        // Child writes to pipe, so close unused read end
        TEMP_FAILURE_RETRY (close(pipe_output_fdnum));

        int errfile_fdnum = TEMP_FAILURE_RETRY (open (errfilename,
                                                      O_CREAT|O_WRONLY|O_TRUNC,
                                                      S_IRUSR|S_IWUSR));
        if (errfile_fdnum < 0) {
            fprintf(stderr, "Error on open of errfile %s: %s\n",
                    errfilename, strerror(errno));
            exit(EXIT_FAILURE);
        }

        // Duplicate file descriptor for errfile and use it as file descriptor
        // referenced by stderr.
        { 
            int dup2_status = dup2 (errfile_fdnum, STDERR_FILENO);
            if (dup2_status < 0) {
                perror("Error on use of errfile fd for child stderr");
                exit(EXIT_FAILURE);
            }
        }
        
        // Delete original file descriptor for errfile.
        TEMP_FAILURE_RETRY (close (errfile_fdnum));

        // Duplicate file descriptor for pipe input and use it as file
        // descriptor referenced by stdout.
        {
            int dup2_status = dup2 (pipe_input_fdnum, STDOUT_FILENO);
            if (dup2_status < 0) {
                perror("Error on use of pip input fd for child stdout");
                exit(EXIT_FAILURE);
            }
        }
        
        // Delete original file descriptor for pipe input
        TEMP_FAILURE_RETRY (close (pipe_input_fdnum));

        // Run command with supplied arguments.
        int status = execvp (cmd, cmdargv);
        // Only reach here if something goes wrong with 
        // execv.
        if (status == -1) { perror("execv of cmd failed"); }
        exit(EXIT_FAILURE);


    } else {
        // PARENT PROCESS CODE

        // Close unused input end of pipe.
        TEMP_FAILURE_RETRY (close(pipe_input_fdnum));
        
        int outfile_fdnum = TEMP_FAILURE_RETRY (open (outfilename,
                                                      O_CREAT|O_WRONLY|O_TRUNC,
                                                      S_IRUSR|S_IWUSR));
        if (outfile_fdnum < 0) {
            fprintf(stderr, "Error on open of outfile %s: %s\n",
                    outfilename, strerror(errno));
            exit(EXIT_FAILURE);
        }
        while (1) {
            int input_status = input_timeout (pipe_output_fdnum, timeout);
            if (input_status == -1) {
                fprintf (stderr,
                         "Error on select input check: %s\n",
                         strerror(errno));
                exit(EXIT_FAILURE);
            }
            else if (input_status == 0) { // Timeout on read.
                // Does it matter which signal is used?
                // kill(cpid, SIGKILL);
                int kill_status = kill(cpid, SIGTERM);
                if (kill_status < 0) {
                    perror("Error on kill of child");
                    exit(EXIT_FAILURE);
                }
                int term_status;
                int wait_status = wait(&term_status);
                                     // Wait is required to ensure child
                                     // isn't left as zombie.
                if (wait_status < 0) {
                    perror("Error on wait after kill of child: ");
                    exit(EXIT_FAILURE);
                }
                process_term_status(term_status);
                assert(0);
            }
            // input_status = 1, at least 1 character present
            int read_status = read(pipe_output_fdnum, &buf, 1);
            if (read_status == 0) { // EOF
                int term_status; // termination status
                int wait_status = wait(&term_status);
                                       // Wait for child to finish exiting.
                                       // Know then it has closed the errfile.
                if (wait_status < 0) {
                    perror("Error on wait after child exit: ");
                    exit(EXIT_FAILURE);
                }
                process_term_status(term_status);
                assert(0); // Should not reach here
            }
            else if (read_status == -1) { // Error on read
                perror("Error on read of pipe from child");
                exit(EXIT_FAILURE);
            }
            write(outfile_fdnum, &buf, 1);
        }    
    }
}

