/*
 * RMSMotorController.h
 *
 *
 Copyright (c) 2017 Collin Kidder

 Permission is hereby granted, free of charge, to any person obtaining
 a copy of this software and associated documentation files (the
 "Software"), to deal in the Software without restriction, including
 without limitation the rights to use, copy, modify, merge, publish,
 distribute, sublicense, and/or sell copies of the Software, and to
 permit persons to whom the Software is furnished to do so, subject to
 the following conditions:

 The above copyright notice and this permission notice shall be included
 in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 */

#ifndef RMSCTRL_H_
#define RMSCTRL_H_

#define CFG_TICK_INTERVAL_MOTOR_CONTROLLER		40000

/*
 * Class for Rinehart PM Motor Controller specific configuration parameters
 */
 
enum PowerMode {
	modeTorque,
	modeSpeed
};


void sendCmdFrame();
void handleCANMsgTemperature1(uint8_t *data);
void handleCANMsgTemperature2(uint8_t *data);
void handleCANMsgTemperature3(uint8_t *data);
void handleCANMsgAnalogInputs(uint8_t *data);
void handleCANMsgDigitalInputs(uint8_t *data);
void handleCANMsgMotorPos(uint8_t *data);
void handleCANMsgCurrent(uint8_t *data);
void handleCANMsgVoltage(uint8_t *data);
void handleCANMsgFlux(uint8_t *data);
void handleCANMsgIntVolt(uint8_t *data);
void handleCANMsgIntState(uint8_t *data);
void handleCANMsgFaults(uint8_t *data);
void handleCANMsgTorqueTimer(uint8_t *data);
void handleCANMsgModFluxWeaken(uint8_t *data);
void handleCANMsgFirmwareInfo(uint8_t *data);
void handleCANMsgDiagnostic(uint8_t *data);

int16_t throttleRequested; // -1000 to 1000 (per mille of throttle level)
int16_t speedRequested; // in rpm
int16_t speedActual; // in rpm
int16_t torqueRequested; // in 0.1 Nm
int16_t torqueActual; // in 0.1 Nm
int16_t torqueAvailable; // the maximum available torque in 0.1Nm

uint16_t outVoltage; // Out voltage
uint16_t dcVoltage; // DC voltage in 0.1 Volts
int16_t dcCurrent; // DC current in 0.1 Amps
uint16_t acCurrent; // AC current in 0.1 Amps

int16_t temperatureMotor; // temperature of motor in 0.1 degree C
int16_t temperatureInverter; // temperature of inverter power stage in 0.1 degree C
int16_t temperatureSystem; // temperature of controller in 0.1 degree C



uint16_t prechargeTime; //time in ms that precharge should last
uint32_t milliStamp; //how long we have precharged so far

uint32_t skipcounter;

bool isEnabled;
bool isLockedOut;
bool isCANControlled;
PowerMode powerMode;
bool faulted;
int torqueCommand;
 

#endif /* RMSCTRL_H_ */



