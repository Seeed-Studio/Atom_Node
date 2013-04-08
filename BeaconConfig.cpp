/*
  BeaconConfig.cpp
  2012 Copyright (c) Seeed Technology Inc.  All right reserved.

  Author:Loovee
  2012-12-3

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <EEPROM.h>
#include <BeaconSensor.h>
#include <BeaconActuator.h>
#include <BeaconActuatorDfs.h>
#include <BeaconSensorDfs.h>
#include <LightCom.h>
#include <I2C_Uart.h>
#include <BeaconDrive.h>

#include "BeaconApplication.h"
#include "BeaconConfig.h"
#include "BeaconConfigDfs.h"
#include "BeaconApplicationDfs.h"

/*********************************************************************************************************
** Function name:           init
** Descriptions:            init
*********************************************************************************************************/
void BeaconConfig::init()
{
    __printlnCfg("CONFIG: INIT!");
    
    ifCloud     = 0;

    freqSensor  = EEPROM.read(EEPADDFREQBROADCAST);
    freqSensor  = freqSensor == 0 ? BDF100MS : freqSensor;
    ifSetDevice = EEPROM.read(EEPADDISSET);
    
    if(ifSetDevice != 0x55)
    {
        __printlnCfg("CONFIG: DEVICE NOT CONFIG!");
        return;
    }

    idDevice = EEPROM.read(EEPADDDEVICEID);
    __printCfg("CONFIG: idDevice = ");
    __printlnCfg(idDevice);
    /*
     *  config sensor
     */
    ifSetSensor = EEPROM.read(EEPADDIFSETSE);
    if(ifSetSensor == 0x55)
    {
        idSensor    = EEPROM.read(EEPADDSENSORID);                      // id of sensor
        freqSensor  = EEPROM.read(EEPADDFREQBROADCAST);

    }
    else
    {
        __printlnCfg("CONFIG: SENSOR NOT CONFIG");
    }

    /*
     *  config actuator
     */

    ifSetActuator = EEPROM.read(EEPADDIFSETAC);
    __printCfg("ifSetActuator = ");__printlnCfg(ifSetActuator);

    if(ifSetActuator == 0x55)        // been config
    {
        idActuator = EEPROM.read(EEPADDACTUATORID);
        nTC        = EEPROM.read(EEPADDACTCN);
        
        __printCfg("idActuator = ");__printlnCfg(idActuator);
        __printCfg("nTC = ");__printlnCfg(nTC);
        
        int eepAddOffCnt = 0;
        for(int i = 0; i<nTC; i++)
        {
            TC[i][eepAddOffCnt] = EEPROM.read(EEPADDTCSTART + eepAddOffCnt);        // len of each tc
            delay(5);
            unsigned char cnTmp = TC[i][eepAddOffCnt++];

            __printCfg("cnTmp = ");__printlnCfg(cnTmp);

            for(int j = 0; j<cnTmp; j++)                                            // read other data of tc
            {
                TC[i][eepAddOffCnt++] = EEPROM.read(EEPADDTCSTART + eepAddOffCnt);
                delay(5);
            }
        }

        if(TC[0][0] > 10)
        {
            TC[0][0] = 10;
        }

        __printCfg("CONFIG: IDACTUATOR:\t");
        __printlnCfg(idActuator);
        __printCfg("CONFIG: NTC:\t");
        __printlnCfg(nTC);

        __printCfg("TC[i][0] = ");__printlnCfg(TC[0][0]);
        for(int i = 0; i<nTC; i++)
        {
            for(int j = 0; j<TC[i][0]+1; j++)
            {
                __printCfg(TC[i][j]);
                __printCfg("\t");
            }
            __printlnCfg();
        }
    }
    else
    {
        __printlnCfg("CONFIG:ACTUATOR HAS NOT BEEN CONFIGED!");
    }

}

/*********************************************************************************************************
** Function name:           isLightConfig
** Descriptions:            if get data from light sensor
*********************************************************************************************************/
unsigned char BeaconConfig::isLightConfig()
{
    if(LightCom1.isData())
    {
        return 1;
    }
    return 0;

}

/*********************************************************************************************************
** Function name:           lightConfig
** Descriptions:            config
*********************************************************************************************************/
bool BeaconConfig::lightConfig()
{
    
    if(!LightCom1.isData())
    {
        return 0;
    }
    unsigned char lenLight = LightCom1.Recv(dtaLight);

    if((dtaLight[0] == 0x51) && (lenLight == 2))                    // config device id
    {
        __printlnCfg("CONFIG: get config device id!");
        __printCfg("lenLight = ");__printlnCfg(lenLight);
        EEPROM.write(EEPADDISSET, 0x55);
        EEPROM.write(EEPADDIFSETSE, 0x00);
        EEPROM.write(EEPADDIFSETAC, 0x00);
        EEPROM.write(EEPADDDEVICEID, dtaLight[1]);

    }
    
    else if((dtaLight[0] == 0x51) && (lenLight == 5))                // config device id & sensor id
    {
        EEPROM.write(EEPADDDEVICEID, dtaLight[1]);
        EEPROM.write(EEPADDISSET, 0x55);
        EEPROM.write(EEPADDIFSETSE, 0x55);
        EEPROM.write(EEPADDIFSETAC, 0x00);
        EEPROM.write(EEPADDSENSORID, dtaLight[3]);
        EEPROM.write(EEPADDFREQBROADCAST, dtaLight[4]);

    }

    else if((dtaLight[0] == 0x51) && (lenLight == 9))           // config device id & actuator
    {
        EEPROM.write(EEPADDISSET, 0x55);
        EEPROM.write(EEPADDDEVICEID, dtaLight[1]);
		EEPROM.write(EEPADDIFSETSE, 0x00);
		EEPROM.write(EEPADDIFSETAC, 0x55);
        EEPROM.write(EEPADDACTUATORID, dtaLight[3]);
        EEPROM.write(EEPADDACTCN, 1);
        EEPROM.write(EEPADDTCSTART+EEPOFFSETACLEN, 7);                  // ?
        EEPROM.write(EEPADDTCSTART+EEPOFFSETACDESTID, dtaLight[4]);            // ?
        EEPROM.write(EEPADDTCSTART+EEPOFFSETACACTIONTYPE, dtaLight[5]);
        EEPROM.write(EEPADDTCSTART+EEPOFFSETACOMTYPE, dtaLight[6]);
        EEPROM.write(EEPADDTCSTART+EEPOFFSETACDATALEN, 1);
        EEPROM.write(EEPADDTCSTART+EEPOFFSETACDATALONG, 2);
        EEPROM.write(EEPADDTCSTART+EEPOFFSETACDATA, dtaLight[7]);
        EEPROM.write(EEPADDTCSTART+EEPOFFSETACDATA+1, dtaLight[8]);


    }       
    else if((dtaLight[0] == 0x51) && (lenLight == 12))                   // config all
    {
        // device id
		EEPROM.write(EEPADDISSET, 0x55);
        EEPROM.write(EEPADDDEVICEID, dtaLight[1]);
		
        // sensor
		EEPROM.write(EEPADDIFSETSE, 0x55);
        EEPROM.write(EEPADDSENSORID, dtaLight[3]);
        EEPROM.write(EEPADDFREQBROADCAST, dtaLight[4]);
		
        // actuator
		EEPROM.write(EEPADDIFSETAC, 0x55);
        EEPROM.write(EEPADDACTUATORID, dtaLight[6]);
        EEPROM.write(EEPADDACTCN, 1);
        EEPROM.write(EEPADDTCSTART+EEPOFFSETACLEN, 7);                  // ?
        EEPROM.write(EEPADDTCSTART+EEPOFFSETACDESTID, dtaLight[7]);     // ?
        EEPROM.write(EEPADDTCSTART+EEPOFFSETACACTIONTYPE, dtaLight[8]);
        EEPROM.write(EEPADDTCSTART+EEPOFFSETACOMTYPE, dtaLight[9]);
        EEPROM.write(EEPADDTCSTART+EEPOFFSETACDATALEN, 1);
        EEPROM.write(EEPADDTCSTART+EEPOFFSETACDATALONG, 2);
        EEPROM.write(EEPADDTCSTART+EEPOFFSETACDATA, dtaLight[10]);
        EEPROM.write(EEPADDTCSTART+EEPOFFSETACDATA+1, dtaLight[11]);

    }
	else if(lenLight > 0)
	{
        BcnDrive.setLedShine(LEDCOLORGREEN, 50);
		return 0;
	}
    else                                                                // bad data
    {
        __printlnCfg("CONFIG: get config bad data!");
        for(int i = 0; i<lenLight; i++)
        {
            __printCfg(dtaLight[i]);
            __printCfg("\t");
        }
        __printlnCfg();
        BcnDrive.setLedShine(LEDCOLORGREEN, 50);
        return 0;
    }
    
    BcnDrive.setLedShine(LEDCOLORGREEN, 1000000);

    init();
    SENSOR.init(idSensor); 
    __printlnCfg("init Sensor over!");
    ACTUATOR.init(idActuator);
    __printlnCfg("init Actuator over!");
    BeaconApp.init();
    __printlnCfg("init Application over!");
    return 1;
}

BeaconConfig CONFIG;

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
