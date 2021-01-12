#ifndef CREATE_PID_FILE_H   /* Prevent accidental double inclusion */
#define CREATE_PID_FILE_H

#define CPF_CLOEXEC 1        //It causes createPidFile() to set close-on-exec flag for the fd.
                             //It's useful for servers that restart themselves using exec 
                             //If the fd is not closed during exec(),then the restarted server would think that a duplicate instance of server is running

int createPidFile(const char *progName, const char *pidFile, int flags);

#endif
