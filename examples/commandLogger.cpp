/*
 * 
 * Program to handle received Commands 0X0C0
 * stores in .csv file interval current, voltage, power, force
 * 
 * 
*/


#include "socketcan_cpp/socketcan_cpp.h"
#include "socketcan_cpp/RMSMotorController.h"
#include <string>
#include <iostream>
#include <ctime>
#include <unistd.h>
#include <signal.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <chrono>
#include <iomanip> // put_time


//detect CTRL+C or CTRL+Z
struct sigaction old_action_c, old_action_z;
//file
FILE *f;
//timestamp
uint64_t timestamp;
char timeString[80];
char * date;
//for printing id
uint32_t id_CAN;



/*
 * //obtained from logger.cpp
    uint16_t dcVoltage;
    int16_t dcCurrent;
    uint16_t acCurrent;
    uint16_t outVoltage;
    int16_t torqueActual;
 * */

void logTime(){
    sprintf(timeString, "%llu,", timestamp);
    strcat(timeString, strtok(date, "\n"));
    fputs(timeString, f);
    fputs(",",f);

}

void handleCANMsgMotorPos(uint8_t *data)
{
	int motorAngle, motorSpeed, elecFreq, deltaResolver;
	motorAngle = data[0] + (data[1] * 256);
	motorSpeed = data[2] + (data[3] * 256);
	elecFreq = data[4] + (data[5] * 256);
	deltaResolver = data[6] + (data[7] * 256);
	speedActual = motorSpeed;
}

void handleCANMsgCurrent(uint8_t *data)
{
	int phaseCurrentA, phaseCurrentB, phaseCurrentC, busCurrent;
	phaseCurrentA = data[0] + (data[1] * 256);
	phaseCurrentB = data[2] + (data[3] * 256);
	phaseCurrentC = data[4] + (data[5] * 256);
	busCurrent = data[6] + (data[7] * 256);
	dcCurrent = busCurrent;
	acCurrent = phaseCurrentA;
	if (phaseCurrentB > acCurrent) acCurrent = phaseCurrentB;
	if (phaseCurrentC > acCurrent) acCurrent = phaseCurrentC;
}

void handleCANMsgVoltage(uint8_t *data)
{
	dcVoltage = data[0] + (data[1] * 256);
	outVoltage = data[2] + (data[3] * 256);

}



void handleCANMsgCommand(uint8_t *data)
{
    int16_t speedReq, torqueReq, mechanicalPower;
    
    torqueReq = data[0] + (data[1] * 256);
    speedReq = data[2] + (data[3] * 256);
    
    mechanicalPower = dcVoltage * dcCurrent / 10000; //In kilowatts. DC voltage is x10
    
    
    
    //torque values stored are 0.1 times smaller
    fprintf(f, "%d, %d, %d, %d, %d, %d, %d, %d, %d \n",mechanicalPower, dcCurrent, acCurrent, outVoltage, dcVoltage, speedReq, speedActual, torqueReq, torqueActual);
    printf("\n");

}

void handleCANMsgTorqueTimer(uint8_t *data)
{
	int cmdTorque, actTorque;
	uint32_t uptime;
	
	cmdTorque = data[0] + (data[1] * 256);
	actTorque = data[2] + (data[3] * 256);
	uptime = data[4] + (data[5] * 256) + (data[6] * 65536ul) + (data[7] * 16777216ul);
	torqueActual = actTorque;
	torqueCommand = cmdTorque;
}


void sigint_handler(int sig_no)
{
    sigaction(SIGINT, &old_action_c, NULL);
    if (f!=NULL) {fclose(f);}
    kill(0, SIGINT);
}

void sigtstp_handler(int sig_no)
{
    sigaction(SIGTSTP, &old_action_z, NULL);
    if (f!=NULL) {fclose(f);}
    kill(0, SIGTSTP);
}


int main()
{   
    //create .csv file
    time_t rawtime;
    struct tm * timeinfo;
    char filename[80];

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    sprintf(filename, "/home/pi/cppInterface/can_bus_inverter/examples/output%02d%02d%02d.csv",
    timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec );
    f = fopen(filename, "w");

    //create handler of user pressing ctrl+C
    struct sigaction action_c;
    //user pressing ctrl+z
    struct sigaction action_z;

    memset(&action_c, 0, sizeof(action_c));
    memset(&action_z, 0, sizeof(action_z));
    action_c.sa_handler = &sigint_handler;
    action_z.sa_handler = &sigtstp_handler;



    //open CAN socket
    scpp::SocketCan sockat_can;

    
    if (sockat_can.open("vcan0") == scpp::STATUS_OK)
    {
    for (int j = 0; j < 20000; ++j)
    {
        scpp::CanFrame fr; //struct defined in socketcan_cpp.h

        auto start = std::chrono::high_resolution_clock::now();
        auto now = std::chrono::system_clock::now();
        timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(start.time_since_epoch()).count();

        auto in_time_t= std::chrono::system_clock::to_time_t(now);
        date = std::ctime(&in_time_t);


        while(sockat_can.read(fr) == scpp::STATUS_OK)
        {
            //beginning of message received
	    timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(start.time_since_epoch()).count();
            logTime();
            char messageLog[80];
            sprintf(messageLog, "id: %X, inverter msg:   %X   %X   %X   %X   %X   %X  %X, ", fr.len, fr.id,
                    fr.data[0], fr.data[1], fr.data[2], fr.data[3],
                    fr.data[4], fr.data[5], fr.data[6], fr.data[7]);
            fputs(messageLog,f);

            printf("len %d byte, id: %X, inverter msg:  %X   %X   %X   %X   %X   %X   %X  %X  \n", fr.len, fr.id,
                fr.data[0], fr.data[1], fr.data[2], fr.data[3],
                fr.data[4], fr.data[5], fr.data[6], fr.data[7]);

            sigaction(SIGINT, &action_c, &old_action_c);
            sigaction(SIGTSTP, &action_z, &old_action_z);

            switch (fr.id)
            {
            case 0xC0: //Speed or Torque command
                handleCANMsgCommand(fr.data);
                break;
	    case 0xA5: //Motor position info
                handleCANMsgMotorPos(fr.data);
                break;
            case 0xA6: //Current info
                handleCANMsgCurrent(fr.data);	    
                break;
	    case 0xA7: //Voltage info
                handleCANMsgVoltage(fr.data);
                break;
	    case 0xAC: //Torque and Timer info
                handleCANMsgTorqueTimer(fr.data);
                break;
            default: fprintf(f, "nope");
                break;
        
        }
    }
}

}
    else
    {
        fprintf(f, "Cannot open can socket!");
    }
    return 0;
}   
