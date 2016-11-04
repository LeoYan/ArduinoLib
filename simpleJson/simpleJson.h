/*
 Copyright (c) 2001, Interactive Matter, Marcus Nowotny

 Based on the cJSON Library, Copyright (C) 2009 Dave Gamble
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 */

#ifndef JSON_TYPE__h
#define JSON_TYPE__h

#include <Arduino.h>  // To get access to the Arduino millis() function
#include <ctype.h>
#include <string.h>
#include <ESP8266WiFi.h>


#if defined(USBCON)
#ifdef CDC_ENABLED
#define USB_SERIAL
#endif
#endif

/******************************************************************************
 * Definitions
 ******************************************************************************/
// aJson Types:
#define JSON_TYPE_NULL 0
#define JSON_TYPE_Boolean 1
#define JSON_TYPE_Int 2
#define JSON_TYPE_Float 3
#define JSON_TYPE_String 4
#define JSON_TYPE_Array 5
#define JSON_TYPE_Object 6
#define JSON_TYPE_Invalid 0xFF

#ifndef NODATA
#define NODATA -1
#endif

#define JSON_NAME_MAXNUM 32
#define JSON_STRING_MAXNUM 128   //include Name


/****/
// The aJson structure:


typedef struct JsonData {
    uint8_t type;
    String valueStr;
    union {
        boolean valuebool; //the items value for true & false
        int valueint; // The item's value, if type==JSON_TYPE_Int
        double valuefloat; // The item's value, if type==JSON_TYPE_Float
    };
}JsonData_struct;


typedef struct JsonItem {
    String name; // The item's name string, if this item is the child of, or is in the list of subitems of an object.
    JsonData_struct data;
}JsonItem_struct;


class ParseString_class {
public:
    boolean start(WiFiClient &);   //start a process
    boolean start(HardwareSerial &);   //start a process
#ifdef USB_SERIAL
    boolean start(Serial_ &);   //start a process
#endif

    boolean getItem(JsonItem_struct &JsonItemPara, uint8_t &levelPara);


    void stop(){level=0; SimpleData.valueStr="";};


private:
    HardwareSerial *p_HardSerial;
#ifdef USB_SERIAL
    Serial_ *p_UsbSerial;
#endif
    WiFiClient *p_WifiClient;

    uint8_t level;  //{, [ = increase;         ] } = decrease
    int storeIn;            //reserved char for getItem, because there is not indicator for some data( int ,float)

    JsonData_struct SimpleData;  //store simple data from Name or Value
    char a_Buffer[JSON_STRING_MAXNUM];


    int getch();
    int getchSkip();
    inline void incLevel(){level++;};
    inline void decLevel(){level--;};

    inline boolean start();

    boolean parseSimpleData(int in, JsonData_struct & SimpleDataPara);
    boolean parseString(String &StrPara);
    boolean parseWord ( int in, JsonData_struct & SimpleDataPara );
    boolean parseNumber ( int in, JsonData_struct & SimpleDataPara );
};

extern ParseString_class LightJson;


#endif
