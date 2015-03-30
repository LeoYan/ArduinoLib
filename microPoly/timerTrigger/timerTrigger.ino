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
 The uHex is waked by period timer (using watchdog timer).
 Once waked, the LED on the board will flash for some seconds and then go to sleep again.
 ******************************************************************************/

#include "microPoly.h"

void setup()
{
  pinMode( 13, OUTPUT );

  /**set waking condition from sleep state**/
  /** setTimer()
    *  TIMER_SECOND  [1-59]
    *  TIMER_MINUTE  [1-59]
    *  TIMER_HOUR    [1-12]
  **/
 PolyuHex.setTimer(TIMER_SECOND, 8);

 /** Set the inial overtime: it will go to sleep after 'overtime' senconds when there is not any trigger event
  * use 'PolyuHex.setOvertime(seconds)'
  *   -seconds: 0 to 255
  * note: the overtime will be reset to 0 after wake from sleep
  * **/
 PolyuHex.setRunOvertime(0);
}

void loop()
{

  digitalWrite( 13, LOW);
  delay(100);
  digitalWrite( 13, HIGH);
  delay(100);

  /**Overtime will use the longest time if it is setted by different**/
  if ( PolyuHex.isRunOvertime() )
  {
    PolyuHex.sleep();
    PolyuHex.setRunOvertime(2);
  }


}
