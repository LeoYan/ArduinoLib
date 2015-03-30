/******************************************************************************

                        Demo for uHex

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
 ******************************************************************************/

/***************************** Function *************************************
 The uHex is in sleep state until pin2=LOW and pin3=HIGH.
 Once waked, the LED on the board will flash for some seconds and then go to sleep again.
 ******************************************************************************/


#include "microPoly.h"

void setup()
{
  
 /** Set waking condition from sleep state
  * use 'PolyuHex.addPinTrigger(pin, state, logic)'
  *  -pin: 2 or 3
  *  -state: HIGH or LOW
  *  -logic: OR or AND, it is optional, default is OR
  * **/

 pinMode( 2, INPUT_PULLUP);  //or INPUT mode based on what's the sensor
 pinMode( 3, INPUT);

 /* only 2 will trigger*/
 //PolyuHex.addPinTrigger( 2, LOW );

 /* only 3 will trigger*/
 //PolyuHex.addPinTrigger( 3, HIGH );

 /* Any 2 or 3 will trigger*/
 //PolyuHex.addPinTrigger( 2, LOW );
 //PolyuHex.addPinTrigger( 3, HIGH, OR);

 /* Both 2 and 3 will trigger */
 PolyuHex.addPinTrigger( 2, LOW );
 PolyuHex.addPinTrigger( 3, HIGH, AND);

 /** Set the inial overtime: it will go to sleep after 'overtime' senconds when there is not any trigger event
  * use 'PolyuHex.setOvertime(seconds)'
  *   -seconds: 0 to 255
  * note: the overtime will be reset to 0 after wake from sleep
  * **/
 PolyuHex.setRunOvertime(5);
 
 /** application init **/
 pinMode( 13, OUTPUT );

}


void loop() 
{
  /**your code**/
  digitalWrite(13, HIGH);
  delay(500);
  digitalWrite(13, LOW);
  delay(500);

  /**Goto sleep if timeout**/
  if ( PolyuHex.isRunOvertime() )
  {
    PolyuHex.sleep(); //the program will stop here until be waked, and run again from here
    PolyuHex.setRunOvertime(10);    //It will go to sleep after 10 senconds when there is not any trigger event
  }

}
