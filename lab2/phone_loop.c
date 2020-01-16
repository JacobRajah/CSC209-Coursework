#include <stdio.h>
#include <stdlib.h>

int main(){

	char phone[11];
	int number;
	int status = 1;
	
	scanf("%s\n",phone);

	do{
		status = scanf("%d", &number);
		
		if(number == -1){
			printf("%s\n", phone);

		}
		else if(number >= 0 && number <= 9){
		
			printf("%c\n", phone[number]);

		}
		else{
			printf("ERROR\n");
		}

	} while(status != EOF);
	return 0;
}
