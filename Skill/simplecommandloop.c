#include<stdio.h>
#include<unistd.h>
#include<string.h>

int main() {
	char command[100];
	while(true) {
 		printf("2520030187_SHELLFORGE$");
		fgets(command, sizeof(command), stdin);
		command[strcspn(command, "\n")] = '\0';

		if(strcmp(command, "exit") == 0)
		break;

		printf("you entered:  %s \n", command);
	}
	return 0;
}
