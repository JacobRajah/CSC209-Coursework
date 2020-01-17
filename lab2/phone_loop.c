#include <stdio.h>
#include <stdlib.h>

int main(){

	char phone[11];
	int number;
	int status = 0;
	int wrong = 0;
	
	scanf("%s %d",phone, &number);

	while(status != EOF){
		
		if(number == -1){
			printf("%s\n", phone);

		}
		else if(number >= 0 && number <= 9){
		
			printf("%c\n", phone[number]);

		}
		else{
			printf("ERROR\n");
			wrong++;
		}
		status = scanf("%d", &number);
	}
	if(wrong){
		return 1;
	}
	else{
		return 0;
	}
}
