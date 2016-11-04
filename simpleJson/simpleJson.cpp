#include <math.h>
#include <stdlib.h>
#include <public.h>
#include <float.h>
#include "simpleJson.h"

#define RETURN(condition) { if(condition) { DBG_WARNING(0);return false;} }
#define CALL_MAX 12




ParseString_class LightJson;

boolean ParseString_class::start()
{
    SimpleData.type = JSON_TYPE_Invalid;
    SimpleData.valueStr = String();

    storeIn = NODATA;
    level = 0;


    /**wait the start of Json**/
    int in;
    unsigned int startMillis, delta;

    startMillis = millis();

    do
    {
        in = getch();
//        Serial.print((char)in);
//        Serial.print(" ");


        if ( ('{' == in) || ('[' == in) )
        {
            incLevel();
            return true;
        }

        delta = (unsigned int)((unsigned int)millis() - startMillis);

    }while ( delta < 3000 );

    return false;
}

boolean ParseString_class::start(HardwareSerial &SerialPara)
{
    p_HardSerial = &SerialPara;
#ifdef USB_SERIAL
    p_UsbSerial = NULL;
#endif

    return start();
}

boolean ParseString_class::start(  WiFiClient &Client )
{
	p_WifiClient = &Client;

	p_HardSerial = NULL;
#ifdef USB_SERIAL
    p_UsbSerial = NULL;
#endif

    return start();
}

#ifdef USB_SERIAL
    boolean ParseString_class::start(Serial_ &SerialPara)
    {
        p_HardSerial = NULL;
        p_UsbSerial = &SerialPara;

        return start();
    }
#endif

boolean ParseString_class::getItem(JsonItem_struct &JsonItemPara, uint8_t &levelPara)
{
    static byte callCnt = 0;
    boolean ret;
    int in;

    String &itemName = JsonItemPara.name;
    uint8_t &itemDataType = JsonItemPara.data.type ;
    String &valueStr = JsonItemPara.data.valueStr;

    if ( 0 == level)
    {
        levelPara = level;
        itemDataType = JSON_TYPE_Invalid;
        return true;
    }


    if ( NODATA != storeIn )
    {
        in = storeIn;
        storeIn = NODATA;
    }
    else
    {
        in = getchSkip();
        RETURN( NODATA == in );
    }

    if ( ('{' == in) || ('[' == in) )
    {
        //todo if name<>null than set Item

        levelPara = level;
        incLevel();

        if ( itemName.length() > 0 )
        {
            itemDataType = ('{' == in) ? JSON_TYPE_Object : JSON_TYPE_Array;
            return true;
        }
        //else, there is no data

    }
    else if ( in == ':')
    {
        if ( JSON_TYPE_String == SimpleData.type )
        {
            itemName = SimpleData.valueStr;
            SimpleData.valueStr = String();
            SimpleData.type = JSON_TYPE_Invalid;
            /*continue to get Value*/
        }
        else
        {
            return false;
        }
    }
    else if ( (',' == in) || ('}' == in) || (']' == in) )
    {
        levelPara = level;

        if (',' != in) decLevel();

        if ( JSON_TYPE_Invalid != SimpleData.type )
        {
            JsonItemPara.data = SimpleData;
            SimpleData.valueStr = String();
            SimpleData.type = JSON_TYPE_Invalid;

            return true;
        }
        else
        {
            //else, there is no valid data, continue

        }
    }
    else
    {


        if ( parseSimpleData(in, SimpleData) )
        {
            //nothing
        }
        else
        {
            DBG_WARNING(0);
            return false;
        }



    }


    callCnt++;
    DBG_PRINTLN_VAR(callCnt,DEC);
    ret = ( callCnt > CALL_MAX )? false : getItem(JsonItemPara, levelPara);
    callCnt = 0;

    return ret;
}


boolean ParseString_class::parseSimpleData(int in, JsonData_struct & SimpleDataPara)
{
    boolean ret;
    DBG_PRINT_FUNNAME;

    if (in == '\"')
    {
        ret = parseString( SimpleDataPara.valueStr );
        if ( ret )
        {
            SimpleDataPara.type = JSON_TYPE_String;
        }
    }
    else if (in == '-' || (in >= '0' && in <= '9'))
    {
        ret = parseNumber( in, SimpleDataPara );
    }
    else if ( (in == 'f') || (in == 't') || (in == 'n'))
    {
        ret = parseWord( in, SimpleDataPara );
    }
    else
    {   /*unknow data, skip*/
        ret = true;
    }

    return ret; // failure.
}


// Parse the input text into an unescaped cstring, and populate item.
boolean ParseString_class::parseString(String &StrPara)
{
  //we do not need to skip here since the first byte should be '\"'
    char data;
    boolean ret;
    int in;
    uint16_t count;

    DBG_PRINT_FUNNAME;

    StrPara = "";
    ret = false;
    count = 0;



    do
    {
        /**get next**/
        in = getch();
        RETURN( NODATA == in );

        if('"' == in)
        {
            ret = true;
            break;
        }
        else if (in != '\\')
        {
            data = (char) in;
        }
        else
        {
            in = getch();
            RETURN( NODATA == in );

            switch (in)
            {
                case 'b':
                    data = '\b';
                    break;
                case 'f':
                    data = '\f';
                    break;
                case 'n':
                    data = '\n';
                    break;
                case 'r':
                    data = '\r';
                    break;
                case 't':
                    data = '\t';
                    break;
                default:  //'\\', '\"', '/' and understand char
                    data = (char)in;
                    break;
            }

        }

        StrPara += data;
        count++;

    }while ( count < JSON_STRING_MAXNUM );

    DBG_PRINTLN_VAR(count,DEC);
    DBG_PRINTLN_VAR(ret,DEC);

    return ret;
}


boolean ParseString_class::parseWord ( int in, JsonData_struct & SimpleDataPara )
{
    /**check only the first char**/
    uint8_t count = 0;
    int data;

    DBG_PRINT_FUNNAME;

    do
    {
        data = getch();
        RETURN( NODATA == data );

        count++;

        if ( count > 8 )
        {
            return false;
        }

    }while( !((',' == data) || ('}' == data) || (']' == data)) );


    if ( 'f' == (char)in )
    {
        SimpleDataPara.type = JSON_TYPE_Boolean;
        SimpleDataPara.valuebool = false;
    }
    else if ( 't' == (char)in )
    {
        SimpleDataPara.type = JSON_TYPE_Boolean;
        SimpleDataPara.valuebool = true;
    }
    else
    {
        SimpleDataPara.type = JSON_TYPE_NULL;
    }

    storeIn = data;

    return true;
}


boolean ParseString_class::parseNumber ( int in, JsonData_struct & SimpleDataPara )
{
    int i = 0;
    int sign = 1;

    DBG_PRINT_FUNNAME;

    // It is easier to decode ourselves than to use sscnaf,
    // since so we can easier decide between int & double
    if (in == '-')
    {
        //it is a negative number
        sign = -1;
        in = getch();
        RETURN( NODATA == in );
    }

    if (in >= '0' && in <= '9') do
    {
        i = (i * 10) + (in - '0');
        in = getch();
    }while (in >= '0' && in <= '9'); // Number?


    //end of integer part � or isn't it?
    if (!(in == '.' || in == 'e' || in == 'E'))
    {
        SimpleDataPara.valueint = i * (int) sign;
        SimpleDataPara.type = JSON_TYPE_Int;
    }
    else  //ok it seems to be a double
    {
        double n = (double) i;
        int scale = 0;
        int subscale = 0;
        char signsubscale = 1;
        if (in == '.')
        {
            in = getch();
            do
            {
                n = (n * 10.0) + (in - '0'), scale--;
                in = getch();
            }while (in >= '0' && in <= '9');
        } // Fractional part?

        if (in == 'e' || in == 'E') // Exponent?
        {
            in = getch();
            if (in == '+')
            {
                in = getch();
            }
            else if (in == '-')
            {
                signsubscale = -1;
                in = getch();
            }
            while (in >= '0' && in <= '9')
            {
                subscale = (subscale * 10) + (in - '0'); // Number?
                in = getch();
            }
        }

        n = sign * n
                * pow( 10.0,
                       ((double) scale
                               + (double) subscale * (double) signsubscale) ); // number = +/- number.fraction * 10^+/- exponent

        SimpleDataPara.valuefloat = n;
        SimpleDataPara.type = JSON_TYPE_Float;
    }
    //preserve the last character for the next routine
    if (in > 32)
    {
        storeIn = in;
    }

    return true;
}

#if 0

int ParseString_class::getch()
{
    uint16_t count = 0;


    do
    {

            if (p_HardSerial->available()>0)
            {
                char ctmp;
                ctmp = p_HardSerial->read();
                Serial.print(ctmp);

                return ctmp;
            }

            count++;



    }while (count < 10000 );

    return NODATA;
}

#endif

int ParseString_class::getch()
{
    unsigned int startMillis, delta;

    startMillis = millis();

    do
    {
#ifdef USB_SERIAL
        if ( NULL != p_UsbSerial )
        {
            if (p_UsbSerial->available())
            {
                return p_UsbSerial->read();
            }
        }
        else
        {

#endif

        	if (NULL != p_WifiClient)
        	{
                if (p_WifiClient->available())
                {
                    return p_WifiClient->read();
                }

        	}
        	else if ( NULL != p_HardSerial )
        	{
                if (p_HardSerial->available())
                {
                    return p_HardSerial->read();
                }

        	}
        	else
        	{
        		//nothing
        	}

#ifdef USB_SERIAL
        }
#endif
        delta = (unsigned int)((unsigned int)millis() - startMillis);

    }while (delta < 10);

    return NODATA;
}

// Utility to jump whitespace and cr/lf

int ParseString_class::getchSkip()
{
    int in;

    do
    {
        in = getch();
    }while ( (in != NODATA) && ((in <= 32) || (in > 127)) );

    return in;

}


