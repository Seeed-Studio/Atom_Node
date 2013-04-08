/*
  BeaconApplication.h
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
#ifndef __BEACONAPPLICATION_H__
#define __BEACONAPPLICATION_H__

#include "BeaconApplicationDfs.h"

class BeaconApplication{

private:

    unsigned char tcNum;                    // tc num
    unsigned int  cntButtonOn;              // button on counter
    unsigned int  cntButtonOff;             // button off counter
    unsigned int  cntButton;                // count button
    unsigned long cntButtonMain;            // count button
    unsigned long cntButtonMainBuf;         // count button buf
    unsigned char cntButtonHit;             // count of hit
    unsigned char dtaSendRf[20];            // data send to rf
    
    unsigned char ledMode;                  // led Mode

public:

    unsigned char carryDeviceId;            // min device id   
    unsigned char workState;                // work state
    unsigned char workStateBuf;             // work state buf
    unsigned long workStateCnt;             // count of work state
    unsigned char flgGetSync;               // get sync
    unsigned char bdFreq;                   // broadcart freq...

private:
    void sendJoin();                        // tell other devices a minId join
    void sendSync();                        // tell other devices to sync
    void sendRfSleep();                     // tell rfbee to sleep

    void TriggerAnalog(unsigned char *dta); // trigger when analog
    void sensorBroadCast();                 // sensor broadcast 
    
    void carryState();                      // carry state
    void supportState();                    // supportState
    

public:
    
    void init();                            // init
	void sendDtaRfbee(unsigned char len, 
                      unsigned char *dta);  // send buf to rfbee
    void appTimerIsr();                     // put in a tiemr isr, 1ms
    bool isTrigger(unsigned char *dta);     // if get trigger
    void Trigger(unsigned char *dta);       // trigger
    unsigned char getBatLev();              // get battery level
    void buttonManage();                    // button
    void workStateMachine();                // working state machine
    void stateChange(unsigned char state);  // change work state
    
};

extern BeaconApplication BeaconApp;
#endif

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/