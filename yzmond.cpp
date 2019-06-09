#include "async_server.h"
#include <signal.h>
#include <sys/stat.h>

void create_deamon(){
	// fork off the parent process
	pid_t pid = fork();
	if(pid < 0){
		exit(EXIT_FAILURE);
	}
	
	// exit the parent process
	if(pid > 0){
		exit(EXIT_SUCCESS); 
	}
	
	// create a new sid for the child process
	pid_t sid = setsid();
	if (sid < 0){
		exit(EXIT_FAILURE);
	}

	
	pid = fork();
	
	if(pid < 0){
		exit(EXIT_FAILURE);
	}
	
	if(pid > 0){
		exit(EXIT_SUCCESS);
	}
	
	// set new file permissions
	umask(0);	

	// change the current working directory
	// if((chdir("/")) < 0){
	// 	exit(EXIT_FAILURE);
	// }
	
	// close out the standard file descriptors
	/*	
	int x;
	for(x = sysconf(_SC_OPEN_MAX); x >= 0; x--)
		close(x);
	
	openlog("zyc_daemon", LOG_PID, LOG_DAEMON);*/
}

int main() {
    create_deamon();

    signal(SIGPIPE, SIG_IGN);

    ServerConf conf;
    conf.readConf();

    AsyncServer async_server(conf);
    async_server.eventLoop();
}