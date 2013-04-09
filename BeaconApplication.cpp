/*
  BeaconApplication.cpp
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
#include <Arduino.h>
#include <BeaconSensor.h>
#include <BeaconActuator.h>
#include <BeaconActuatorDfs.h>
#include <I2C_Uart.h>
#include <BeaconDrive.h>
#include <EEPROM.h>

#include "BeaconApplication.h"
#include "BeaconConfig.h"
#include "BeaconApplicationDfs.h"
#include "BeaconConfigDfs.h"

/*********************************************************************************************************
** Function name:           init
** Descriptions:            init
*********************************************************************************************************/
void BeaconApplication::init()
{
    carryDeviceId   = CONFIG.idDevice;
    workState       = WORKSTATEJOIN;
    workStateBuf    = WORKSTATEJOIN;
    workStateCnt    = 0;
    cntButtonOn     = 0;
    cntButton       = 0;
    flgGetSync      = 0;
    ledMode         = 1;
    cntButtonMainBuf    = 0;
    cntButtonMain   = 0;
    cntButtonHit    = 0;
    bdFreq          = CONFIG.freqSensor;
    BcnDrive.init();
}

/*********************************************************************************************************
** Function name:           appTimerIsr
** Descriptions:            appTimerIsr
*********************************************************************************************************/
void BeaconApplication::appTimerIsr()
{
    BcnDrive.ledIsr();
    cntButton++;
    workStateCnt++;
    cntButtonMain++;
}

/*********************************************************************************************************
** Function name:           sendDtaRfbee
** Descriptions:            send buf to rfbee
*********************************************************************************************************/
void BeaconApplication::sendDtaRfbee(unsigned char len, unsigned char *dta)
{
    SendByteToRfbee(FRAMESTART1);
    SendByteToRfbee(FRAMESTART2);
    for(unsigned int i = 0; i<len; i++)
    {
        SendByteToRfbee(dta[i]);
    }
    SendByteToRfbee(FRAMEEND1);
    SendByteToRfbee(FRAMEEND2);

}

/*********************************************************************************************************
** Function name:           sensorBroadCast
** Descriptions:            BroadCast sensor value now
*********************************************************************************************************/
void BeaconApplication::sensorBroadCast()
{
    if(CONFIG.ifSetSensor != 0x55)
    {
        return ;
    }
    
    if(ledMode)
    {
        BcnDrive.setLedShine(LEDCOLORGREEN, 5);
    }
    unsigned char dtaSe[10];
    SENSOR.getSensor(dtaSe);

    dtaSendRf[0] = CONFIG.idDevice;
    dtaSendRf[1] = CONFIG.idSensor;
    dtaSendRf[2] = 0;
    dtaSendRf[3] = FRAMETYPEBC;
    dtaSendRf[4] = dtaSe[0];

    for(int i = 0; i<dtaSe[0]; i++)
    {
        dtaSendRf[i+5] = dtaSe[i+1];
    }
    dtaSendRf[5+dtaSe[0]] = 0;

    sendDtaRfbee(6+dtaSe[0], dtaSendRf);

    for(int i = 5+dtaSe[0]+4; i>=2; i--)
    {
        dtaSendRf[i] = dtaSendRf[i-2];
    }
    dtaSendRf[0] = FRAMESTART1;
    dtaSendRf[1] = FRAMESTART2;
    if(isTrigger(dtaSendRf))                        // trigger device itself
    {
        Trigger(dtaSendRf);
    }
}

/*********************************************************************************************************
** Function name:           isTrigger
** Descriptions:            if trigger
*********************************************************************************************************/
bool BeaconApplication::isTrigger(unsigned char *dta)
{
    __printlnApp("GET IN ISTRIGGER!");

    if(CONFIG.ifSetActuator != 0x55)
    {
        __printlnApp("ACTUATOR NO CONFIG!");
        return 0;
    }
    // it is a broadcast frame!
    if(dta[FRAMEBITDESTID] == 0 && dta[FRAMEBITFRAME] == FRAMETYPEBC)
    {
        for(int i = 0; i<CONFIG.nTC; i++)
        {
            if(CONFIG.TC[i][1] == dta[FRAMEBITSRCID])
            {
                __printApp("GET TRIGGER!!\t");
                __printlnApp(i+1);
                tcNum = i+1;
                return tcNum;
            }
            else
            {
                __printlnApp("SRCID ERR!!\t");
                __printApp("tc[i][1] = ");
                __printlnApp(CONFIG.TC[i][1]);
                __printApp("dta[FRA..] = ");
                __printlnApp(dta[FRAMEBITSRCID]); //CONFIG.idDevice
                __printApp("CONFIG.idDevice = ");
                __printlnApp(CONFIG.idDevice);
            }
        }
    }
    else
    {
        __printlnApp("NO TRIGGER!");
    }
    return 0;
}

/*********************************************************************************************************
** Function name:           Trigger
** Descriptions:            do something
*********************************************************************************************************/
void BeaconApplication::Trigger(unsigned char *dta)
{
    //if(ledMode)
    //{
        BcnDrive.setLedShine(LEDCOLORGREEN, 10);
    //}
    
    unsigned char nTmp[3];
    /*
     *      IO Actuator
     */

    __printAppS("trigger");
    if(CONFIG.idActuator <= 200)        // IO
    {
        TriggerAnalog(dta);
    }
    else
    {
        switch(CONFIG.idActuator)
        {
            case ACTUATOROLED12864:

            nTmp[0] = dta[FRAMEBITDATALEN];
            for(int i = 0; i<dta[FRAMEBITDATALEN]; i++)
            {
                nTmp[i+1] = dta[FRAMEBITDATA+i];
            }
            ACTUATOR.driveActuator(nTmp);
            break;

            default:
            ;
        }
    }
}

/*********************************************************************************************************
** Function name:           TriggerAnalog
** Descriptions:            trigger when analog
*********************************************************************************************************/
void BeaconApplication::TriggerAnalog(unsigned char *dta)
{

    unsigned int cmpDtaSensor   = 0;
    unsigned int cmpDtaSet      = 0;
    unsigned char dtaAc[2];

    unsigned int cmpLarge      = 0;             // lower limit
    unsigned int cmpSmall      = 0;             // higer limit

    __printAppS("\r\ndta[LEN] = ");
    __printlnAppS(dta[FRAMEBITDATALEN]);

    if(dta[FRAMEBITDATALEN] == 1)
    {
        cmpDtaSensor = dta[EEPOFFSETACDATA];
        for(int i = 0; i<2; i++)
        {
            cmpDtaSet       = cmpDtaSet<<8;
            cmpDtaSet      += CONFIG.TC[tcNum-1][EEPOFFSETACDATA+i];
            __printlnAppS("\r\ncmpDtaSet = :");
            __printlnAppS(cmpDtaSet);
        }
    }
    else
    for(int i = 0; i<dta[FRAMEBITDATALEN]; i++)
    {
        cmpDtaSensor    = cmpDtaSensor<<8;
        cmpDtaSet       = cmpDtaSet<<8;
        cmpDtaSet      += CONFIG.TC[tcNum-1][EEPOFFSETACDATA+i];
        __printlnAppS("\r\ncmpDtaSet = :");
        __printlnAppS(cmpDtaSet);
        cmpDtaSensor   += dta[FRAMEBITDATA+i];
    }

    /*
     *      to give value to cmpLarge and cmpSmall
     */

    switch(CONFIG.TC[tcNum-1][EEPOFFSETACOMTYPE])
    {
        case COMPTYPEACGREAT:
        cmpLarge = 1023;
        cmpSmall = cmpDtaSet;

        break;

        case COMPTYPEACLESS:
        cmpLarge = cmpDtaSet;
        cmpSmall = 0;

        break;

        default:
        ;
    }

    __printlnAppS("TRIGGER:");
    __printAppS("cmpLarge = ");
    __printlnAppS(cmpLarge);
    __printAppS("cmpSmall = ");
    __printlnAppS(cmpSmall);

    unsigned char dtaCmp = (cmpDtaSensor >= cmpSmall && cmpDtaSensor <= cmpLarge);

    dtaAc[0] = 1;
    dtaAc[1] = (CONFIG.TC[tcNum-1][EEPOFFSETACACTIONTYPE] == ACTIONTYPEON) ? dtaCmp : 1-dtaCmp;
    ACTUATOR.driveActuator(dtaAc);
}

/*********************************************************************************************************
** Function name:           getBatLev
** Descriptions:            get battery level
*********************************************************************************************************/
unsigned char BeaconApplication::getBatLev()
{
    unsigned int uiA2 = analogRead(A2);

    if(uiA2 > BATTERY80VAL)
    {
        digitalWrite(PINBATLOW, LOW);
        return BATTERY80;
    }
    else if(uiA2 > BATTERY60VAL)
    {
        digitalWrite(PINBATLOW, LOW);
        return BATTERY60;
    }
    else if(uiA2 > BATTERY40VAL)
    {
        digitalWrite(PINBATLOW, LOW);
        return BATTERY40;
    }
    else if(uiA2 > BATTERY20VAL)
    {
        digitalWrite(PINBATLOW, LOW);
        return BATTERY20;
    }
    else
    {
        digitalWrite(PINBATLOW, HIGH);
        return BATTERYLOW;
    }

    return 0;
}

/*********************************************************************************************************
** Function name:           sendJoin
** Descriptions:            sendJoin
*********************************************************************************************************/
void BeaconApplication::sendJoin()
{
    dtaSendRf[0] = CONFIG.idDevice;
    dtaSendRf[1] = CONFIG.idSensor;
    dtaSendRf[2] = 0;
    dtaSendRf[3] = 4;
    dtaSendRf[4] = 1;
    dtaSendRf[5] = bdFreq;
    dtaSendRf[6] = 0;
    sendDtaRfbee(7, dtaSendRf);
}

/*********************************************************************************************************
** Function name:           sendSync
** Descriptions:            tell other devices to sync
*********************************************************************************************************/
void BeaconApplication::sendSync()
{
    dtaSendRf[0] = CONFIG.idDevice;
    dtaSendRf[1] = CONFIG.idSensor;
    dtaSendRf[2] = 0;
    dtaSendRf[3] = 5;
    dtaSendRf[4] = 0;
    dtaSendRf[5] = 0;
    sendDtaRfbee(6, dtaSendRf);
}

/*********************************************************************************************************
** Function name:           sendRfSleep
** Descriptions:            tell rfbee to sleep
*********************************************************************************************************/
void BeaconApplication::sendRfSleep()
{
    dtaSendRf[0] = 0x55;
    dtaSendRf[1] = 0x55;
    dtaSendRf[2] = 0x55;
    sendDtaRfbee(3, dtaSendRf);
    delay(10);
}

/*********************************************************************************************************
** Function name:           buttonManage
** Descriptions:            button
*********************************************************************************************************/
void BeaconApplication::buttonManage()
{
    unsigned char completeHit = 0;
    
    if(cntButton > 10)
    {
        cntButton = 0;
        if(!digitalRead(PINSYSBUTT))                    // button on
        {
            cntButtonOn++;
            cntButtonOff = 0;
        }
        else
        {
            cntButtonOff++;
        }
    }

    if(cntButtonOn > TURNOFFDELAYTIME-5)                // turn off
    {
        __printlnAppS("TURN OFF");
        __printAppS("cntButtonOn = ");
        __printlnAppS(cntButtonOn);

        cntButtonOn = 0;
        BcnDrive.beepOn();
        delay(200);
        BcnDrive.beepOff();

        BcnDrive.sysPowerOff();
        delay(3000);
        BcnDrive.sysPowerOn();
    }
    
    if(cntButtonOff > 3 && cntButtonOn > 1)
    {
        cntButtonHit++;
        cntButtonMainBuf = cntButtonMain;
        cntButtonOn      = 0;
        cntButtonOff     = 0;
        cntButton        = 0;
    }
    
    if(cntButtonHit)
    {
        if((cntButtonMain - cntButtonMainBuf) > 500)
        {
            completeHit = 1;
        }
    }
    
    if(completeHit)
    {
        completeHit = 0;
        if(cntButtonHit == 1)           // config
        {
            if(workState == WORKSTATECFG)
            {
                BcnDrive.setLedShine(LEDCOLORGREEN, 5);
                stateChange(workStateBuf);
            }
            else if(workStateBuf == WORKSTATECARRY || workStateBuf == WORKSTATENARMAL || workStateBuf == WORKSTATEJOIN)
            {
                BcnDrive.setLedShine(LEDCOLORRED, 1000000);
                stateChange(WORKSTATECFG);
            }
        }
        else if(cntButtonHit == 2)      // set led mode
        {
            ledMode = 1-ledMode;
            if(bdFreq != BDF100MS)
            {
                stateChange(workStateBuf);
            }
        }
        else if(cntButtonHit == 4)      // CLEAR ALL THE EEPROM DATA AND TURN OFF
        {
            for(int i = 0; i<512; i++)
            {
                EEPROM.write(i, 0);
                BcnDrive.setLedShine(LEDCOLORRED, 1);
                delay(3);
            }
            BcnDrive.beepOn();
            delay(200);
            BcnDrive.beepOff();
            BcnDrive.sysPowerOff();
        }
        cntButtonOn     = 0;
        cntButtonOff    = 0;
        cntButton       = 0;
        cntButtonHit    = 0;
    }
}

/*********************************************************************************************************
** Function name:           carryState
** Descriptions:            carryState
*********************************************************************************************************/
void BeaconApplication::carryState()
{
    if(carryDeviceId != CONFIG.idDevice)return ;

    if(bdFreq == BDF1S)                             // this device is the min id
    {
        if(workStateCnt % 1000 == 10)               // send sync
        {
            workStateCnt++;
            sendSync();
        }
        else if(workStateCnt % 1000 == 50)          // broadcast sensor value
        {
            workStateCnt++;
            sensorBroadCast();                      // broadcast
        }
		
        else if(workStateCnt % 1000 == 100)         // begin to sleep
        {
            sendRfSleep();                          // tell rfbee to sleep 900ms
            for(int i = 0; i<9; i++)
            {
                BcnDrive.pwrDown(105);              // sleep 100 ms
                BcnDrive.wakeUp();
                if(!digitalRead(PINSYSBUTT))        // button ?
                {
                    workStateBuf = WORKSTATECARRY;
                    delay(10);
                    stateChange(WORKSTATEBUTTON);
                    BcnDrive.rLedCnt = BcnDrive.rLedCnt>100 ? BcnDrive.rLedCnt-100 : 0;
                    BcnDrive.gLedCnt = BcnDrive.gLedCnt>100 ? BcnDrive.gLedCnt-100 : 0;
                    break;
                }
                BcnDrive.rLedCnt = BcnDrive.rLedCnt>100 ? BcnDrive.rLedCnt-100 : 0;
                BcnDrive.gLedCnt = BcnDrive.gLedCnt>100 ? BcnDrive.gLedCnt-100 : 0;
            }
            workStateCnt  = 0;
        }
    }

    else if(bdFreq == BDF100MS)
    {
        if(workStateCnt % 100 == 10)                // send sync
        {
            workStateCnt++;
            sendSync();
        }
        else if(workStateCnt % 100 == 50)
        {
            workStateCnt++;
            sensorBroadCast();                      // broadcast
        }
        else if(workStateCnt % 100 >= 99)
        {
            workStateCnt = 0;
        }
    }
}

/*********************************************************************************************************
** Function name:           supportState
** Descriptions:            supportState
*********************************************************************************************************/
void BeaconApplication::supportState()
{
    if(!flgGetSync)return;
    
    if(bdFreq == BDF1S)
    {
        if(workStateCnt % 1000 == 50)               // broadcast sensor value
        {
            workStateCnt++;
            sensorBroadCast();                      // broadcast
        }
        else if(workStateCnt % 1000 == 90)          // begin to sleep
        {
            flgGetSync = 0;
#if __SleepMode
            sendRfSleep();                          // tell rfbee to sleep 900ms
#endif
            for(int i = 0; i<9; i++)
            {
#if __SleepMode
                BcnDrive.pwrDown(100);              // sleep 100 ms
                BcnDrive.wakeUp();
#else
                delay(100);
#endif
                if(!digitalRead(PINSYSBUTT))        // button ?
                {
                    workStateBuf = WORKSTATENARMAL;
                    delay(10);
                    stateChange(WORKSTATEBUTTON);
                    BcnDrive.rLedCnt = BcnDrive.rLedCnt>100 ? BcnDrive.rLedCnt-100 : 0;
                    BcnDrive.gLedCnt = BcnDrive.gLedCnt>100 ? BcnDrive.gLedCnt-100 : 0;
                    break;
                }
                BcnDrive.rLedCnt = BcnDrive.rLedCnt>100 ? BcnDrive.rLedCnt-100 : 0;
                BcnDrive.gLedCnt = BcnDrive.gLedCnt>100 ? BcnDrive.gLedCnt-100 : 0;
            }
            workStateCnt  = 0;
        }
    }
    else if(bdFreq == BDF100MS)
    {
        if(workStateCnt % 100 == 50)               // broadcast sensor value
        {
            workStateCnt++;
            sensorBroadCast();                      // broadcast
        }
        else if(workStateCnt % 100 >= 95)
        {
            workStateCnt = 0;
            flgGetSync   = 0;
        }
    }
}
/*********************************************************************************************************
** Function name:           workStateMachine
** Descriptions:            workStateMachine
*********************************************************************************************************/
void BeaconApplication::workStateMachine()
{

    if(CONFIG.ifSetDevice != 0x55 && workState != WORKSTATECFG)
    {
        return ;
    }

    switch(workState)
    {
        /***********************************************************************************************
        ******************************************* WORKSTATEJOIN **************************************
        ***********************************************************************************************/
        case WORKSTATEJOIN:

        if(CONFIG.ifSetDevice != 0x55)                      // device not config
        {
            return ;
        }

        if(workStateCnt < 1500)                             // send join for 2s
        {
            if(workStateCnt % 60 == 0)                      // send join per 60ms
            {
                workStateCnt++;
                sendJoin();
                BcnDrive.setLedShine(LEDCOLORRED, 5);
            }
        }
        else
        {
            workStateBuf = WORKSTATEJOIN;
            stateChange(WORKSTATECARRY);
        }

        break;

        /***********************************************************************************************
        ******************************************* WORKSTATECARRY *************************************
        ***********************************************************************************************/
        case WORKSTATECARRY:
        carryState();
        break;

        /***********************************************************************************************
        ******************************************* WORKSTATENARMAL ************************************
        ***********************************************************************************************/
        case WORKSTATENARMAL:
        supportState();
        break;

        /***********************************************************************************************
        ********************************************WORKSTATECFG****************************************
        ***********************************************************************************************/
        case WORKSTATECFG:

        CONFIG.lightConfig();
        
        break;

        default:
        ;
    }
}

/*********************************************************************************************************
** Function name:           stateChange
** Descriptions:            goto state
*********************************************************************************************************/
void BeaconApplication::stateChange(unsigned char state)
{
    workState = state;
}

BeaconApplication BeaconApp;
/*********************************************************************************************************
  END FILE
*********************************************************************************************************/