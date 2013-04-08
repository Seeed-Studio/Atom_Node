/*
  BeaconConfigDfs.h
  2013 Copyright (c) Seeed Technology Inc.  All right reserved.

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
#ifndef __BEACONCONFIGDFS_H__
#define __BEACONCONFIGDFS_H__

// control
#define __DebugConfig               0                                   // debug

#if __DebugConfig
#define __pirntCfg(x)               __print(x)
#define __printlnCfg(x)             __println(x)
#else
#define __printCfg(x)
#define __printlnCfg(x)
#endif

// number of trigger
#define NTR                         1                                   // number of actuator trigger

// EEPRMO ADDRESS
#define EEPADDISSET                 100                                 // if setted! if not ,zero, if setted, 1
#define EEPADDDEVICEID              101                                 // device id

// SENSOR ADDRESS
#define EEPADDIFSETSE               120                                 // if set sensor
#define EEPADDSENSORID              EEPADDIFSETSE+1                     // sensor id
#define EEPADDFREQBROADCAST         EEPADDSENSORID+1                    // frequence of broadcast the sensor value
#define EEPADDFREQBROADCASTUNIT     EEPADDFREQBROADCAST+2               // unit of frequence

// ACTUATOR ADDRESS
#define EEPADDIFSETAC               200                                 // if set actuator
#define EEPADDACTUATORID            EEPADDIFSETAC+1                     // actuator id
#define EEPADDACTCN                 EEPADDACTUATORID+1                  // how many trigger conditions

#define EEPADDTCSTART               EEPADDACTCN+1           

#define EEPOFFSETACLEN              0                                   // length of a Trigger conditions
#define EEPOFFSETACDESTID           EEPOFFSETACLEN+1                    // dest ID
#define EEPOFFSETACACTIONTYPE       EEPOFFSETACDESTID+1                 // action type
#define EEPOFFSETACOMTYPE           EEPOFFSETACACTIONTYPE+1             // compare type
#define EEPOFFSETACDATALEN          EEPOFFSETACOMTYPE+1                 // data len
#define EEPOFFSETACDATALONG         EEPOFFSETACDATALEN+1                // data long
#define EEPOFFSETACDATA             EEPOFFSETACDATALONG+1               // data

// compare type of actuator
#define COMPTYPEACGREAT             1                                   // greater than
#define COMPTYPEACLESS              2                                   // less than
#define COMPTYPEACEQUAL             3                                   // equal to
#define COMPTYPEACGREQUAL           4                                   // greater than or equal to
#define COMPTYPEACLEQUAL            5                                   // less than or equal to
#define COMPTYPEACRANGE             6                                   // bethin two day
#define COMPTYPEACNO                7                                   // no conditions trigger

// action type of actuator
#define ACTIONTYPEON                1                                   // ON
#define ACTIONTYPEOF                2                                   // OFF
#define ACTIONTYPEOUTPUT            3                                   // just output, such display a data!


#endif

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
