/*
  BeaconDemoCodeV0.2.ino
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
#include <EEPROM.h>
#include <BeaconSensor.h>
#include <BeaconActuator.h>
#include <BeaconDrive.h>
#include <TimerOne.h>
#include <LightCom.h>
#include <Wire.h>
#include <Sleep_n0m1.h>
#include <SeeedOLED.h>

#include "BeaconApplication.h"
#include "BeaconConfig.h"
#include "BeaconApplicationDfs.h"

#define __DebugIno      0

#if __DebugIno
#define __printIno(x)   __print(x)
#define __printlnIno(x) __println(x)
#else
#define __printIno(x)
#define __printlnIno(x)
#endif

unsigned char __GdtaUart[50];                   // uart data buff
unsigned char __GdtaUartLen        = 0;         // uart data len
unsigned char __GstringComplete    = 0;         // if get data

/*********************************************************************************************************
** Function name:           timer1ISR
** Descriptions:            timer interrupt service 
*********************************************************************************************************/
void timer1ISR()
{
    BeaconApp.appTimerIsr();                    // application isr
    if(BeaconApp.workState == WORKSTATECFG)
    {
        LightCom1.TIMEISR();                    // light com isr
    }
}

void just_for_fun(unsigned char *dta)
{
    for(int i = 0; i<5; i++)
    {
        if(dta[i+2] != 0x55)
        {
            return ;
        }
    }
    BcnDrive.beepOn();
    
    while(1);               // get 10 continus 0x55, down!!
}

/*********************************************************************************************************
** Function name:           checkGoodDta
** Descriptions:            if uart get good data
*********************************************************************************************************/
unsigned char checkGoodDta(unsigned char *dta)
{
    unsigned char ioffset = 0;
    for(ioffset = 0; ioffset < __GdtaUartLen; ioffset++)
    {
        if(dta[ioffset] == FRAMESTART1)
        {
            break;
        }
    }

    if(ioffset > 0)
    {
        for(int i = 0; i<7; i++)
        {
            dta[i] = dta[i+ioffset];
        }
    }

    if((__GdtaUartLen - ioffset)<6 \
        || !(   dta[0] == FRAMESTART1 \
             && dta[1] == FRAMESTART2 \
             && dta[8+dta[6]] == FRAMEEND1 \
             && dta[9+dta[6]] == FRAMEEND2 ))
    {
        __GdtaUartLen     = 0;
        __GstringComplete = 0;
        return 0;
    }

    return 1;
}

/*********************************************************************************************************
** Function name:           rfDtaProc
** Descriptions:            get data from rfBee and process
*********************************************************************************************************/
void rfDtaProc()
{
    if(__GstringComplete == 1 && checkGoodDta(__GdtaUart))                      // if serial get data
    {
        if(__GdtaUart[FRAMEBITFRAME] == 4)                                      // other device join
        {
            BeaconApp.carryDeviceId  = __GdtaUart[FRAMEBITSRCID];
            BeaconApp.workStateCnt   = 0;
            BeaconApp.workStateBuf   = BeaconApp.workState;
            BeaconApp.stateChange(WORKSTATENARMAL);
            BeaconApp.bdFreq         = __GdtaUart[FRAMEBITDATA];                // change freq
            EEPROM.write(EEPADDFREQBROADCAST, BeaconApp.bdFreq);                // write to eeprom
        }
        else if(__GdtaUart[FRAMEBITFRAME] == 5 && BeaconApp.workState == WORKSTATENARMAL)   // sync
        {
            if(BeaconApp.flgGetSync == 0)
            {
                BeaconApp.workStateCnt = 0;
                BeaconApp.flgGetSync   = 1;
                
            }
        }
        else if((BeaconApp.workState == WORKSTATECARRY || BeaconApp.workState == WORKSTATENARMAL) && BeaconApp.isTrigger(__GdtaUart))     // if the data trigger
        {
            BeaconApp.Trigger(__GdtaUart);
        }
        __GdtaUartLen      = 0;
        __GstringComplete  = 0;
    }
}

/*********************************************************************************************************
** Function name:           setup
** Descriptions:            setup
*********************************************************************************************************/
void setup()
{
    
    BcnDrive.init();
    BcnDrive.sysPowerOn();                      // power on
    delay(500);
    
    Serial1.begin(57600);                       // Serial1, to send/rev data from RFBee
    while(!Serial1);                            // Unitll Serial1 init over
    
    LightCom1.init(12);                         // light com  D12: input
    CONFIG.init();                              // init config
    SENSOR.init(CONFIG.idSensor);               // init sensor
    ACTUATOR.init(CONFIG.idActuator);           // init actuator
    BeaconApp.init();                           // init application
    
    while(1)                                    // while button release
    {
        if(digitalRead(PINSYSBUTT))
        {
            break;
        }
        delay(10);
    }
    Timer1.initialize(1000);                    // set a timer of length 1ms
    Timer1.attachInterrupt( timer1ISR );        // attach the service routine here
}

/*********************************************************************************************************
** Function name:           loop
** Descriptions:            loop
*********************************************************************************************************/
void loop()
{
    rfDtaProc();                                // data process
    BeaconApp.buttonManage();                   // button manage
    BeaconApp.workStateMachine();               // state machine
    BeaconApp.getBatLev();                      // get batary level
    serialEvent1();                             // check serial data
}

/*********************************************************************************************************
** Function name:           serialEvent1
** Descriptions:            Serial1 event
*********************************************************************************************************/
void serialEvent1() 
{
    while (Serial1.available())
    {
        __GdtaUart[__GdtaUartLen++] = (unsigned char)Serial1.read();
        __GstringComplete =   (__GdtaUart[__GdtaUartLen-1] == FRAMEEND2) ? 1 : __GstringComplete;
        __GdtaUartLen = (__GdtaUartLen > 45) ? 0 :  __GdtaUartLen;
    }
}

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
