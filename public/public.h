/******************************************************************************

  Copyright (C) <2014>  <Leo Yan>

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
  Author        : Leo Yan
  Created       : 2014/2
  Last Modified : public head file
  Description   :
  Function List 1

******************************************************************************/


#ifndef _PUBLIC_H
#define _PUBLIC_H

#define _DEBUG 1


/****/
#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN 1234
#endif

#ifndef BIG_ENDIAN
#define BIG_ENDIAN 4321
#endif

#define ENDIANNESS LITTLE_ENDIAN


#if (ENDIANNESS == LITTLE_ENDIAN)
#define HTON16(val) ( val = (val>>8 & 0xFF) | (val<<8 & 0xFF00) )
#define NTOH16(val) HTON16(val)
#else
#define HTON16(val)
#define NTOH16(val)
#endif

#define INVALID_FLOAT 3.4e38
#define INVALID_INT16 -32768
#define INVALID_UINT8 0xFF
#define INVALID_UINT16 0xFFFF



#define INT16_MIN -32768
#define INT16_MAX 32767


/****/

#if _DEBUG
    #define DBG_BEGIN(baud)  { Serial.begin(baud); while(!Serial); }
    #define DBG_END  { Serial.end(); }

    /*assert */
    #define DBG_ASSERT(A) if( !(A) ) { Serial.print("ERR:F=");Serial.print(__FILE__);Serial.print(" L=");Serial.println(__LINE__); delay(1000); while(1);}
    #define DBG_WARNING(A) if( !(A) ) { Serial.print("WARNING:F=");Serial.print(__FILE__);Serial.print(" L=");Serial.println(__LINE__); delay(500);}

    /*print information*/
    #define DBG_PRINT_FUNNAME  { Serial.print("INFO:Fun=");Serial.println(__func__);}  //put it into the begin of function, you can trace the flow of program
    #define DBG_PRINT(info)  { Serial.print(info);}
    #define DBG_PRINTLN(info)  { Serial.println(info);}
    #define DBG_PRINTLN_VAR(var, type)  { Serial.print(#var);  Serial.print("=");Serial.print(var, type); Serial.print(" @"); Serial.println(type);}
    #define DBG_RUNCODE(code)  { code;}


#else
    #define DBG_BEGIN
    #define DBG_END

    #define DBG_ASSERT(A)
    #define DBG_WARNING(A)
    #define DBG_PRINT_FUNNAME
    #define DBG_PRINT(info)
    #define DBG_PRINTLN(info)
    #define DBG_PRINTLN_VAR(val, type)
    #define DBG_RUNCODE(code)

#endif




#endif
