/*
 * 
 * Program to handle received (Rx) RMS CAN frames 
 * RMS CAN frames from ID 0xA0 to 0xAF
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



void logTime(){
    sprintf(timeString, "%llu,", timestamp);
    strcat(timeString, strtok(date, "\n"));
    fputs(timeString, f);
    fputs(",",f);

}


void handleCANMsgTemperature1(uint8_t *data)
{
	int igbtTemp1, igbtTemp2, igbtTemp3, gateTemp;
    igbtTemp1 = data[0] + (data[1] * 256);
	igbtTemp2 = data[2] + (data[3] * 256);
    igbtTemp3 = data[4] + (data[5] * 256);
    gateTemp = data[6] + (data[7] * 256);

    fprintf(f, "IGBT Temps - 1: %d  2: %d  3: %d     Gate Driver: %d    (0.1C)\n", igbtTemp1, igbtTemp2, igbtTemp3, gateTemp);
    temperatureInverter = igbtTemp1;
    if (igbtTemp2 > temperatureInverter) temperatureInverter = igbtTemp2;
    if (igbtTemp3 > temperatureInverter) temperatureInverter = igbtTemp3;
    if (gateTemp > temperatureInverter) temperatureInverter = gateTemp;

}

void handleCANMsgTemperature2(uint8_t *data)
{
    int ctrlTemp, rtdTemp1, rtdTemp2, rtdTemp3;
    ctrlTemp = data[0] + (data[1] * 256);
	rtdTemp1 = data[2] + (data[3] * 256);
    rtdTemp2 = data[4] + (data[5] * 256);
    rtdTemp3 = data[6] + (data[7] * 256);

    fprintf(f, "Ctrl Temp: %d  RTD1: %d   RTD2: %d   RTD3: %d    (0.1C)\n", ctrlTemp, rtdTemp1, rtdTemp2, rtdTemp3);
	temperatureSystem = ctrlTemp;

}

void handleCANMsgTemperature3(uint8_t *data)
{
    int rtdTemp4, rtdTemp5, motorTemp, torqueShudder;
    rtdTemp4 = data[0] + (data[1] * 256);
	rtdTemp5 = data[2] + (data[3] * 256);
    motorTemp = data[4] + (data[5] * 256);
    torqueShudder = data[6] + (data[7] * 256);
    
    fprintf(f, "RTD4: %d   RTD5: %d   Motor Temp: %d    Torque Shudder: %d", rtdTemp4, rtdTemp5, motorTemp, torqueShudder);
	temperatureMotor = motorTemp;
}

void handleCANMsgAnalogInputs(uint8_t *data)
{
	int analog1, analog2, analog3, analog4;
    analog1 = data[0] + (data[1] * 256);
	analog2 = data[2] + (data[3] * 256);
    analog3 = data[4] + (data[5] * 256);
    analog4 = data[6] + (data[7] * 256);

	fprintf(f, "RMS  A1: %d   A2: %d   A3: %d   A4: %d", analog1, analog2, analog3, analog4);
}

void handleCANMsgDigitalInputs(uint8_t *data)
{
	//in case it matters:  (1 - 8 not 0 - 7)
	//DI 1 = Forward switch, 2 = Reverse Switch, 3 = Brake Switch, 4 = Regen Disable Switch, 5 = Ignition, 6 = Start 
	uint8_t digInputs = 0;
	for (int i = 0; i < 8; i++)
	{
		if (data[i] == 1) digInputs |= 1 << i;
	}

	fprintf(f, "Digital Inputs: %b", digInputs);
}

void handleCANMsgMotorPos(uint8_t *data)
{
	int motorAngle, motorSpeed, elecFreq, deltaResolver;
    motorAngle = data[0] + (data[1] * 256);
	motorSpeed = data[2] + (data[3] * 256);
    elecFreq = data[4] + (data[5] * 256);
    deltaResolver = data[6] + (data[7] * 256);
	speedActual = motorSpeed;

	fprintf(f, "Angle: %d   Speed: %d   Freq: %d    Delta: %d", motorAngle, motorSpeed, elecFreq, deltaResolver);
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

	fprintf(f, "Phase A: %d    B: %d   C: %d    Bus Current: %d", phaseCurrentA, phaseCurrentB, phaseCurrentC, busCurrent);
}

void handleCANMsgVoltage(uint8_t *data)
{
	int Vd, Vq;
	dcVoltage = data[0] + (data[1] * 256);
	outVoltage = data[2] + (data[3] * 256);
	Vd = data[4] + (data[5] * 256);
	Vq = data[6] + (data[7] * 256);

	fprintf(f, "Bus Voltage: %d    OutVoltage: %d   Vd: %d    Vq: %d", dcVoltage, outVoltage, Vd, Vq);
}

void handleCANMsgFlux(uint8_t *data)
{
	int fluxCmd, fluxEst, Id, Iq;
    fluxCmd = data[0] + (data[1] * 256);
	fluxEst = data[2] + (data[3] * 256);
    Id = data[4] + (data[5] * 256);
    Iq = data[6] + (data[7] * 256);

	fprintf(f, "Flux Cmd: %d  Flux Est: %d   Id: %d    Iq: %d", fluxCmd, fluxEst, Id, Iq);
}

void handleCANMsgIntVolt(uint8_t *data)
{
	int volts15, volts25, volts50, volts120;
    volts15 = data[0] + (data[1] * 256);
	volts25 = data[2] + (data[3] * 256);
    volts50 = data[4] + (data[5] * 256);
    volts120 = data[6] + (data[7] * 256);

	fprintf(f, "1.5V: %d   2.5V: %d   5.0V: %d    12V: %d", volts15, volts25, volts50, volts120);
}

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


    switch (vsmState)
    {
    case 0:
	    fprintf(f, "VSM Start");
		break;		
    case 1:
	    fprintf(f, "VSM Precharge Init");
		break;		
    case 2:
	    fprintf(f, "VSM Precharge Active");
		break;		
    case 3:
	    fprintf(f, "VSM Precharge Complete");
		break;		
    case 4:
	    fprintf(f, "VSM Wait");
		break;		
    case 5:
	    fprintf(f, "VSM Ready");
		break;		
    case 6:
	    fprintf(f, "VSM Motor Running");
		break;		
    case 7:
	    fprintf(f, "VSM Blink Fault Code");
		break;		
    case 14:
	    fprintf(f, "VSM Shutdown in process");
		break;		
    case 15:
	    fprintf(f, "VSM Recycle power state");
		break;		
    default:
	    fprintf(f, "Unknown VSM State!");
		break;				
	}	
	
	switch (invState)
	{
    case 0:
	    fprintf(f, "Inv - Power On");
		break;		
    case 1:
	    fprintf(f, "Inv - Stop");
		break;		
    case 2:
	    fprintf(f, "Inv - Open Loop");
		break;		
    case 3:
	    fprintf(f, "Inv - Closed Loop");
		break;		
    case 4:
	    fprintf(f, "Inv - Wait");
		break;		
    case 8:
	    fprintf(f, "Inv - Idle Run");
		break;		
    case 9:
	    fprintf(f, "Inv - Idle Stop");
		break;		
    default:
	    fprintf(f, "Internal Inverter State");
		break;				
	}
	
	fprintf(f, "Relay States: %b", relayState);
	
	if (invRunMode) powerMode = modeSpeed;
	else powerMode = modeTorque;
	
	switch (invActiveDischarge)
	{
	case 0:
		fprintf(f, "Active Discharge Disabled");
		break;
	case 1:
		fprintf(f, "Active Discharge Enabled - Waiting");
		break;
	case 2:
		fprintf(f, "Active Discharge Checking Speed");
		break;
	case 3:
		fprintf(f, "Active Discharge In Process");
		break;
	case 4:
		fprintf(f, "Active Discharge Completed");
		break;		
	}
	
	if (invCmdMode)
	{
		fprintf(f, "VSM Mode Active");
		isCANControlled = false;
	}
	else
	{
		fprintf(f, "CAN Mode Active");
		isCANControlled = true;
	}
	
	fprintf(f, "Enabled: %t    Forward: %t", isEnabled, invDirection);
}

void handleCANMsgFaults(uint8_t *data)
{
	uint32_t postFaults, runFaults;
	
	postFaults = data[0] + (data[1] * 256) + (data[2] * 65536ul) + (data[3] * 16777216ul);
	runFaults = data[4] + (data[5] * 256) + (data[6] * 65536ul) + (data[7] * 16777216ul);
	
	//for non-debugging purposes if either of the above is not zero then crap has hit the fan. Register as faulted and quit trying to move
	if (postFaults != 0 || runFaults != 0) faulted = true;
	else faulted = false;
	
	if (postFaults & 1) fprintf(f, "Desat Fault!");
	if (postFaults & 2) fprintf(f, "HW Over Current Limit!");
	if (postFaults & 4) fprintf(f, "Accelerator Shorted!");
	if (postFaults & 8) fprintf(f, "Accelerator Open!");
	if (postFaults & 0x10) fprintf(f, "Current Sensor Low!");
	if (postFaults & 0x20) fprintf(f, "Current Sensor High!");
	if (postFaults & 0x40) fprintf(f, "Module Temperature Low!");
	if (postFaults & 0x80) fprintf(f, "Module Temperature High!");
	if (postFaults & 0x100) fprintf(f, "Control PCB Low Temp!");
	if (postFaults & 0x200) fprintf(f, "Control PCB High Temp!");
	if (postFaults & 0x400) fprintf(f, "Gate Drv PCB Low Temp!");
	if (postFaults & 0x800) fprintf(f, "Gate Drv PCB High Temp!");
	if (postFaults & 0x1000) fprintf(f, "5V Voltage Low!");
	if (postFaults & 0x2000) fprintf(f, "5V Voltage High!");
	if (postFaults & 0x4000) fprintf(f, "12V Voltage Low!");
	if (postFaults & 0x8000) fprintf(f, "12V Voltage High!");
	if (postFaults & 0x10000) fprintf(f, "2.5V Voltage Low!");
	if (postFaults & 0x20000) fprintf(f, "2.5V Voltage High!");
	if (postFaults & 0x40000) fprintf(f, "1.5V Voltage Low!");
	if (postFaults & 0x80000) fprintf(f, "1.5V Voltage High!");
	if (postFaults & 0x100000) fprintf(f, "DC Bus Voltage High!");
	if (postFaults & 0x200000) fprintf(f, "DC Bus Voltage Low!");
	if (postFaults & 0x400000) fprintf(f, "Precharge Timeout!");
	if (postFaults & 0x800000) fprintf(f, "Precharge Voltage Failure!");
	if (postFaults & 0x1000000) fprintf(f, "EEPROM Checksum Invalid!");
	if (postFaults & 0x2000000) fprintf(f, "EEPROM Data Out of Range!");
	if (postFaults & 0x4000000) fprintf(f, "EEPROM Update Required!");
	if (postFaults & 0x40000000) fprintf(f, "Brake Shorted!");
	if (postFaults & 0x80000000) fprintf(f, "Brake Open!");
	
	if (runFaults & 1) fprintf(f, "Motor Over Speed!");
	if (runFaults & 2) fprintf(f, "Over Current!");
	if (runFaults & 4) fprintf(f, "Over Voltage!");
	if (runFaults & 8) fprintf(f, "Inverter Over Temp!");
	if (runFaults & 0x10) fprintf(f, "Accelerator Shorted!");
	if (runFaults & 0x20) fprintf(f, "Accelerator Open!");
	if (runFaults & 0x40) fprintf(f, "Direction Cmd Fault!");
	if (runFaults & 0x80) fprintf(f, "Inverter Response Timeout!");
	if (runFaults & 0x100) fprintf(f, "Hardware Desat Error!");
	if (runFaults & 0x200) fprintf(f, "Hardware Overcurrent Fault!");
	if (runFaults & 0x400) fprintf(f, "Under Voltage!");
	if (runFaults & 0x800) fprintf(f, "CAN Cmd Message Lost!");
	if (runFaults & 0x1000) fprintf(f, "Motor Over Temperature!");
	if (runFaults & 0x10000) fprintf(f, "Brake Input Shorted!");
	if (runFaults & 0x20000) fprintf(f, "Brake Input Open!");
	if (runFaults & 0x40000) fprintf(f, "IGBT A Over Temperature!");
	if (runFaults & 0x80000) fprintf(f, "IGBT B Over Temperature!");
	if (runFaults & 0x100000) fprintf(f, "IGBT C Over Temperature!");
	if (runFaults & 0x200000) fprintf(f, "PCB Over Temperature!");
	if (runFaults & 0x400000) fprintf(f, "Gate Drive 1 Over Temperature!");
	if (runFaults & 0x800000) fprintf(f, "Gate Drive 2 Over Temperature!");
	if (runFaults & 0x1000000) fprintf(f, "Gate Drive 3 Over Temperature!");
	if (runFaults & 0x2000000) fprintf(f, "Current Sensor Fault!");
	if (runFaults & 0x40000000) fprintf(f, "Resolver Not Connected!");
	if (runFaults & 0x80000000) fprintf(f, "Inverter Discharge Active!");
	
}

void handleCANMsgTorqueTimer(uint8_t *data)
{
	int cmdTorque, actTorque;
	uint32_t uptime;
	
	cmdTorque = data[0] + (data[1] * 256);
	actTorque = data[2] + (data[3] * 256);
	uptime = data[4] + (data[5] * 256) + (data[6] * 65536ul) + (data[7] * 16777216ul);

	fprintf(f, "Torque Cmd: %d   Actual: %d     Uptime: %d", cmdTorque, actTorque, uptime);
	torqueActual = actTorque;
	torqueCommand = cmdTorque;
}

void handleCANMsgModFluxWeaken(uint8_t *data)
{
	int modIdx, fieldWeak, IdCmd, IqCmd;
    modIdx = data[0] + (data[1] * 256);
	fieldWeak = data[2] + (data[3] * 256);
    IdCmd = data[4] + (data[5] * 256);
    IqCmd = data[6] + (data[7] * 256);
	fprintf(f, "Mod: %d  Weaken: %d   Id: %d   Iq: %d", modIdx, fieldWeak, IdCmd, IqCmd);
}

void handleCANMsgFirmwareInfo(uint8_t *data)
{
	int EEVersion, firmVersion, dateMMDD, dateYYYY;
    EEVersion = data[0] + (data[1] * 256);
	firmVersion = data[2] + (data[3] * 256);
    dateMMDD = data[4] + (data[5] * 256);
    dateYYYY = data[6] + (data[7] * 256);
	fprintf(f, "EEVer: %d  Firmware: %d   Date: %d %d", EEVersion, firmVersion, dateMMDD, dateYYYY);
}

void handleCANMsgDiagnostic(uint8_t *data)
{
}

// Logger
/*
void debug(DeviceId deviceId, char *message, ...) {
    if (logLevel > Debug)
        return;
    va_list args;
    va_start(args, message);
    log(deviceId, Debug, message, args);
    va_end(args);
    
    std::time_t result = std::time(nullptr);
}
*/

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
    sprintf(filename, "/home/pi/cppInterface/can_bus_inverter/examples/%02d%02d%02d.csv",
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

    
    if (sockat_can.open("can0") == scpp::STATUS_OK)
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
            logTime();
            char messageLog[80];
            sprintf(messageLog, "len %d byte, id: %X, inverter msg:  %X   %X   %X   %X   %X   %X   %X  %X, ", fr.len, fr.id,
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
            case 0xA0: //Temperatures 1 (driver section temperatures)
                handleCANMsgTemperature1(fr.data);
                break;
            case 0xA1: //Temperatures 2 (ctrl board and RTD inputs)
                handleCANMsgTemperature2(fr.data);
                break;
            case 0xA2: //Temperatures 3 (More RTD, Motor Temp, Torque Shudder)
                handleCANMsgTemperature3(fr.data);
                break;
            case 0xA3: //Analog input voltages
                handleCANMsgAnalogInputs(fr.data);	    
                break;
            case 0xA4: //Digital input status
                handleCANMsgDigitalInputs(fr.data);
                break;
            case 0xA5: //Motor position info
                handleCANMsgMotorPos(fr.data);
                break;
            case 0xA6: //Current info
                handleCANMsgCurrent(fr.data);	    
                break;
            case 0xA7: //Voltage info
                handleCANMsgVoltage(fr.data);
		printf("%d", dcVoltage);
                break;
            case 0xA8: //Flux Info
                handleCANMsgFlux(fr.data);
                break;
            case 0xA9: //Internal voltages
                handleCANMsgIntVolt(fr.data);
                break;
            case 0xAA: //Internal states
                handleCANMsgIntState(fr.data);
                break;
            case 0xAB: //Fault Codes
                handleCANMsgFaults(fr.data);
                break;
            case 0xAC: //Torque and Timer info
                handleCANMsgTorqueTimer(fr.data);
                break;
            case 0xAD: //Mod index and flux weakening info
                handleCANMsgModFluxWeaken(fr.data);
                break;
            case 0xAE: //Firmware Info
                handleCANMsgFirmwareInfo(fr.data);   	
                break;
            case 0xAF: //Diagnostic Data
                handleCANMsgDiagnostic(fr.data);			
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
