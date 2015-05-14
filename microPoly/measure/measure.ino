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
 1)The uHex is in sleep state until Timer is overtime or Button is pressed
 1)The uHex sample data every 1 minute. If button is pressed the average data will be display
   on the LCD screen. After some seconds, it will go to sleep again.

 ***************************** Hardware *************************************
  uHex Pin  --  Module                      --     Library
    2           Button  (High lever when pressed)
    9           DHT11                              http://www.dfrobot.com/index.php?route=product/product&product_id=174&search=dht11&description=true
   I2C          LCD1602                            http://www.dfrobot.com/index.php?route=product/product&product_id=135&search=lcd&description=true
 ******************************************************************************/

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <dht11.h>

#include "microPoly.h"


/** Global varibles **/
#define DHT_PIN 9
dht11 myDHT;
LiquidCrystal_I2C myLCD(0x20,16,2);


#define SAMPLE_NUM 3
#define INVALID_DATA (-200)

typedef struct
{
  int temperature;
  int humidity;
}Data_struct;

typedef struct
{
  char index;
  Data_struct data[SAMPLE_NUM];
}DataStore_struct;

DataStore_struct DataStore;


/**Main program**/
void setup()
{
 /**set waking condition from sleep state**/
 /*Button*/
 pinMode( 2, INPUT);      //or INPUT_PULLUP
 PolyuHex.addPinTrigger( 2, HIGH );

 /*Timer*/
 PolyuHex.setTimer(TIMER_MINUTE, 1);

 /** application init **/
 DataStore_init();

 initModule();

}

void loop()
{
  int errorCode, data;

  /**Deal timer, sample data from DHT22 and save data**/
  if ( PolyuHex.isTimerOut() )
  {
    /**get data**/
    errorCode = myDHT.read(DHT_PIN);

    /**save data**/
    if ( DHTLIB_OK == errorCode )
    {
      DataStore_putData(myDHT.temperature, myDHT.humidity);
    }


    /**set overtime before sleep**/
    PolyuHex.setRunOvertime(0);
  }


  /**deal button, display average temperature and humidity from DataStore**/
  if ( PolyuHex.isPinTriggered() )
  {
    /**show data**/
    myLCD.clear();
    myLCD.backlight();
    myLCD.setCursor(1, 0);
    myLCD.print("TEMP: ");

    data = DataStore_getTemperatureAverage();
    if ( INVALID_DATA != data )
    {
        myLCD.print(data);
        myLCD.print("C");
    }
    else
    {
      myLCD.print("Sorry");
    }


    myLCD.setCursor(1, 1);
    myLCD.print("  RH: ");

    data = DataStore_getHumidityAverage();
    if ( INVALID_DATA != data )
    {
        myLCD.print(data);
        myLCD.print("%");
    }
    else
    {
      myLCD.print("Sorry");
    }

    /**set overtime before sleep**/
    PolyuHex.setRunOvertime(15);
  }


  /**Goto sleep if timeout. Overtime will use the longest time if it is setted by different value**/
  if ( PolyuHex.isRunOvertime() )
  {
    PolyuHex.sleep();  //the program will stop here until be waked, and run again from here

    /**some modules need reinitialize after wake**/
    initModule();
  }

}

/**Fuctions**/
void initModule()
{
  myLCD.init();
  myLCD.noBacklight();
}



void DataStore_init()
{
  DataStore.index = 0;
  for ( char i = 0; i < SAMPLE_NUM; i++ )
  {
    DataStore.data[i].temperature = INVALID_DATA;
    DataStore.data[i].humidity = INVALID_DATA;

  }
}

void DataStore_putData( int temperature, int humidity )
{
  char &index = DataStore.index;

  DataStore.data[index].temperature = temperature;
  DataStore.data[index].humidity = humidity;
}

int DataStore_getTemperatureAverage()
{
  int temperature, data;
  char num;

  data = 0;
  num = 0;

  for ( char i = 0; i < SAMPLE_NUM; i++ )
  {
    temperature = DataStore.data[i].temperature;
    if ( temperature != INVALID_DATA )
    {
      data += temperature;
      num++;
    }
  }

  if ( 0 == num )
  {
    return INVALID_DATA;
  }
  else
  {
    return (data/num);
  }

}

int DataStore_getHumidityAverage()
{
  int humidity, data;
  char num;

  data = 0;
  num = 0;

  for ( char i = 0; i < SAMPLE_NUM; i++ )
  {
    humidity = DataStore.data[i].humidity;
    if ( humidity != INVALID_DATA )
    {
      data += humidity;
      num++;
    }
  }

  if ( 0 == num )
  {
    return INVALID_DATA;
  }
  else
  {
    return (data/num);
  }

}
