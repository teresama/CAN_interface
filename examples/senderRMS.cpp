/*
 * 
 * Program to send (Tx) RMS CAN frames 
 * 
 * Usage: run ./senderRMS and enter
 * input from the user is the bytes willing to be sent, each byte separated by space
 * example: input: 0 0 0 0 0 0 0 0
 * 	    output in candump: will send 0C0# 00 00 00 00 00 00 00 00 
 * 
*/


#include "socketcan_cpp/socketcan_cpp.h"
#include "socketcan_cpp/RMSMotorController.h"
#include <string>
#include <iostream>
#include <time.h>
#include <vector>
#include <sstream>
#include <fstream>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <cstdlib>

#define CAN_ID 192; // id is an uint32_t, therefore from HEX = 0xC0 we convert to uint32_t which is 192
#define CAN_LENGTH 8;
using namespace std;

#define DEFAULT 48
#define PANIC 30
#define SINGLE 49
#define CYCLIC 50
#define CONTROLLER 51
#define MATR 52

#define MAX_TORQUE 327 

enum opState {
        DISABLED = 0,
        STANDBY = 1,
        ENABLE = 2,
        POWERDOWN = 3
};

enum opState operationState = DISABLED;

//global status variables


scpp::SocketCan sockat_can;

void handleCANMsgIntState(uint8_t *data)
{
	int vsmState, invState, relayState, invRunMode, invActiveDischarge, invCmdMode, invEnable, invLockout, invDirection;
	
	vsmState = data[0] + (data[1] * 256);
	invState = data[2];
	relayState = data[3];
	invRunMode = data[4] & 1;
	invActiveDischarge = data[4] >> 5;
	invCmdMode = data[5];
	isEnabled = data[6] & 1;
	isLockedOut = data[6] >> 7;
	invDirection = data[7];
	
	if (invRunMode) powerMode = modeSpeed;
	else powerMode = modeTorque;
	
	if (invCmdMode)
	{
		isCANControlled = false;
	}
	else
	{
		isCANControlled = true;
	}
	
}

void init(){
    scpp::CanFrame frinit;  
    //TODO read from EEPROM parameters, for the moment we do what is described in p.30 which is
    frinit.id = CAN_ID;
    frinit.len = CAN_LENGTH;
    int i = 0;
    for (i=0; i < frinit.len ; i++){
        frinit.data[i] =0;
    }
    auto write_sc_status = sockat_can.write(frinit);
    operationState = ENABLE;
}



int checkStatus() {
    //scpp::CanFrame frr;
    isCANControlled = true;
        /*
        while(sockat_can.read(frr) == scpp::STATUS_OK)
        {
            
        }
        switch(frr.id){
            
            case 0xAA: //Internal states
                handleCANMsgIntState(frr.data);
                printf("hereeee");
                break;
        }
        * */
        
    if (isCANControlled && operationState == ENABLE){
        return 1;
    }
    else{return 0;}    
    
}	


void DelayMicrosecondsNoSleep (int delay_us)
{
	long int start_time;
	long int time_difference;
	struct timespec gettime_now;

	clock_gettime(CLOCK_REALTIME, &gettime_now);
	start_time = gettime_now.tv_nsec;		//Get nS value
	while (1)
	{
		clock_gettime(CLOCK_REALTIME, &gettime_now);
		time_difference = gettime_now.tv_nsec - start_time;
		if (time_difference < 0)
			time_difference += 1000000000;				//(Rolls over every 1 second)
		if (time_difference > (delay_us * 1000))		//Delay for # nS
			break;
	}
}


int kbhit(void)
{
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if(ch != EOF)
    {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
}


int main()
{   
    scpp::SocketCan sockat_can;
    int opt = 0;
    int c;
    int inside = 0;

        
    if (sockat_can.open("can0") == scpp::STATUS_OK)
    {
        
        printf("Enter the Option: 1 - single input CAN frame 2 - cyclic message 3 - speed commands from PX4 0 - default q - panic");
        for (int j = 0; j < 20000; ++j)
        {
        scpp::CanFrame frr;  
          
        //handle incoming frame
            while(sockat_can.read(frr) == scpp::STATUS_OK)
            {
                switch(frr.id){
                
                case 0xAA: //Internal states
                    handleCANMsgIntState(frr.data);
                    printf("hereeee");
                    break;
            }  
                
            }
        
        //create frame
        scpp::CanFrame fr; //struct defined in socketcan_cpp.h
        inside = 1 ;

        if (kbhit() && inside == 1){

            c = getchar();
            printf("\nstarting mode:");
            printf("%d", c);
            switch (c)
            {
                case PANIC: printf("panic\n");break;
                case DEFAULT: printf("Default Mode\n");break;
                case SINGLE: printf("Single message\n");break;
                case CYCLIC: printf("Cyclic message\n");break;
                case CONTROLLER: printf("Input controller\n");break;

            }

            opt = c;
        }


        switch (opt)
        {
        case SINGLE: {
            printf("enter message separated by nums i.e: 1 2 3 4 5 6 7 8: ");
            std::string nums;
            std::getline(std::cin, nums);
            std::stringstream stream(nums);
            int num;
            std::vector<int> vec;

            while (stream >> num) { vec.push_back(num); }

            std::copy(vec.begin(), vec.end(), fr.data);
            auto write_sc_status = sockat_can.write(fr);
            if (write_sc_status != scpp::STATUS_OK)
                printf("something went wrong on socket write, error code : %d \n", int32_t(write_sc_status));
            else{
                printf("Message was written to the socket \n");
                //back to do nothing
                opt = 0 ;
                printf("Enter the Option: 1 - single input CAN frame 2 - cyclic message 3 - speed commands from PX4");

            }
            break;
        }
        case CYCLIC:{
            fr.id = 170;
            fr.len = CAN_LENGTH;
            for (int i = 0; i < 8; ++i)
                fr.data[i] = 2;
            auto write_sc_status = sockat_can.write(fr);
            if (write_sc_status != scpp::STATUS_OK)
                printf("something went wrong on socket write, error code : %d \n", int32_t(write_sc_status));
            else{
                printf("Message was written to the socket \n");

            }
            break;
        }

        case CONTROLLER:{
            //read file
            ifstream myfile ("/home/pi/cppInterface/can_bus_inverter/examples/matrix.csv");
            std::string line;

            int speedCommand;
            int i = 0;
            int start = 0;
            int queue[50];
            char delim = ',';


            if (myfile.is_open())
            {
                while ( getline (myfile, line) ){
                    std::string values;
                    //values.push_back(line);
                    for (char& c : line){
                        if (start == 1){values +=c;}
                        if (c == delim) {start =1;}
                    }

                    speedCommand = std::atoi(values.c_str());
                    queue[i] = speedCommand;
                    i++;
                    start = 0;
                }
                myfile.close();


            }
            else cout << "unable";
            
            if (operationState == DISABLED){
                init();
            }
            
            //prior to commanding value we set some booleans
            if (checkStatus() ==1){
                //per value of speed command we send CAN frame
                fr.id = CAN_ID;
                fr.len = CAN_LENGTH;



                for  (int j = 0; j < i ; j++ ){
                    //we create CAN frame
                    //Byte 0-1 = Torque command
                    //Byte 2-3 = Speed command (send 0, we don't do speed control)
                    //Byte 4 is Direction (0 = CW, 1 = CCW)
                    //Byte 5 = Bit 0 is Enable, Bit 1 = Discharge (Discharge capacitors)
                    //Byte 6-7 = Commanded Torque Limit (Send as 0 to accept EEPROM parameter unless we're setting the limit really low for some reason such as faulting or a warning)


                    fr.data[0] = 0;
                    fr.data[1] = 0;
                    
                    if (!isLockedOut){
                        fr.data[5] = 1;
                    }
                    else{
                        fr.data[5] = 0;
                    }
                    //depending on reverse input configured default 0
                    fr.data[4] = 0;

                    //Speed set as speedCommand
                    fr.data[3] = (queue[j] & 0xFF00) >> 8; //Stow torque command in bytes 2 and 3.
                    fr.data[2] = (queue[j] & 0x00FF);

                    //Torque limit set as 0
                    fr.data[6] = 0;
                    fr.data[7] = 0;

                    auto write_sc_status = sockat_can.write(fr);
                    if (write_sc_status != scpp::STATUS_OK)
                        printf("something went wrong on socket write, error code : %d \n", int32_t(write_sc_status));
                    else{
                        printf("Message was written to the socket \n");

                    }
                }



                DelayMicrosecondsNoSleep(40000);
            }
            else {
                opt = PANIC;
            }
            



            opt = DEFAULT;
            break;

    }

        case PANIC:{
            //set all values to zero
            
            break;
        }

        case DEFAULT:{
            //do nothing
            printf("%d\n", temperatureMotor);
            break;

        }
    }

        //Next iteration in loop after 500 ms
        DelayMicrosecondsNoSleep(500000);
           }

     }
   
  
  

    else
    {
        printf("Cannot open can socket!");
    }
    return 0;
}   
