#include <stdio.h>	// C standard
#include <stdlib.h>	// C standard lib
#include <string.h>	// lib for string proc
#include <pwd.h>	// lib to query info
#include <dirent.h>	// lib for dir traversing

void processorInfo(){
	FILE *fp = fopen("/proc/cpuinfo", "r");
	char* line = NULL;		// required to getline
	ssize_t read;			// required to getline
	size_t len = 0;			// required to getline

	char temp[11];			// for comparison
	temp[10] = '\0';		// end of line
	while( (read = getline(&line, &len, fp) != -1 )){
		// model name
		if (!strcmp(strncpy(temp, line, 10), "model name")) {
			// printf("%s", strchr(line, ':')+2);
			sscanf(line,"model name\t: %[^\t\n]", line);
			printf("%s\n", line);
			free(line);		// free space
			break;
		}
			
	}
	fclose(fp);
}

void kernelInfo(){
	FILE *fp = fopen("/proc/version", "r");		// open file
	char* line = NULL;		// required to getline
	size_t len = 0;
	getline(&line, &len, fp);
	*strchr(strchr(strchr(line, ' ')+1 , ' ') + 1, ' ') = '\0'; 	// set null just after the target string
	printf("%s\n", strchr(strchr(line, ' ')+1 , ' ') + 1);			// print the target string
	free(line);		// free the line
	fclose(fp);		// close file
}

void memoryInfo(){
	FILE *fp = fopen("/proc/meminfo", "r");		// open file
	char* line = NULL;
	size_t len = 0;

	getline(&line, &len, fp);
	sscanf(line, "MemTotal: %[^\t\n]", line);
	printf("%s\n", line);

	free(line);
	fclose(fp);

}

void printTwo(int i){
	if (i==0){
		printf("00");
	}
	else if(i<10){
		printf("0%d", i);
	}
	else{
		printf("%d", i);
	}
}

void uptimeInfo(){

	FILE *fp = fopen("/proc/uptime", "r");
	double time;
	char* line = NULL;
	size_t len = 0;

	int day;
	int hour;
	int min;
	int sec;

	getline(&line, &len, fp);
	sscanf(line, "%lf ", &time);

	day = time/86400;
	time = time - (int)(time/86400)*86400;

	hour = time/3600;
	time = time - (int)(time/3600)*3600;

	min = time/60;
	time = time- (int)(time/60)*60;

	sec = time;

	printTwo(day);
	printf("D:");
	printTwo(hour);
	printf("H:");
	printTwo(min);
	printf("M:");
	printTwo(sec);
	printf("S\n");


	free(line);
	fclose(fp);


}

void userProcessInfo(const char * user_name){
	struct passwd* passwdp = getpwnam(user_name);// get the user id of given user name
	if (!passwdp){							// if the user_name wrong, then quit
		printf("Error: Invalid username %s\n",user_name);
		return;
	}

	DIR* dirp = opendir("/proc");	// iterate the pid files name
	char* line;						// for getline
	size_t len = 0;					// for getline
	ssize_t read;					// for getline, -1 when end of file
	FILE * fp;						
	struct dirent * p;				// point to the file? in the directory
	
	char file[50];					// prepare to store the path of the /proc/[PID]/status
	char Uid[10];					// store the "Uid:"
	char process_name[50];			// store the process name
	int uid;						// store the uid of a process
	int readName = 0;				// determine captured the name of the process or not

	strcpy(file, "/proc/");			// initialize the path
	if(dirp){
		while((p=readdir(dirp))!=NULL){					// read all files under /proc/
			int a = -1;
			sscanf(p->d_name, "%d", &a);
			if(a!=-1){									// if the file is a integer

				strcat(file, p->d_name);				// append the pid
				strcat(file, "/status");				// append the /status

				fp = fopen(file, "r");					// open file
				if(!fp){								// quit if fp is NULL
					continue;
				}
				readName = 0;
				while( (read = getline(&line, &len, fp)) != -1){
					sscanf(line, "%4s", Uid);			// get Uid and uid, be careful the Uid should large enough to prevent buffer overflow
					
					if(readName==0) {					// scan the process name just once
						sscanf(line, "Name: %s", process_name);
						readName = 1;					// reset flag 
					}

					if(strcmp(Uid, "Uid:")==0){			// time to compare uid
						sscanf(line, "Uid: %d", &uid);
						if(uid == passwdp->pw_uid) {	// if uid two are equals
							printf("%s: %s\n", p->d_name, process_name);
							readName = 0;				// reset flag
							break;
						}
					}
				}
				file[6] = '\0';							// reset file back to "/proc/"
				fclose(fp);								// close the file
			}else{
				continue;
			}
			
		}
	}
	closedir(dirp);

}


int main(int argc, char *argv[]){


	// testing true
	if (argc < 2){
		printf("myproc: [option] [parameter]\n");
		printf("Usage 1: $> ./myproc processor - display processor type\n");
		printf("Usage 2: $> ./myproc kernel_version - display kernel version\n");
		printf("Usage 3: $> ./myproc memory - display the amount of memory in kB\n");
		printf("Usage 4: $> ./myproc uptime - display up time since booted in D:H:M:S format\n");
		printf("Usage 5: $> ./myproc user_process [username] - display a list of processes owned by the given username\n");
	}		
	else {

		if(strcmp(argv[1],"processor") == 0){
			// print processor info
			// info found in /proc/cpuinfo
			processorInfo();
		}
		

		else if(strcmp(argv[1], "kernel_version") == 0){
			// print kernel info
			// info found in /proc/version
			kernelInfo();
		}
		

		else if(strcmp(argv[1], "memory") == 0){
			// print memory info
			// info found in /proc/meminfo
			memoryInfo();
		}


		else if(strcmp(argv[1], "uptime") == 0){
			// print uptime info
			// info found in /proc/uptime
			uptimeInfo();
		}


		else if(strcmp(argv[1], "user_process") == 0){
			if (argc < 3){
				printf("Error: Missing username\n");
				return 0;
			}
			// print user_process info
			// info found in /proc/[PID]/status
			userProcessInfo(argv[2]);
		}

		else{
			printf("Error: Unknown option %s\n", argv[1]);
		}
		
	}

	return 0;
}
