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



void handleCANMsgTemperature1(uint8_t *data)
{
	int igbtTemp1, igbtTemp2, igbtTemp3, gateTemp;
    igbtTemp1 = data[0] + (data[1] * 256);
	igbtTemp2 = data[2] + (data[3] * 256);
    igbtTemp3 = data[4] + (data[5] * 256);
    gateTemp = data[6] + (data[7] * 256);
    printf("IGBT Temps - 1: %d  2: %d  3: %d     Gate Driver: %d    (0.1C)\n", igbtTemp1, igbtTemp2, igbtTemp3, gateTemp);
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
    printf("Ctrl Temp: %d  RTD1: %d   RTD2: %d   RTD3: %d    (0.1C)\n", ctrlTemp, rtdTemp1, rtdTemp2, rtdTemp3);
	temperatureSystem = ctrlTemp;
}

void handleCANMsgTemperature3(uint8_t *data)
{
    int rtdTemp4, rtdTemp5, motorTemp, torqueShudder;
    rtdTemp4 = data[0] + (data[1] * 256);
	rtdTemp5 = data[2] + (data[3] * 256);
    motorTemp = data[4] + (data[5] * 256);
    torqueShudder = data[6] + (data[7] * 256);
    printf("RTD4: %d   RTD5: %d   Motor Temp: %d    Torque Shudder: %d", rtdTemp4, rtdTemp5, motorTemp, torqueShudder);
	temperatureMotor = motorTemp;
}

void handleCANMsgAnalogInputs(uint8_t *data)
{
	int analog1, analog2, analog3, analog4;
    analog1 = data[0] + (data[1] * 256);
	analog2 = data[2] + (data[3] * 256);
    analog3 = data[4] + (data[5] * 256);
    analog4 = data[6] + (data[7] * 256);
	printf("RMS  A1: %d   A2: %d   A3: %d   A4: %d", analog1, analog2, analog3, analog4);
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
	printf("Digital Inputs: %b", digInputs);
}

void handleCANMsgMotorPos(uint8_t *data)
{
	int motorAngle, motorSpeed, elecFreq, deltaResolver;
    motorAngle = data[0] + (data[1] * 256);
	motorSpeed = data[2] + (data[3] * 256);
    elecFreq = data[4] + (data[5] * 256);
    deltaResolver = data[6] + (data[7] * 256);
	speedActual = motorSpeed;
	printf("Angle: %d   Speed: %d   Freq: %d    Delta: %d", motorAngle, motorSpeed, elecFreq, deltaResolver);
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
	printf("Phase A: %d    B: %d   C: %d    Bus Current: %d", phaseCurrentA, phaseCurrentB, phaseCurrentC, busCurrent);
}

void handleCANMsgVoltage(uint8_t *data)
{
	int Vd, Vq;
	printf("hereee%d", dcVoltage);
    dcVoltage = data[0] + (data[1] * 256);
	outVoltage = data[2] + (data[3] * 256);
    Vd = data[4] + (data[5] * 256);
    Vq = data[6] + (data[7] * 256);
	printf("Bus Voltage: %d    OutVoltage: %d   Vd: %d    Vq: %d", dcVoltage, outVoltage, Vd, Vq);
}

void handleCANMsgFlux(uint8_t *data)
{
	int fluxCmd, fluxEst, Id, Iq;
    fluxCmd = data[0] + (data[1] * 256);
	fluxEst = data[2] + (data[3] * 256);
    Id = data[4] + (data[5] * 256);
    Iq = data[6] + (data[7] * 256);
	printf("Flux Cmd: %d  Flux Est: %d   Id: %d    Iq: %d", fluxCmd, fluxEst, Id, Iq);
}

void handleCANMsgIntVolt(uint8_t *data)
{
	int volts15, volts25, volts50, volts120;
    volts15 = data[0] + (data[1] * 256);
	volts25 = data[2] + (data[3] * 256);
    volts50 = data[4] + (data[5] * 256);
    volts120 = data[6] + (data[7] * 256);
	printf("1.5V: %d   2.5V: %d   5.0V: %d    12V: %d", volts15, volts25, volts50, volts120);
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
	    printf("VSM Start");
		break;		
    case 1:
	    printf("VSM Precharge Init");
		break;		
    case 2:
	    printf("VSM Precharge Active");
		break;		
    case 3:
	    printf("VSM Precharge Complete");
		break;		
    case 4:
	    printf("VSM Wait");
		break;		
    case 5:
	    printf("VSM Ready");
		break;		
    case 6:
	    printf("VSM Motor Running");
		break;		
    case 7:
	    printf("VSM Blink Fault Code");
		break;		
    case 14:
	    printf("VSM Shutdown in process");
		break;		
    case 15:
	    printf("VSM Recycle power state");
		break;		
    default:
	    printf("Unknown VSM State!");
		break;				
	}	
	
	switch (invState)
	{
    case 0:
	    printf("Inv - Power On");
		break;		
    case 1:
	    printf("Inv - Stop");
		break;		
    case 2:
	    printf("Inv - Open Loop");
		break;		
    case 3:
	    printf("Inv - Closed Loop");
		break;		
    case 4:
	    printf("Inv - Wait");
		break;		
    case 8:
	    printf("Inv - Idle Run");
		break;		
    case 9:
	    printf("Inv - Idle Stop");
		break;		
    default:
	    printf("Internal Inverter State");
		break;				
	}
	
	printf ("Relay States: %b", relayState);
	
	if (invRunMode) powerMode = modeSpeed;
	else powerMode = modeTorque;
	
	switch (invActiveDischarge)
	{
	case 0:
		printf("Active Discharge Disabled");
		break;
	case 1:
		printf("Active Discharge Enabled - Waiting");
		break;
	case 2:
		printf("Active Discharge Checking Speed");
		break;
	case 3:
		printf("Active Discharge In Process");
		break;
	case 4:
		printf("Active Discharge Completed");
		break;		
	}
	
	if (invCmdMode)
	{
		printf("VSM Mode Active");
		isCANControlled = false;
	}
	else
	{
		printf("CAN Mode Active");
		isCANControlled = true;
	}
	
	printf("Enabled: %t    Forward: %t", isEnabled, invDirection);
}

void handleCANMsgFaults(uint8_t *data)
{
	uint32_t postFaults, runFaults;
	
	postFaults = data[0] + (data[1] * 256) + (data[2] * 65536ul) + (data[3] * 16777216ul);
	runFaults = data[4] + (data[5] * 256) + (data[6] * 65536ul) + (data[7] * 16777216ul);
	
	//for non-debugging purposes if either of the above is not zero then crap has hit the fan. Register as faulted and quit trying to move
	if (postFaults != 0 || runFaults != 0) faulted = true;
	else faulted = false;
	
	if (postFaults & 1) printf("Desat Fault!");
	if (postFaults & 2) printf("HW Over Current Limit!");
	if (postFaults & 4) printf("Accelerator Shorted!");
	if (postFaults & 8) printf("Accelerator Open!");
	if (postFaults & 0x10) printf("Current Sensor Low!");
	if (postFaults & 0x20) printf("Current Sensor High!");
	if (postFaults & 0x40) printf("Module Temperature Low!");
	if (postFaults & 0x80) printf("Module Temperature High!");
	if (postFaults & 0x100) printf("Control PCB Low Temp!");
	if (postFaults & 0x200) printf("Control PCB High Temp!");
	if (postFaults & 0x400) printf("Gate Drv PCB Low Temp!");
	if (postFaults & 0x800) printf("Gate Drv PCB High Temp!");
	if (postFaults & 0x1000) printf("5V Voltage Low!");
	if (postFaults & 0x2000) printf("5V Voltage High!");
	if (postFaults & 0x4000) printf("12V Voltage Low!");
	if (postFaults & 0x8000) printf("12V Voltage High!");
	if (postFaults & 0x10000) printf("2.5V Voltage Low!");
	if (postFaults & 0x20000) printf("2.5V Voltage High!");
	if (postFaults & 0x40000) printf("1.5V Voltage Low!");
	if (postFaults & 0x80000) printf("1.5V Voltage High!");
	if (postFaults & 0x100000) printf("DC Bus Voltage High!");
	if (postFaults & 0x200000) printf("DC Bus Voltage Low!");
	if (postFaults & 0x400000) printf("Precharge Timeout!");
	if (postFaults & 0x800000) printf("Precharge Voltage Failure!");
	if (postFaults & 0x1000000) printf("EEPROM Checksum Invalid!");
	if (postFaults & 0x2000000) printf("EEPROM Data Out of Range!");
	if (postFaults & 0x4000000) printf("EEPROM Update Required!");
	if (postFaults & 0x40000000) printf("Brake Shorted!");
	if (postFaults & 0x80000000) printf("Brake Open!");	
	
	if (runFaults & 1) printf("Motor Over Speed!");
	if (runFaults & 2) printf("Over Current!");
	if (runFaults & 4) printf("Over Voltage!");
	if (runFaults & 8) printf("Inverter Over Temp!");
	if (runFaults & 0x10) printf("Accelerator Shorted!");
	if (runFaults & 0x20) printf("Accelerator Open!");
	if (runFaults & 0x40) printf("Direction Cmd Fault!");
	if (runFaults & 0x80) printf("Inverter Response Timeout!");
	if (runFaults & 0x100) printf("Hardware Desat Error!");
	if (runFaults & 0x200) printf("Hardware Overcurrent Fault!");
	if (runFaults & 0x400) printf("Under Voltage!");
	if (runFaults & 0x800) printf("CAN Cmd Message Lost!");
	if (runFaults & 0x1000) printf("Motor Over Temperature!");
	if (runFaults & 0x10000) printf("Brake Input Shorted!");
	if (runFaults & 0x20000) printf("Brake Input Open!");
	if (runFaults & 0x40000) printf("IGBT A Over Temperature!");
	if (runFaults & 0x80000) printf("IGBT B Over Temperature!");
	if (runFaults & 0x100000) printf("IGBT C Over Temperature!");
	if (runFaults & 0x200000) printf("PCB Over Temperature!");
	if (runFaults & 0x400000) printf("Gate Drive 1 Over Temperature!");
	if (runFaults & 0x800000) printf("Gate Drive 2 Over Temperature!");
	if (runFaults & 0x1000000) printf("Gate Drive 3 Over Temperature!");
	if (runFaults & 0x2000000) printf("Current Sensor Fault!");
	if (runFaults & 0x40000000) printf("Resolver Not Connected!");
	if (runFaults & 0x80000000) printf("Inverter Discharge Active!");
	
}

void handleCANMsgTorqueTimer(uint8_t *data)
{
	int cmdTorque, actTorque;
	uint32_t uptime;
	
	cmdTorque = data[0] + (data[1] * 256);
	actTorque = data[2] + (data[3] * 256);
	uptime = data[4] + (data[5] * 256) + (data[6] * 65536ul) + (data[7] * 16777216ul);
	printf("Torque Cmd: %d   Actual: %d     Uptime: %d", cmdTorque, actTorque, uptime);
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
	printf("Mod: %d  Weaken: %d   Id: %d   Iq: %d", modIdx, fieldWeak, IdCmd, IqCmd);
}

void handleCANMsgFirmwareInfo(uint8_t *data)
{
	int EEVersion, firmVersion, dateMMDD, dateYYYY;
    EEVersion = data[0] + (data[1] * 256);
	firmVersion = data[2] + (data[3] * 256);
    dateMMDD = data[4] + (data[5] * 256);
    dateYYYY = data[6] + (data[7] * 256);
	printf("EEVer: %d  Firmware: %d   Date: %d %d", EEVersion, firmVersion, dateMMDD, dateYYYY);
}

void handleCANMsgDiagnostic(uint8_t *data)
{
}




int main()
{   
    scpp::SocketCan sockat_can;
    
    if (sockat_can.open("vcan0") == scpp::STATUS_OK)
    {
    for (int j = 0; j < 20000; ++j)
    {
        scpp::CanFrame fr; //struct defined in socketcan_cpp.h
        
        while(sockat_can.read(fr) == scpp::STATUS_OK)
        {
            printf("len %d byte, id: %X, inverter msg:  %X   %X   %X   %X   %X   %X   %X  %X  \n", fr.len, fr.id, 
                fr.data[0], fr.data[1], fr.data[2], fr.data[3],
                fr.data[4], fr.data[5], fr.data[6], fr.data[7]);
                
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
                
            default: printf("nope");
                break;
        
        }
    }
}
}
    else
    {
        printf("Cannot open can socket!");
    }
    return 0;
}   
