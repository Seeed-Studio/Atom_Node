/*
  NodeGlobal.h
  2012 Copyright (c) Seeed Technology Inc.  All right reserved.

  Author:Loovee
  2012-12-3

  https://github.com/reeedstudio/Atom_Node

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
#ifndef __NODEGLOBAL_H__
#define __NODEGLOBAL_H__

#define __SleepMode                 0                           // sleep mode

#define _XBEE                       0                           // use XBEE
#define _RFBEE                      1                           // use RFBEE

#define BAUDRATE                    38400U                      // Serial1 baudrate
#define SendByteToRfbee             Serial1.write               // send a byte to Serial1
#define TURNOFFDELAYTIME            80                          // when press the button for xx*10 ms, turn off the system

// broadcast freq
#define BDF100MS                    0x80                        // 100ms
#define BDF1S                       0x81                        // 1s
#define BDF1MIN                     0x82                        // 1min
#define BDF1HOUR                    0x83                        // 1hour

// data frame
#define FRAMESTART1                 0x53                        // data frame start1
#define FRAMESTART2                 0x01                        // data frame start2
#define FRAMEEND1                   0x2f                        // data frame end1
#define FRAMEEND2                   0x45                        // data frame end2

//bit of frame
#define FRAMEBITSTART1              0                           // frame data bit0
#define FRAMEBITSTART2              1
#define FRAMEBITSRCID               2
#define FRAMEBITSENSORID            3
#define FRAMEBITDESTID              4
#define FRAMEBITFRAME               5
#define FRAMEBITDATALEN             6
#define FRAMEBITDATA                7

// frame type
#define FRAMETYPEBC                 1                           // broadcast
#define FRAMETYPECTRL               2                           // control
#define FRAMETYPESET                3                           // setting

// led Mode
#define LEDEANBLE                   1                           // led enable
#define LEDDISABLE                  0                           // turn off user led

// level of battery
#define BATTERYLOW                  1                           // less that 20%
#define BATTERY20                   2                           // 20% - 40%
#define BATTERY40                   3                           // 40% - 60%
#define BATTERY60                   4                           // 60% - 80%
#define BATTERY80                   5                           // 80% - 100%

// power level of battery
#define BATTERY20VAL                575                         // adc val of less than 20%
#define BATTERY40VAL                583                         // adc val of less than 40%
#define BATTERY60VAL                595                         // adc val of less than 60%    
#define BATTERY80VAL                615                         // adc val of less than 80%

// work state
#define WORKSTATEJOIN               1                           // state join
#define WORKSTATECARRY              2                           // state carry
#define WORKSTATENARMAL             3                           // state normal-not carry
#define WORKSTATEBUTTON             4                           // when button on
#define WORKSTATECFG                5                           // when config

// EEPRMO ADDRESS
#define EEPADDISSET                 100                         // if setted! if not ,zero, if setted, 1
#define EEPADDDEVICEID              101                         // device id

// SENSOR ADDRESS
#define EEPADDIFSETSE               120                         // if set sensor
#define EEPADDSENSORID              EEPADDIFSETSE+1             // sensor id
#define EEPADDFREQBROADCAST         EEPADDSENSORID+1            // frequence of broadcast the sensor value
#define EEPADDFREQBROADCASTUNIT     EEPADDFREQBROADCAST+2       // unit of frequence

// ACTUATOR ADDRESS
#define EEPADDIFSETAC               200                         // if set actuator
#define EEPADDACTUATORID            EEPADDIFSETAC+1             // actuator id
#define EEPADDACTCN                 EEPADDACTUATORID+1          // how many trigger conditions
#define EEPADDTCSTART               EEPADDACTCN+1

#define EEPOFFSETACLEN              0                           // length of a Trigger conditions
#define EEPOFFSETACDESTID           EEPOFFSETACLEN+1            // dest ID
#define EEPOFFSETACACTIONTYPE       EEPOFFSETACDESTID+1         // action type
#define EEPOFFSETACOMTYPE           EEPOFFSETACACTIONTYPE+1     // compare type
#define EEPOFFSETACDATALEN          EEPOFFSETACOMTYPE+1         // data len
#define EEPOFFSETACDATALONG         EEPOFFSETACDATALEN+1        // data long
#define EEPOFFSETACDATA             EEPOFFSETACDATALONG+1       // data

// compare type of actuator
#define COMPTYPEACGREAT             1                           // greater than
#define COMPTYPEACLESS              2                           // less than
#define COMPTYPEACEQUAL             3                           // equal to
#define COMPTYPEACGREQUAL           4                           // greater than or equal to
#define COMPTYPEACLEQUAL            5                           // less than or equal to
#define COMPTYPEACRANGE             6                           // bethin two day
#define COMPTYPEACNO                7                           // no conditions trigger

// action type of actuator
#define ACTIONTYPEON                1                           // ON
#define ACTIONTYPEOF                2                           // OFF
#define ACTIONTYPEOUTPUT            3                           // just output, such display a data!

#define RELEASSBUTTON()                 while(1)\
                                        {\
                                            if(digitalRead(PINSYSBUTT))\
                                                break;\
                                            delay(10);\
                                        }\

#endif

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/