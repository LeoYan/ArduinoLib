/******************************************************************************
                        Low power library for uHex

  Copyright (C) <2015>  <Leo Yan>

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

  Contact: leo.yan.cn@gmail.com

 ******************************************************************************
  Version       : V1.0
  Author        : Leo Yan
  Created       : 2015/1
  Last Modified :
  Description   : The library is depend on Arduino environment.
  Function List :

******************************************************************************/

#ifndef _MICROPOLY_H
#define _MICROPOLY_H



#include <wiring_private.h>
#include <avr/sleep.h>
#include <avr/io.h>
#include <stdint.h>


#define WAKING_OVERTIME_DEFAULT 10;

#define INVALID_UINT8 0xFF

#ifndef AND
#define AND 0x0
#endif

#ifndef OR
#define OR 0x1
#endif

typedef enum
{
  TIMER_SECOND,
  TIMER_MINUTE,
  TIMER_HOUR
}TimerType_enum;

#define MICROPOLY_PORT_NUM 3

typedef struct
{
  uint8_t triggerMode;
  boolean state;
}Interrupt_struct;

//typedef void (*VoidFunc)(void);


/// @class	<#Description#>

class MicroPoly_class {
  
public:

  MicroPoly_class();
  void addPinTrigger( uint8_t pin, uint8_t condition, uint8_t logic = OR );
  void setRunOvertime( uint8_t seconds );   //0: cancel overtime
  boolean isRunOvertime();
  void sleep();
  boolean isPinTriggered( void );
  void pinInterrupt();

  
  void setTimer(TimerType_enum, uint8_t time);
  boolean isTimerOut();
  void timerInterrupt();

private:
  
  Interrupt_struct a_InterruptManager[EXTERNAL_NUM_INTERRUPTS];
  uint8_t interruptLogic;  //the logic betweeen Interrupters: AND, OR
  volatile boolean wakeEventFlag;
  
  static const uint8_t a_interruptMapPin[EXTERNAL_NUM_INTERRUPTS];
  
  uint8_t wakingOvertime;   //unit: second
  uint16_t startTime;        //unit: second
  boolean pinTriggerState;
  
  uint16_t timerCntOut;
  uint16_t timerState;

  /** extern INT,  pin2 3 **/
  void disableINT();
  void enableINT();


  void updatePinTriggerState();
  boolean checkPinTriggered( void );



  /**Timer**/

  
  /**physical opration**/
  static const uint8_t a_ioPort[MICROPOLY_PORT_NUM];        //the port of Poly
  static const uint8_t a_ioMask[MICROPOLY_PORT_NUM]; //the mask of Poly's pin
  
  uint8_t a_preDDRValue[MICROPOLY_PORT_NUM];
  uint8_t a_prePORTValue[MICROPOLY_PORT_NUM];
  uint8_t preUCSR0BValue;
  uint8_t preTWCRValue;
  uint8_t preADCSRAValue;
  
  void disablePort();
  void resumePort();
  
};


extern MicroPoly_class PolyuHex;

#ifdef __cplusplus
   extern "C"{
#endif
void userFunINT( void );

#ifdef __cplusplus
}
#endif

#endif
