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

#include "MicroPoly.h"

MicroPoly_class PolyuHex;

/*
Board	int.0	int.1	int.2	int.3	int.4	int.5
Uno, Ethernet	2	3
Mega2560	2	3	21	20	19	18
Leonardo	3	2	0	1	7
*/
const uint8_t MicroPoly_class::a_interruptMapPin[EXTERNAL_NUM_INTERRUPTS] = {2, 3};

const uint8_t MicroPoly_class::a_ioPort[MICROPOLY_PORT_NUM] = { 2, 3, 4 };   //POR:TB C D
const uint8_t MicroPoly_class::a_ioMask[MICROPOLY_PORT_NUM] = { 0b101110, 0b0111111, 0b100011 };  //include SCK(PB5); exclude D7(PD7), D2(PD2), D3(PD3)

/*******************************************************************************
* Fuction: init class
* Input:
* return:
* note       :
*******************************************************************************/
MicroPoly_class::MicroPoly_class() {

  uint8_t i;
  
  for ( i = 0; i < EXTERNAL_NUM_INTERRUPTS; i++ )
  {
    a_InterruptManager[i].state = INVALID_UINT8;
  }
  
  wakeEventFlag = false;
  wakingOvertime = 0;
  pinTriggerState = true;  //it will be run when start
  timerState = true;  //it will be run when start

  interruptLogic = OR;

  noInterrupts( );
  
  startTime = millis()/1000;
  
}

#if defined(BODS) && defined(BODSE)
#ifndef sleep_bod_disable
#define sleep_bod_disable() \
do { \
  uint8_t tempreg; \
  __asm__ __volatile__("in %[tempreg], %[mcucr]" "\n\t" \
                       "ori %[tempreg], %[bods_bodse]" "\n\t" \
                       "out %[mcucr], %[tempreg]" "\n\t" \
                       "andi %[tempreg], %[not_bodse]" "\n\t" \
                       "out %[mcucr], %[tempreg]" \
                       : [tempreg] "=&d" (tempreg) \
                       : [mcucr] "I" _SFR_IO_ADDR(MCUCR), \
                         [bods_bodse] "i" (_BV(BODS) | _BV(BODSE)), \
                         [not_bodse] "i" (~_BV(BODSE))); \
} while (0)
#endif
#endif

/*******************************************************************************
* Fuction: set AVR to Power-down sleep state
* Note   :
*******************************************************************************/
void MicroPoly_class::sleep()         // here we put the arduino to sleep
{
  wakeEventFlag = false;
  
  cli();
  
  disablePort();
  
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);   // sleep mode is set here
  
  sleep_enable();          // enables the sleep bit in the mcucr register
  
  //so sleep is possible. just a safety pin
    
  enableINT();
  
  
  do
  {
    sleep_bod_disable();
    sei();
    sleep_cpu();   // here the device is actually put to sleep!!
		// THE PROGRAM CONTINUES FROM HERE AFTER WAKING UP
    delay(1);  //if there is no delay, wakeEventFlag always false, why?
    
  }while(!wakeEventFlag);

  sleep_disable();         // first thing after waking from sleep:  disable sleep...
  
  /**recovery**/

  resumePort();
  delay(25);  //wait for stable power

  wakingOvertime = 0;
  startTime = millis()/1000;
}


/*******************************************************************************
* Fuction: add pin-trigger to system; it only supports INT0/pin2 and INT1pin3
* Input:
*    --pin:  2,3
*    --mode: HIGH,LOW
*    --logic: AND, OR;  (trigger condition = pin2 logic pin3)
* return: void
* note       :
*******************************************************************************/
void MicroPoly_class::addPinTrigger( uint8_t pin, uint8_t mode, uint8_t logic )
{
  boolean result;
  uint8_t interruptNum;
  
  result = false;
  
  for ( interruptNum = 0; interruptNum < EXTERNAL_NUM_INTERRUPTS; interruptNum++ )
  {
    if ( pin == a_interruptMapPin[interruptNum] )
    {
      a_InterruptManager[interruptNum].triggerMode = mode;
      a_InterruptManager[interruptNum].state = false;
      
      if ( AND == logic )
      {
        interruptLogic = logic;
      }

      
      attachInterrupt( interruptNum, userFunINT, CHANGE );
      
      result = true;
      break;
    }
  }

}

/*******************************************************************************
* Fuction: set overtime to sleep.
* Input:
*    --seconds:  [0,255]
* return: void
* note1       :  the overtime is 0 after wake
* note1       :  If it is setted by many times, it will keep the longest overtime.
*******************************************************************************/
void MicroPoly_class::setRunOvertime( uint8_t seconds )
{

  startTime = millis()/1000;

  if ( seconds > wakingOvertime )
  {
    wakingOvertime = seconds;
  }

}

/*******************************************************************************
* Fuction: check whether overtime.
* return: true or false
*******************************************************************************/
boolean MicroPoly_class::isRunOvertime( void )
{
  boolean ret;

  ret = false;

  if ((uint8_t)((millis()/1000) - startTime) >= wakingOvertime )
  {
    ret = true;
  }

  return ret;
}

/******************************Pin Trigger *************************************/

/*******************************************************************************
* Fuction: get triggered state base on addPinTrigger() conditions.
* return: true or false
*******************************************************************************/
boolean MicroPoly_class::isPinTriggered()
{

  if ( pinTriggerState )
  {
    cli();
    pinTriggerState = false;
    sei();

    return true;
  }
  else
  {
    return false;
  }

}

/*******************************************************************************
* Fuction: check whether triggered base on addPinTrigger() conditions.
* return: true or false
*******************************************************************************/
boolean MicroPoly_class::checkPinTriggered()
{
  uint8_t interruptNum, state;
  boolean isTriggered;

  isTriggered = false;

  for ( interruptNum = 0; interruptNum < EXTERNAL_NUM_INTERRUPTS; interruptNum++ )
  {
    state = a_InterruptManager[interruptNum].state;

    if ( AND == interruptLogic )
    {
      if ( false == state )
      {
        isTriggered = false;
        break;
      }

      isTriggered = true;
    }
    else
    {
      if ( true == state )
      {
        isTriggered = true;
        break;
      }
    }

  }

  return isTriggered;

}




/*******************************************************************************
* Fuction: disable ports and modules.
* return: void
*******************************************************************************/
void MicroPoly_class::disablePort(void)
{

  /**close port**/
  uint8_t i;
  volatile uint8_t *out, *ddr;

  /**close ports**/
  for( i = 0; i < MICROPOLY_PORT_NUM; i++ )
  {
    ddr = portModeRegister(a_ioPort[i]);
    out = portOutputRegister(a_ioPort[i]);
    
    a_preDDRValue[i] = *ddr;
    a_prePORTValue[i] = *out;
    
    (*ddr) &= ~a_ioMask[i];
    (*out) &= ~a_ioMask[i];
    
  }

  
  /**close I2C**/
  preTWCRValue = TWCR;
  cbi(TWCR, TWEN);

  /**close uart **/
  preUCSR0BValue = UCSR0B;
  
  cbi(UCSR0B, TXEN0);
  cbi(UCSR0B, RXEN0);
  cbi(UCSR0B, TXCIE0);
  
  /**shutdown power**/
  DDRD |= _BV(7);
  PORTD |= _BV(7);
  
  /**close ADC**/
  preADCSRAValue = ADCSRA;
  cbi(ADCSRA, ADEN);
}


void MicroPoly_class::resumePort(void)
{
  uint8_t i;
  volatile uint8_t *out, *ddr;
  
  /**inner recover**/
  ADCSRA |=(preADCSRAValue & _BV(ADEN));
  
  /**First: Enable Power**/
  DDRD |= _BV(7);
  PORTD &= ~_BV(7);
  
  /**recovery ports**/
  for( i = 0; i < MICROPOLY_PORT_NUM; i++ )
  {
    ddr = portModeRegister(a_ioPort[i]);
    out = portOutputRegister(a_ioPort[i]);

    *out = a_prePORTValue[i];
    *ddr = a_preDDRValue[i];
  }
  
  /**resume uart **/
  UCSR0B |= ( preUCSR0BValue & (_BV(TXEN0) | _BV(RXEN0) | _BV(TXCIE0)) );
  
  /**resume I2C**/
  TWCR |= (preTWCRValue & _BV(TWEN));
  
}

void MicroPoly_class::enableINT( void )
{
  uint8_t i;
  
  for ( i = 0; i < EXTERNAL_NUM_INTERRUPTS; i++ )
  {
    if ( INVALID_UINT8 != a_InterruptManager[i].state )
    {
       EIMSK |= (1<<i);
    }
  }

}


void MicroPoly_class::disableINT( void )
{
  uint8_t i;

  for ( i = 0; i < EXTERNAL_NUM_INTERRUPTS; i++ )
  {
    if ( INVALID_UINT8 != a_InterruptManager[i].state )
    {
       EIMSK &= ~(1 << i);
    }
  }
}

void MicroPoly_class::updatePinTriggerState()
{
  uint8_t pin, triggerMode;
  uint8_t interruptNum;
  uint8_t isTriggered;

  isTriggered = false;

  for ( interruptNum = 0; interruptNum < EXTERNAL_NUM_INTERRUPTS; interruptNum++ )
  {
    boolean &state = a_InterruptManager[interruptNum].state;
    if ( INVALID_UINT8 !=  state )
    {
      pin = a_interruptMapPin[interruptNum];
      triggerMode = a_InterruptManager[interruptNum].triggerMode;

      state = (digitalRead( pin ) == triggerMode);
    }
  }
}



void MicroPoly_class::pinInterrupt()
{

  updatePinTriggerState();

  if ( checkPinTriggered() )
  {
    wakeEventFlag = true;
    pinTriggerState = true;
  }

}
             
             
             
     
void userFunINT( void )
{
  PolyuHex.pinInterrupt();
}

/******************************Watchdog Timer Trigger *************************************/

#if defined(WDP3)
# define _WD_PS3_MASK       _BV(WDP3)
#else
# define _WD_PS3_MASK       0x00
#endif

#if defined(WDTCSR)
#  define _WD_CONTROL_REG     WDTCSR
#else
#  define _WD_CONTROL_REG     WDTCR
#endif

#if defined(WDTOE)
#define _WD_CHANGE_BIT      WDTOE
#else
#define _WD_CHANGE_BIT      WDCE
#endif

#define wdt_enableInterrupt(value)   \
__asm__ __volatile__ (  \
    "in __tmp_reg__,__SREG__" "\n\t"    \
    "cli" "\n\t"    \
    "wdr" "\n\t"    \
    "sts %0,%1" "\n\t"  \
    "out __SREG__,__tmp_reg__" "\n\t"   \
    "sts %0,%2" "\n\t" \
    : /* no outputs */  \
    : "M" (_SFR_MEM_ADDR(_WD_CONTROL_REG)), \
    "r" (_BV(_WD_CHANGE_BIT) | _BV(WDE) ), \
    "r" ((uint8_t) ((value & 0x08 ? _WD_PS3_MASK : 0x00) | \
        _BV(WDIE) | (value & 0x07)) ) \
    : "r0"  \
)

/*******************************************************************************
* Fuction: init Timer (watchdog timer).
* input:
*   --type: see TimerType_enum
*   --time: [1-59]@TIMER_SECOND, [1-59]@TIMER_MINUTE, [1-12]@TIMER_HOUR
* return: true or false
*******************************************************************************/
void MicroPoly_class::setTimer(TimerType_enum type, uint8_t time)
{
  uint8_t i, period, prescaler;
  uint16_t seconds;

  if ( 0 == time )
  {
    return;
  }

  switch (type)
  {
    case TIMER_SECOND:
      seconds = (time < 60) ? time : 59;

      break;

    case TIMER_MINUTE:
      seconds = (time < 60) ? ((uint16_t)time * 60) : (59 * 60);

      break;

    case TIMER_HOUR:
      seconds = (time <= 12) ? ((uint16_t)time * 3600) : (12 * 3600);

      break;

    default:
      return;
  }

  /**  prescaler, time-out
   *   0b0110,      1s
   *   0b0111,      2s
   *   0b1000,      4s
   *   0b1001,      8s
   * **/

  for ( i = 0; i <= 3; i++)
  {
    period = 8>>i;
    if ( 0 == seconds%period )
    {
      break;
    }
  }

  timerCntOut = seconds/period;

  prescaler = 0b1001 - i;

  MCUSR &= ~(1<<WDRF);

  wdt_enableInterrupt(prescaler);

}


boolean MicroPoly_class::isTimerOut()
{
  uint8_t state;

  state = timerState;
  timerState = false;

  return state;
}






/*******************************************************************************
*
* Description: Watchdog Timer interrupt service routine. This routine is
*		           required to allow automatic WDIF and WDIE bit clearance in
*			         hardware.
*
*******************************************************************************/
ISR (WDT_vect)
{
  PolyuHex.timerInterrupt();
}

void MicroPoly_class::timerInterrupt()
{
  static uint16_t count = 0;

  count++;

  if (count >= timerCntOut)
  {
    timerState = true;
    wakeEventFlag = true;

    count = 0;
  }
}



