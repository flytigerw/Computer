#ifndef BECOME_DAEMON_H
#define BECOME_DAEMON_H 

//Bit-mask values for falgs
#define BD_NO_CHIDIR  01  //Dont chdir("/")
#define BD_NO_CLOSE_FILES 02  //Don't close all open files 
#define BD_NO_REOPEN_STD_FDS 04  //Don't reopen stdin stdout and stderr to /dev/null 
#define BD_NO_UMASK0     010   //Don't do a umask(0)
#define BD_MAX_CLOSE   8192   //Maximum fds to close if sysconf(_SC_OPEN_MAX) is indeterminated

int becomeDaemon(int flags);
#endif
