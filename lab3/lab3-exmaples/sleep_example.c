#include <stdio.h>
#include <unistd.h>

int main() 
{
	printf("Start to sleep for 10s\n");
	sleep(10); 
	printf("End of sleep\n");
	return 0;
}

