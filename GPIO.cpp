#include <iostream>
#include <stdio.h>
#include <string.h>
#include "GPIO.h"
using namespace std;

void flashLED(int, int, int);
void controlGPIO(int, int);

int main(){

	int LEDNum = 5, LEDToggle = 5, GPIONum = 10000, GPIOToggle = 5;
	int Select = 3;
	
	while(Select != 1 && Select !=2){
		cout<<"LED (Press 1) or GPIO (Press 2): ";
		cin>>Select;
		cout<<endl;
	}


	//Flash LEDS
	if(Select == 1){
		while(LEDNum > 3 || LEDNum < 0){
			cout<<"Select LED Number (values 0-3): ";
			cin>>LEDNum;
			cout<<endl;
		}

		while(LEDToggle != 0 && LEDToggle != 1){
			cout<<"LED "<<LEDNum<<" on (press 1) or off (press 0): "; 
			cin>>LEDToggle;
			cout<<endl;
			}

		flashLED(LEDNum, LEDToggle, 1);
	}


	if(Select == 2){
		cout<<"Select GPIO Number: ";
		cin>>GPIONum;
		cout<<endl;

		while(GPIOToggle != 0 && GPIOToggle !=1){
			cout<<"Pin "<<GPIONum<<" on (press 1) or off (press 0): ";
			cin>>GPIOToggle;
			cout<<endl;
		}

		controlGPIO(GPIONum, GPIOToggle);		
	}


	return 0;
}



void flashLED(int GPIOPin, int Toggle, int heartbeat){


	FILE *LEDHandle = NULL;
	char LEDNum[64];
	char LEDTrigger[64];
	char setValue[64];

	sprintf(LEDNum, "/sys/class/leds/beaglebone::usr%d/brightness", GPIOPin);
	sprintf(LEDTrigger, "/sys/class/leds/beaglebone::use%d/trigger", GPIOPin);

/*
	for(int i=0; i<10; i++){

		if((LEDHandle = fopen(LEDNum, "r+")) != NULL){
			fwrite("1", sizeof(char), 1, LEDHandle);
			fclose(LEDHandle);
		}
		sleep(1);

		if((LEDHandle = fopen(LEDNum, "r+")) != NULL){
			fwrite("0", sizeof(char), 1, LEDHandle);
			fclose(LEDHandle);
		}
		sleep(1);

	}
*/


	//just turn LED on or off
	if((LEDHandle = fopen(LEDNum, "r+")) != NULL){
		if(Toggle == 1){
			fwrite("1", sizeof(char), 1, LEDHandle);
		}
		else if(Toggle == 0){
			fwrite("0", sizeof(char), 1, LEDHandle);
		}
		fclose(LEDHandle);
	}


	//heartbeat
	if(heartbeat == 1){
		if((LEDHandle = fopen(LEDTrigger, "r+")) != NULL){
			strcpy(setValue, "heartbeat");
			fwrite(&setValue, sizeof(char), strlen("heartbeat"), LEDHandle);
			fclose(LEDHandle);
		}
	}

}






void controlGPIO(int GPIOPin, int Toggle){

	FILE *GPIOHandle = NULL;
	char setValue[64], GPIOString[4];
	char GPIOValue[64], GPIODirection[64];
	
	sprintf(GPIOString, "%d", GPIOPin);
	sprintf(GPIOValue, "/sys/class/gpio/gpio%d/value", GPIOPin);
	sprintf(GPIODirection, "/sys/class/gpio/gpio%d/direction", GPIOPin);


	//Export pin
	if((GPIOHandle = fopen("/sys/class/gpio/export", "ab")) != NULL){
		strcpy(setValue, GPIOString);
		fwrite(&setValue, sizeof(char), strlen(setValue), GPIOHandle);
		fclose(GPIOHandle);
	}



	//Set pin direction
	if((GPIOHandle = fopen(GPIODirection, "rb+")) != NULL){
		strcpy(setValue, "out");
		fwrite(&setValue, sizeof(char), strlen(setValue), GPIOHandle);
		fclose(GPIOHandle);
	}


	//Toggle output
	if((GPIOHandle = fopen(GPIOValue, "rb+")) != NULL){
		if(Toggle == 1){
			strcpy(setValue, "1");
			fwrite(&setValue, sizeof(char), strlen(setValue), GPIOHandle);
		}
		
		if(Toggle == 0){
			strcpy(setValue, "0");
			fwrite(&setValue, sizeof(char), strlen(setValue), GPIOHandle);
		}
		
		fclose(GPIOHandle);
	}

/*	
	//Unexport pin
	if((GPIOHandle = fopen("/sys/class/gpio/unexport", "ab")) != NULL){
		strcpy(setValue, GPIOString);
		fwrite(&setValue, sizeof(char), strlen(setValue), GPIOHandle);
		fclose(GPIOHandle);

	}
*/
}
