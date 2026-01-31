/*
  RCSwitchB - Arduino libary for remote control outlet switches
  Copyright (c) 2011 Suat Özgür.  All right reserved.
  
  Contributors:
  - Andre Koehler / info(at)tomate-online(dot)de
  - Gordeev Andrey Vladimirovich / gordeev(at)openpyro(dot)com
  - Skineffect / http://forum.ardumote.com/viewtopic.php?f=2&t=46
  - Dominik Fischer / dom_fischer(at)web(dot)de
  - Frank Oltmanns / <first name>.<last name>(at)gmail(dot)com
  - Andreas Steinel / A.<lastname>(at)gmail(dot)com
  - Max Horn / max(at)quendi(dot)de
  - Robert ter Vehn / <first name>.<last name>(at)gmail(dot)com
  - Johann Richard / <first name>.<last name>(at)gmail(dot)com
  - Vlad Gheorghe / <first name>.<last name>(at)gmail(dot)com https://github.com/vgheo
  - Matias Cuenca-Acuna 
  
  Project home: https://github.com/sui77/rc-switch/

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

#include "RCSwitchB.h"

#ifdef RaspberryPi
    // PROGMEM and _P functions are for AVR based microprocessors,
    // so we must normalize these for the ARM processor:
    #define PROGMEM
    #define memcpy_P(dest, src, num) memcpy((dest), (src), (num))
#endif

#if defined(ESP8266)
    // interrupt handler and related code must be in RAM on ESP8266,
    // according to issue #46.
    #define RECEIVE_ATTR ICACHE_RAM_ATTR
    #define VAR_ISR_ATTR
#elif defined(ESP32)
    #define RECEIVE_ATTR IRAM_ATTR
    #define VAR_ISR_ATTR DRAM_ATTR
#else
    #define RECEIVE_ATTR
    #define VAR_ISR_ATTR
#endif


/* Format for protocol definitions:
 * {pulselength, Sync bit, "0" bit, "1" bit, invertedSignal}
 * 
 * pulselength: pulse length in microseconds, e.g. 350
 * Sync bit: {1, 31} means 1 high pulse and 31 low pulses
 *     (perceived as a 31*pulselength long pulse, total length of sync bit is
 *     32*pulselength microseconds), i.e:
 *      _
 *     | |_______________________________ (don't count the vertical bars)
 * "0" bit: waveform for a data bit of value "0", {1, 3} means 1 high pulse
 *     and 3 low pulses, total length (1+3)*pulselength, i.e:
 *      _
 *     | |___
 * "1" bit: waveform for a data bit of value "1", e.g. {3,1}:
 *      ___
 *     |   |_
 *
 * These are combined to form Tri-State bits when sending or receiving codes.
 */
// 注意：不能使用 const，否则会放在 Flash 中，ISR 无法访问
static VAR_ISR_ATTR RCSwitchB::Protocol proto[] = {
  // { 350, {  1, 31 }, {  1,  3 }, {  3,  1 }, false },    // protocol 1
  // { 650, {  1, 10 }, {  1,  2 }, {  2,  1 }, false },    // protocol 2
  // { 100, { 30, 71 }, {  4, 11 }, {  9,  6 }, false },    // protocol 3
  // { 380, {  1,  6 }, {  1,  3 }, {  3,  1 }, false },    // protocol 4
  // { 500, {  6, 14 }, {  1,  2 }, {  2,  1 }, false },    // protocol 5
  // { 450, { 23,  1 }, {  1,  2 }, {  2,  1 }, true },     // protocol 6 (HT6P20B)
  // { 150, {  2, 62 }, {  1,  6 }, {  6,  1 }, false },    // protocol 7 (HS2303-PT, i. e. used in AUKEY Remote)
  // { 200, {  3, 130}, {  7, 16 }, {  3,  16}, false},     // protocol 8 Conrad RS-200 RX
  // { 200, { 130, 7 }, {  16, 7 }, { 16,  3 }, true},      // protocol 9 Conrad RS-200 TX
  // { 365, { 18,  1 }, {  3,  1 }, {  1,  3 }, true },     // protocol 10 (1ByOne Doorbell)
  // { 270, { 36,  1 }, {  1,  2 }, {  2,  1 }, true },     // protocol 11 (HT12E)
  // { 320, { 36,  1 }, {  1,  2 }, {  2,  1 }, true }      // protocol 12 (SM5212)
  // --- 1..12 (原始常见协议，保留/扩展) ---
  { 350, {  1, 31 }, {  1,  3 }, {  3,  1 }, false }, // 1 PT2262 标准 (433/315)
  { 650, {  1, 10 }, {  1,  2 }, {  2,  1 }, false }, // 2 PT2262 长同步 (433/315)
  { 100, { 30, 71 }, {  4, 11 }, {  9,  6 }, false }, // 3 Intertechno Classic (欧)
  { 380, {  1,  6 }, {  1,  3 }, {  3,  1 }, false }, // 4 SC2262 短脉宽 (433/315)
  { 500, {  6, 14 }, {  1,  2 }, {  2,  1 }, false }, // 5 HX2262 扩展
  { 450, { 23,  1 }, {  1,  2 }, {  2,  1 }, true  }, // 6 HT6P20B (门铃/门禁)
  { 150, {  2, 62 }, {  1,  6 }, {  6,  1 }, false }, // 7 HS2303-PT (AUKEY)
  { 200, {  3,130 }, {  7,16 }, {  3,16 }, false }, // 8 Conrad RS-200 RX
  { 200, {130,  7 }, { 16, 7 }, { 16, 3 }, true  }, // 9 Conrad RS-200 TX
  { 365, { 18,  1 }, {  3,  1 }, {  1,  3 }, true  }, //10 1ByOne 门铃
  { 270, { 36,  1 }, {  1,  2 }, {  2,  1 }, true  }, //11 HT12E
  { 320, { 36,  1 }, {  1,  2 }, {  2,  1 }, true  }, //12 SM5212

  // --- 13..24 More PT/SC/EV variants and 315-specific variants ---
  { 300, {  1, 23 }, {  1,  2 }, {  2,  1 }, false }, //13 PT2262 variant A (433)
  { 320, {  1, 24 }, {  1,  2 }, {  2,  1 }, false }, //14 PT2262 variant B (433)
  { 290, {  1, 22 }, {  1,  3 }, {  3,  1 }, false }, //15 SC2262 variant A (433)
  { 360, {  1, 30 }, {  1,  4 }, {  4,  1 }, false }, //16 HX2262 long pulse (433)
  { 340, {  1, 28 }, {  1,  3 }, {  3,  1 }, false }, //17 EV1527 typical (433)
  { 310, {  1, 26 }, {  1,  2 }, {  2,  1 }, false }, //18 HS1527 (433)
  { 280, {  2, 30 }, {  1,  3 }, {  3,  1 }, false }, //19 RT1527 variant (433)
  { 450, {  3, 30 }, {  3,  1 }, {  1,  3 }, false }, //20 PT2272 / garage remote style (433)
  { 375, {  1, 15 }, {  1,  3 }, {  3,  1 }, false }, //21 315 PT-like variant (315) *
  { 540, {  1, 12 }, {  1,  2 }, {  2,  1 }, false }, //22 315 long-base variant (315) *
  { 260, {  1, 20 }, {  1,  3 }, {  3,  1 }, false }, //23 EV1527 tight timing (433/315)
  { 240, {  1, 18 }, {  1,  2 }, {  2,  1 }, false }, //24 315 SC variant (315) *

  // --- 25..36 门铃 / 门禁 / 老式芯片多变体 ---
  { 400, { 20,  1 }, {  1,  2 }, {  2,  1 }, true  }, //25 HT6P12 (门铃常用)
  { 420, { 18,  1 }, {  1,  2 }, {  2,  1 }, true  }, //26 HT6036 like
  { 330, { 15,  1 }, {  1,  3 }, {  3,  1 }, true  }, //27 MC145026 style (老门铃)
  { 310, { 12,  1 }, {  1,  3 }, {  3,  1 }, true  }, //28 MC145028 variant
  { 200, {  5, 40 }, {  2,  9 }, {  9,  2 }, false }, //29 Intertechno extended (欧)
  { 220, {  6, 36 }, {  1,  4 }, {  4,  1 }, false }, //30 KlikAanKlikUit variant (欧)
  { 140, {  2, 60 }, {  1,  6 }, {  6,  1 }, false }, //31 HS-series short (cheap remotes)
  { 460, { 24,  2 }, {  2,  3 }, {  3,  2 }, false }, //32 Long-sync PT family (garage)
  { 360, {  9, 18 }, {  1,  2 }, {  2,  1 }, false }, //33 Sanwa / Japan variant (433/315)
  { 380, { 10, 20 }, {  1,  2 }, {  2,  1 }, false }, //34 ELPA / JP style (433)

  // --- 37..48 Garage / gate / rolling-code-compatible fixed variants ---
  { 410, { 12, 24 }, {  2,  4 }, {  4,  2 }, false }, //35 Chamberlain fixed-code (garage style)
  { 390, { 10, 22 }, {  2,  3 }, {  3,  2 }, false }, //36 LiftMaster fixed variant
  { 340, {  8, 16 }, {  1,  3 }, {  3,  1 }, false }, //37 Linear / Skylink style
  { 300, {  6, 14 }, {  1,  3 }, {  3,  1 }, false }, //38 Generic garage remote A
  { 280, {  4, 12 }, {  1,  2 }, {  2,  1 }, false }, //39 Generic garage remote B
  { 320, { 16,  1 }, {  1,  2 }, {  2,  1 }, true  }, //40 Rare HT inverted variant
  { 250, { 14,  1 }, {  1,  2 }, {  2,  1 }, true  }, //41 SM-like inverted (sensor)

  // --- 49..60 国内安防 / 433 sensors / module variants / 315 additions ---
  { 320, { 36,  1 }, {  1,  2 }, {  2,  1 }, true  }, //42 SM5212 (保留)
  { 300, { 25,  1 }, {  1,  2 }, {  2,  1 }, true  }, //43 SM5232
  { 310, { 20,  1 }, {  1,  2 }, {  2,  1 }, false }, //44 HS311 系列 (报警器)
  { 260, { 18,  1 }, {  1,  2 }, {  2,  1 }, false  }, //45 XY-FST module default (cheap)
  { 350, {  3, 28 }, {  1,  3 }, {  3,  1 }, false  }, //46 SmartHome sensor family
  { 240, {  2, 30 }, {  1,  2 }, {  2,  1 }, false  }, //47 315 sensor tight timing (315) *
  { 520, {  1, 12 }, {  1,  2 }, {  2,  1 }, false  }, //48 315 long-base variant B (315) *
  { 370, {  1, 20 }, {  1,  3 }, {  3,  1 }, false  }, //49 Vendor-specific 1
  { 330, {  1, 18 }, {  1,  3 }, {  3,  1 }, false  }, //50 Vendor-specific 2
  { 300, {  2, 16 }, {  1,  2 }, {  2,  1 }, false  }, //51 Vendor-specific 3
  { 280, {  2, 14 }, {  1,  2 }, {  2,  1 }, false  }, //52 Vendor-specific 4
  { 360, {  3, 24 }, {  1,  3 }, {  3,  1 }, false  }, //53 Vendor-specific 5
  { 420, {  4, 22 }, {  1,  3 }, {  3,  1 }, false  }, //54 Vendor-specific 6
  { 500, {  6, 20 }, {  1,  2 }, {  2,  1 }, false  }, //55 Vendor-specific 7
  { 460, {  8, 18 }, {  2,  3 }, {  3,  2 }, false  }, //56 Vendor-specific 8
  { 340, { 10, 16 }, {  1,  3 }, {  3,  1 }, false  }, //57 Vendor-specific 9
  { 300, { 12, 14 }, {  1,  2 }, {  2,  1 }, false  }, //58 Vendor-specific 10
  { 380, { 14, 12 }, {  1,  2 }, {  2,  1 }, false  }, //59 Vendor-specific 11
  { 360, { 16, 10 }, {  1,  3 }, {  3,  1 }, false  }  //60 Vendor-specific 12
};

enum {
   numProto = sizeof(proto) / sizeof(proto[0])
};

#if not defined( RCSwitchBDisableReceiving )
VAR_ISR_ATTR volatile unsigned long RCSwitchB::nReceivedValue = 0;
VAR_ISR_ATTR volatile unsigned int RCSwitchB::nReceivedBitlength = 0;
VAR_ISR_ATTR volatile unsigned int RCSwitchB::nReceivedDelay = 0;
VAR_ISR_ATTR volatile unsigned int RCSwitchB::nReceivedProtocol = 0;
VAR_ISR_ATTR int RCSwitchB::nReceiveTolerance = 60;
VAR_ISR_ATTR const unsigned int RCSwitchB::nSeparationLimit = 4300;
// separationLimit: minimum microseconds between received codes, closer codes are ignored.
// according to discussion on issue #14 it might be more suitable to set the separation
// limit to the same time as the 'low' part of the sync signal for the current protocol.
VAR_ISR_ATTR unsigned int RCSwitchB::timings[RCSWITCHB_MAX_CHANGES];

// 延迟解码相关变量
VAR_ISR_ATTR volatile bool RCSwitchB::needsDecode = false;
VAR_ISR_ATTR volatile unsigned int RCSwitchB::savedChangeCount = 0;
#endif

RCSwitchB::RCSwitchB() {
  this->nTransmitterPin = -1;
  this->setRepeatTransmit(10);
  this->setProtocol(1);
  #if not defined( RCSwitchBDisableReceiving )
  this->nReceiverInterrupt = -1;
  this->setReceiveTolerance(60);
  RCSwitchB::nReceivedValue = 0;
  RCSwitchB::needsDecode = false;
  #endif
}

/**
  * Sets the protocol to send.
  */
void RCSwitchB::setProtocol(Protocol protocol) {
  this->protocol = protocol;
}

/**
  * Sets the protocol to send, from a list of predefined protocols
  */
void RCSwitchB::setProtocol(int nProtocol) {
  if (nProtocol < 1 || nProtocol > numProto) {
    nProtocol = 1;  // TODO: trigger an error, e.g. "bad protocol" ???
  }
#if defined(ESP8266) || defined(ESP32)
  this->protocol = proto[nProtocol-1];
#else
  memcpy_P(&this->protocol, &proto[nProtocol-1], sizeof(Protocol));
#endif
}

/**
  * Sets the protocol to send with pulse length in microseconds.
  */
void RCSwitchB::setProtocol(int nProtocol, int nPulseLength) {
  setProtocol(nProtocol);
  this->setPulseLength(nPulseLength);
}


/**
  * Sets pulse length in microseconds
  */
void RCSwitchB::setPulseLength(int nPulseLength) {
  this->protocol.pulseLength = nPulseLength;
}

/**
 * Sets Repeat Transmits
 */
void RCSwitchB::setRepeatTransmit(int nRepeatTransmit) {
  this->nRepeatTransmit = nRepeatTransmit;
}

/**
 * Set Receiving Tolerance
 */
#if not defined( RCSwitchBDisableReceiving )
void RCSwitchB::setReceiveTolerance(int nPercent) {
  RCSwitchB::nReceiveTolerance = nPercent;
}
#endif
  

/**
 * Enable transmissions
 *
 * @param nTransmitterPin    Arduino Pin to which the sender is connected to
 */
void RCSwitchB::enableTransmit(int nTransmitterPin) {
  this->nTransmitterPin = nTransmitterPin;
  pinMode(this->nTransmitterPin, OUTPUT);
}

/**
  * Disable transmissions
  */
void RCSwitchB::disableTransmit() {
  this->nTransmitterPin = -1;
}

/**
 * Switch a remote switch on (Type D REV)
 *
 * @param sGroup        Code of the switch group (A,B,C,D)
 * @param nDevice       Number of the switch itself (1..3)
 */
void RCSwitchB::switchOn(char sGroup, int nDevice) {
  this->sendTriState( this->getCodeWordD(sGroup, nDevice, true) );
}

/**
 * Switch a remote switch off (Type D REV)
 *
 * @param sGroup        Code of the switch group (A,B,C,D)
 * @param nDevice       Number of the switch itself (1..3)
 */
void RCSwitchB::switchOff(char sGroup, int nDevice) {
  this->sendTriState( this->getCodeWordD(sGroup, nDevice, false) );
}

/**
 * Switch a remote switch on (Type C Intertechno)
 *
 * @param sFamily  Familycode (a..f)
 * @param nGroup   Number of group (1..4)
 * @param nDevice  Number of device (1..4)
  */
void RCSwitchB::switchOn(char sFamily, int nGroup, int nDevice) {
  this->sendTriState( this->getCodeWordC(sFamily, nGroup, nDevice, true) );
}

/**
 * Switch a remote switch off (Type C Intertechno)
 *
 * @param sFamily  Familycode (a..f)
 * @param nGroup   Number of group (1..4)
 * @param nDevice  Number of device (1..4)
 */
void RCSwitchB::switchOff(char sFamily, int nGroup, int nDevice) {
  this->sendTriState( this->getCodeWordC(sFamily, nGroup, nDevice, false) );
}

/**
 * Switch a remote switch on (Type B with two rotary/sliding switches)
 *
 * @param nAddressCode  Number of the switch group (1..4)
 * @param nChannelCode  Number of the switch itself (1..4)
 */
void RCSwitchB::switchOn(int nAddressCode, int nChannelCode) {
  this->sendTriState( this->getCodeWordB(nAddressCode, nChannelCode, true) );
}

/**
 * Switch a remote switch off (Type B with two rotary/sliding switches)
 *
 * @param nAddressCode  Number of the switch group (1..4)
 * @param nChannelCode  Number of the switch itself (1..4)
 */
void RCSwitchB::switchOff(int nAddressCode, int nChannelCode) {
  this->sendTriState( this->getCodeWordB(nAddressCode, nChannelCode, false) );
}

/**
 * Deprecated, use switchOn(const char* sGroup, const char* sDevice) instead!
 * Switch a remote switch on (Type A with 10 pole DIP switches)
 *
 * @param sGroup        Code of the switch group (refers to DIP switches 1..5 where "1" = on and "0" = off, if all DIP switches are on it's "11111")
 * @param nChannelCode  Number of the switch itself (1..5)
 */
void RCSwitchB::switchOn(const char* sGroup, int nChannel) {
  const char* code[6] = { "00000", "10000", "01000", "00100", "00010", "00001" };
  this->switchOn(sGroup, code[nChannel]);
}

/**
 * Deprecated, use switchOff(const char* sGroup, const char* sDevice) instead!
 * Switch a remote switch off (Type A with 10 pole DIP switches)
 *
 * @param sGroup        Code of the switch group (refers to DIP switches 1..5 where "1" = on and "0" = off, if all DIP switches are on it's "11111")
 * @param nChannelCode  Number of the switch itself (1..5)
 */
void RCSwitchB::switchOff(const char* sGroup, int nChannel) {
  const char* code[6] = { "00000", "10000", "01000", "00100", "00010", "00001" };
  this->switchOff(sGroup, code[nChannel]);
}

/**
 * Switch a remote switch on (Type A with 10 pole DIP switches)
 *
 * @param sGroup        Code of the switch group (refers to DIP switches 1..5 where "1" = on and "0" = off, if all DIP switches are on it's "11111")
 * @param sDevice       Code of the switch device (refers to DIP switches 6..10 (A..E) where "1" = on and "0" = off, if all DIP switches are on it's "11111")
 */
void RCSwitchB::switchOn(const char* sGroup, const char* sDevice) {
  this->sendTriState( this->getCodeWordA(sGroup, sDevice, true) );
}

/**
 * Switch a remote switch off (Type A with 10 pole DIP switches)
 *
 * @param sGroup        Code of the switch group (refers to DIP switches 1..5 where "1" = on and "0" = off, if all DIP switches are on it's "11111")
 * @param sDevice       Code of the switch device (refers to DIP switches 6..10 (A..E) where "1" = on and "0" = off, if all DIP switches are on it's "11111")
 */
void RCSwitchB::switchOff(const char* sGroup, const char* sDevice) {
  this->sendTriState( this->getCodeWordA(sGroup, sDevice, false) );
}


/**
 * Returns a char[13], representing the code word to be send.
 *
 */
char* RCSwitchB::getCodeWordA(const char* sGroup, const char* sDevice, bool bStatus) {
  static char sReturn[13];
  int nReturnPos = 0;

  for (int i = 0; i < 5; i++) {
    sReturn[nReturnPos++] = (sGroup[i] == '0') ? 'F' : '0';
  }

  for (int i = 0; i < 5; i++) {
    sReturn[nReturnPos++] = (sDevice[i] == '0') ? 'F' : '0';
  }

  sReturn[nReturnPos++] = bStatus ? '0' : 'F';
  sReturn[nReturnPos++] = bStatus ? 'F' : '0';

  sReturn[nReturnPos] = '\0';
  return sReturn;
}

/**
 * Encoding for type B switches with two rotary/sliding switches.
 *
 * The code word is a tristate word and with following bit pattern:
 *
 * +-----------------------------+-----------------------------+----------+------------+
 * | 4 bits address              | 4 bits address              | 3 bits   | 1 bit      |
 * | switch group                | switch number               | not used | on / off   |
 * | 1=0FFF 2=F0FF 3=FF0F 4=FFF0 | 1=0FFF 2=F0FF 3=FF0F 4=FFF0 | FFF      | on=F off=0 |
 * +-----------------------------+-----------------------------+----------+------------+
 *
 * @param nAddressCode  Number of the switch group (1..4)
 * @param nChannelCode  Number of the switch itself (1..4)
 * @param bStatus       Whether to switch on (true) or off (false)
 *
 * @return char[13], representing a tristate code word of length 12
 */
char* RCSwitchB::getCodeWordB(int nAddressCode, int nChannelCode, bool bStatus) {
  static char sReturn[13];
  int nReturnPos = 0;

  if (nAddressCode < 1 || nAddressCode > 4 || nChannelCode < 1 || nChannelCode > 4) {
    return 0;
  }

  for (int i = 1; i <= 4; i++) {
    sReturn[nReturnPos++] = (nAddressCode == i) ? '0' : 'F';
  }

  for (int i = 1; i <= 4; i++) {
    sReturn[nReturnPos++] = (nChannelCode == i) ? '0' : 'F';
  }

  sReturn[nReturnPos++] = 'F';
  sReturn[nReturnPos++] = 'F';
  sReturn[nReturnPos++] = 'F';

  sReturn[nReturnPos++] = bStatus ? 'F' : '0';

  sReturn[nReturnPos] = '\0';
  return sReturn;
}

/**
 * Like getCodeWord (Type C = Intertechno)
 */
char* RCSwitchB::getCodeWordC(char sFamily, int nGroup, int nDevice, bool bStatus) {
  static char sReturn[13];
  int nReturnPos = 0;

  int nFamily = (int)sFamily - 'a';
  if ( nFamily < 0 || nFamily > 15 || nGroup < 1 || nGroup > 4 || nDevice < 1 || nDevice > 4) {
    return 0;
  }
  
  // encode the family into four bits
  sReturn[nReturnPos++] = (nFamily & 1) ? 'F' : '0';
  sReturn[nReturnPos++] = (nFamily & 2) ? 'F' : '0';
  sReturn[nReturnPos++] = (nFamily & 4) ? 'F' : '0';
  sReturn[nReturnPos++] = (nFamily & 8) ? 'F' : '0';

  // encode the device and group
  sReturn[nReturnPos++] = ((nDevice-1) & 1) ? 'F' : '0';
  sReturn[nReturnPos++] = ((nDevice-1) & 2) ? 'F' : '0';
  sReturn[nReturnPos++] = ((nGroup-1) & 1) ? 'F' : '0';
  sReturn[nReturnPos++] = ((nGroup-1) & 2) ? 'F' : '0';

  // encode the status code
  sReturn[nReturnPos++] = '0';
  sReturn[nReturnPos++] = 'F';
  sReturn[nReturnPos++] = 'F';
  sReturn[nReturnPos++] = bStatus ? 'F' : '0';

  sReturn[nReturnPos] = '\0';
  return sReturn;
}

/**
 * Encoding for the REV Switch Type
 *
 * The code word is a tristate word and with following bit pattern:
 *
 * +-----------------------------+-------------------+----------+--------------+
 * | 4 bits address              | 3 bits address    | 3 bits   | 2 bits       |
 * | switch group                | device number     | not used | on / off     |
 * | A=1FFF B=F1FF C=FF1F D=FFF1 | 1=0FF 2=F0F 3=FF0 | 000      | on=10 off=01 |
 * +-----------------------------+-------------------+----------+--------------+
 *
 * Source: http://www.the-intruder.net/funksteckdosen-von-rev-uber-arduino-ansteuern/
 *
 * @param sGroup        Name of the switch group (A..D, resp. a..d) 
 * @param nDevice       Number of the switch itself (1..3)
 * @param bStatus       Whether to switch on (true) or off (false)
 *
 * @return char[13], representing a tristate code word of length 12
 */
char* RCSwitchB::getCodeWordD(char sGroup, int nDevice, bool bStatus) {
  static char sReturn[13];
  int nReturnPos = 0;

  // sGroup must be one of the letters in "abcdABCD"
  int nGroup = (sGroup >= 'a') ? (int)sGroup - 'a' : (int)sGroup - 'A';
  if ( nGroup < 0 || nGroup > 3 || nDevice < 1 || nDevice > 3) {
    return 0;
  }

  for (int i = 0; i < 4; i++) {
    sReturn[nReturnPos++] = (nGroup == i) ? '1' : 'F';
  }

  for (int i = 1; i <= 3; i++) {
    sReturn[nReturnPos++] = (nDevice == i) ? '1' : 'F';
  }

  sReturn[nReturnPos++] = '0';
  sReturn[nReturnPos++] = '0';
  sReturn[nReturnPos++] = '0';

  sReturn[nReturnPos++] = bStatus ? '1' : '0';
  sReturn[nReturnPos++] = bStatus ? '0' : '1';

  sReturn[nReturnPos] = '\0';
  return sReturn;
}

/**
 * @param sCodeWord   a tristate code word consisting of the letter 0, 1, F
 */
void RCSwitchB::sendTriState(const char* sCodeWord) {
  // turn the tristate code word into the corresponding bit pattern, then send it
  unsigned long code = 0;
  unsigned int length = 0;
  for (const char* p = sCodeWord; *p; p++) {
    code <<= 2L;
    switch (*p) {
      case '0':
        // bit pattern 00
        break;
      case 'F':
        // bit pattern 01
        code |= 1L;
        break;
      case '1':
        // bit pattern 11
        code |= 3L;
        break;
    }
    length += 2;
  }
  this->send(code, length);
}

/**
 * @param sCodeWord   a binary code word consisting of the letter 0, 1
 */
void RCSwitchB::send(const char* sCodeWord) {
  // turn the tristate code word into the corresponding bit pattern, then send it
  unsigned long code = 0;
  unsigned int length = 0;
  for (const char* p = sCodeWord; *p; p++) {
    code <<= 1L;
    if (*p != '0')
      code |= 1L;
    length++;
  }
  this->send(code, length);
}

/**
 * Transmit the first 'length' bits of the integer 'code'. The
 * bits are sent from MSB to LSB, i.e., first the bit at position length-1,
 * then the bit at position length-2, and so on, till finally the bit at position 0.
 */
void RCSwitchB::send(unsigned long code, unsigned int length) {
  if (this->nTransmitterPin == -1)
    return;

#if not defined( RCSwitchBDisableReceiving )
  // make sure the receiver is disabled while we transmit
  int nReceiverInterrupt_backup = nReceiverInterrupt;
  if (nReceiverInterrupt_backup != -1) {
    this->disableReceive();
  }
#endif

  for (int nRepeat = 0; nRepeat < nRepeatTransmit; nRepeat++) {
    for (int i = length-1; i >= 0; i--) {
      if (code & (1L << i))
        this->transmit(protocol.one);
      else
        this->transmit(protocol.zero);
    }
    this->transmit(protocol.syncFactor);
  }

  // Disable transmit after sending (i.e., for inverted protocols)
  digitalWrite(this->nTransmitterPin, LOW);

  // 注意：不再自动恢复接收模式，由调用者决定是否需要重新启用接收
  // 这样可以避免重复注册中断导致的崩溃
}

/**
 * Transmit a single high-low pulse.
 */
void RCSwitchB::transmit(HighLow pulses) {
  uint8_t firstLogicLevel = (this->protocol.invertedSignal) ? LOW : HIGH;
  uint8_t secondLogicLevel = (this->protocol.invertedSignal) ? HIGH : LOW;
  
  digitalWrite(this->nTransmitterPin, firstLogicLevel);
  delayMicroseconds( this->protocol.pulseLength * pulses.high);
  digitalWrite(this->nTransmitterPin, secondLogicLevel);
  delayMicroseconds( this->protocol.pulseLength * pulses.low);
}


#if not defined( RCSwitchBDisableReceiving )
/**
 * Enable receiving data
 */
void RCSwitchB::enableReceive(int interrupt) {
  this->nReceiverInterrupt = interrupt;
  this->enableReceive();
}

void RCSwitchB::enableReceive() {
  if (this->nReceiverInterrupt != -1) {
    RCSwitchB::nReceivedValue = 0;
    RCSwitchB::nReceivedBitlength = 0;
#if defined(RaspberryPi) // Raspberry Pi
    wiringPiISR(this->nReceiverInterrupt, INT_EDGE_BOTH, &handleInterrupt);
#elif defined(ESP32)
    // ESP32 需要使用 digitalPinToInterrupt 转换
    attachInterrupt(digitalPinToInterrupt(this->nReceiverInterrupt), handleInterrupt, CHANGE);
#else // Arduino
    attachInterrupt(this->nReceiverInterrupt, handleInterrupt, CHANGE);
#endif
  }
}

/**
 * Disable receiving data
 */
void RCSwitchB::disableReceive() {
#if not defined(RaspberryPi) // Arduino
  // 只有在有效的中断号时才detach，避免GPIO错误
  if (this->nReceiverInterrupt != -1) {
#if defined(ESP32)
    detachInterrupt(digitalPinToInterrupt(this->nReceiverInterrupt));
#else
    detachInterrupt(this->nReceiverInterrupt);
#endif
  }
#endif // For Raspberry Pi (wiringPi) you can't unregister the ISR
  this->nReceiverInterrupt = -1;
}

bool RCSwitchB::available() {
  return RCSwitchB::nReceivedValue != 0;
}

void RCSwitchB::resetAvailable() {
  RCSwitchB::nReceivedValue = 0;
}

unsigned long RCSwitchB::getReceivedValue() {
  return RCSwitchB::nReceivedValue;
}

unsigned int RCSwitchB::getReceivedBitlength() {
  return RCSwitchB::nReceivedBitlength;
}

unsigned int RCSwitchB::getReceivedDelay() {
  return RCSwitchB::nReceivedDelay;
}

unsigned int RCSwitchB::getReceivedProtocol() {
  return RCSwitchB::nReceivedProtocol;
}

unsigned int* RCSwitchB::getReceivedRawdata() {
  return RCSwitchB::timings;
}

/* helper function for the receiveProtocol method */
// 使用内联实现替代 abs()，因为标准库 abs() 可能不在 IRAM 中
static inline unsigned int RECEIVE_ATTR diff(int A, int B) {
  int d = A - B;
  return (d < 0) ? (unsigned int)(-d) : (unsigned int)d;
}

/**
 *
 */
bool RCSwitchB::receiveProtocol(const int p, unsigned int changeCount) {
    // 必须复制到栈上，避免中断时访问 Flash (proto 在 RODATA 段)
    Protocol pro;
#if defined(ESP8266) || defined(ESP32)
    pro = proto[p-1];  // 复制到栈 (DRAM)
#else
    memcpy_P(&pro, &proto[p-1], sizeof(Protocol));
#endif

    unsigned long code = 0;
    //Assuming the longer pulse length is the pulse captured in timings[0]
    const unsigned int syncLengthInPulses =  ((pro.syncFactor.low) > (pro.syncFactor.high)) ? (pro.syncFactor.low) : (pro.syncFactor.high);
    const unsigned int delay = RCSwitchB::timings[0] / syncLengthInPulses;
    const unsigned int delayTolerance = delay * RCSwitchB::nReceiveTolerance / 100;
    
    /* For protocols that start low, the sync period looks like
     *               _________
     * _____________|         |XXXXXXXXXXXX|
     *
     * |--1st dur--|-2nd dur-|-Start data-|
     *
     * The 3rd saved duration starts the data.
     *
     * For protocols that start high, the sync period looks like
     *
     *  ______________
     * |              |____________|XXXXXXXXXXXXX|
     *
     * |-filtered out-|--1st dur--|--Start data--|
     *
     * The 2nd saved duration starts the data
     */
    const unsigned int firstDataTiming = (pro.invertedSignal) ? (2) : (1);

    for (unsigned int i = firstDataTiming; i < changeCount - 1; i += 2) {
        code <<= 1;
        if (diff(RCSwitchB::timings[i], delay * pro.zero.high) < delayTolerance &&
            diff(RCSwitchB::timings[i + 1], delay * pro.zero.low) < delayTolerance) {
            // zero
        } else if (diff(RCSwitchB::timings[i], delay * pro.one.high) < delayTolerance &&
                   diff(RCSwitchB::timings[i + 1], delay * pro.one.low) < delayTolerance) {
            // one
            code |= 1;
        } else {
            // Failed
            return false;
        }
    }

    if (changeCount > 7) {    // ignore very short transmissions: no device sends them, so this must be noise
        RCSwitchB::nReceivedValue = code;
        RCSwitchB::nReceivedBitlength = (changeCount - 1) / 2;
        RCSwitchB::nReceivedDelay = delay;
        RCSwitchB::nReceivedProtocol = p;
        return true;
    }

    return false;
}

void RECEIVE_ATTR RCSwitchB::handleInterrupt() {

  static unsigned int changeCount = 0;
  static unsigned long lastTime = 0;
  static unsigned int repeatCount = 0;

  const long time = micros();
  const unsigned int duration = time - lastTime;

  if (duration > RCSwitchB::nSeparationLimit) {
    // A long stretch without signal level change occurred. This could
    // be the gap between two transmission.
    if ((repeatCount==0) || (diff(duration, RCSwitchB::timings[0]) < 200)) {
      // This long signal is close in length to the long signal which
      // started the previously recorded timings; this suggests that
      // it may indeed by a a gap between two transmissions (we assume
      // here that a sender will send the signal multiple times,
      // with roughly the same gap between them).
      repeatCount++;
      if (repeatCount == 2) {
        // 不在ISR中解码，只设置标志让外部任务处理
        if (!RCSwitchB::needsDecode && changeCount > 7) {
          RCSwitchB::savedChangeCount = changeCount;
          RCSwitchB::needsDecode = true;
        }
        repeatCount = 0;
      }
    }
    changeCount = 0;
  }
 
  // detect overflow
  if (changeCount >= RCSWITCHB_MAX_CHANGES) {
    changeCount = 0;
    repeatCount = 0;
  }

  RCSwitchB::timings[changeCount++] = duration;
  lastTime = time;  
}

// 在非ISR上下文中调用进行解码
void RCSwitchB::tryDecode() {
  if (!RCSwitchB::needsDecode) {
    return;
  }
  
  unsigned int changeCount = RCSwitchB::savedChangeCount;
  
  for(unsigned int i = 1; i <= numProto; i++) {
    if (receiveProtocol(i, changeCount)) {
      // receive succeeded for protocol i
      RCSwitchB::needsDecode = false;
      return;
    }
  }
  
  // 尝试动态推断协议
  if (inferAndDecode(changeCount)) {
    // note: nReceivedProtocol == 0 indicates a dynamically inferred protocol
  }
  
  RCSwitchB::needsDecode = false;
}
#endif

// Heuristic: infer base pulse length from timings[] captured.
// Returns inferred pulseLength in microseconds (rounded).
unsigned int RCSwitchB::inferPulseLengthFromTimings(unsigned int changeCount) {
    // Find smallest nonzero timing (filter noise)
    unsigned int minT = 0xFFFFFFFFu;
    for (unsigned int i = 0; i < changeCount; ++i) {
        unsigned int t = RCSwitchB::timings[i];
        if (t > 20 && t < minT) minT = t;
    }
    if (minT == 0xFFFFFFFFu) return 350; // fallback

    // Many protocols use multiples of a basic pulse; approximate by dividing minT by small ints to find a reasonable base.
    // But simpler: try to find GCD-like base by scanning divisions.
    unsigned int best = minT;
    // Try divisors 1..8
    for (unsigned int d = 1; d <= 8; ++d) {
        unsigned int cand = minT / d;
        if (cand < 50) break; // too small
        // check how well timings align to multiples of cand
        int matches = 0;
        for (unsigned int i = 0; i < changeCount; ++i) {
            unsigned int q = (RCSwitchB::timings[i] + cand/2) / cand;
            unsigned int approx = q * cand;
            unsigned int diff = (approx > RCSwitchB::timings[i]) ? approx - RCSwitchB::timings[i] : RCSwitchB::timings[i] - approx;
            if (diff < cand / 3) matches++;
        }
        if (matches > (int)(changeCount * 0.6)) { best = cand; break; }
    }
    // if nothing better, return minT as base
    return best;
}

// Try to infer a Protocol from captured timings and decode.
// If successful, it sets RCSwitch::nReceivedValue / nReceivedBitlength etc and returns true.
bool RCSwitchB::inferAndDecode(unsigned int changeCount) {
    if (changeCount < 6) return false;

    unsigned int base = inferPulseLengthFromTimings(changeCount);

    // Build array of normalized pairs (high, low) in units of 'base'
    unsigned int pairs = (changeCount - 1) / 2;
    // store pair counts keyed by (high<<8 | low) up to reasonable max high/low (<=255)
    const int MAXPAIRS = 256;
    unsigned int pairKeys[MAXPAIRS];
    unsigned int pairCounts[MAXPAIRS];
    int pairIdx = 0;

    for (unsigned int i = 1; i < changeCount - 1; i += 2) {
        unsigned int high = (RCSwitchB::timings[i] + base/2) / base;
        unsigned int low  = (RCSwitchB::timings[i+1] + base/2) / base;
        if (high == 0) high = 1;
        if (low == 0) low = 1;
        unsigned int key = (high << 8) | (low & 0xFF);

        // find or add
        int found = -1;
        for (int k = 0; k < pairIdx; ++k) {
            if (pairKeys[k] == key) { found = k; break; }
        }
        if (found == -1) {
            if (pairIdx < MAXPAIRS) {
                pairKeys[pairIdx] = key;
                pairCounts[pairIdx] = 1;
                pairIdx++;
            }
        } else {
            pairCounts[found]++;
        }
    }

    if (pairIdx < 2) return false; // cannot determine 0/1 with <2 distinct pairs

    // Find top two most frequent pairs (likely zero and one)
    int top1 = 0, top2 = 1;
    if (pairCounts[top2] > pairCounts[top1]) { int t=top1; top1=top2; top2=t; }
    for (int i = 2; i < pairIdx; ++i) {
        if (pairCounts[i] > pairCounts[top1]) { top2 = top1; top1 = i; }
        else if (pairCounts[i] > pairCounts[top2]) { top2 = i; }
    }

    unsigned int key0 = pairKeys[top1]; // assume more frequent => zero OR one (we'll test both)
    unsigned int key1 = pairKeys[top2];

    // compose guessed Protocol
    RCSwitchB::Protocol guess;
    guess.pulseLength = base;
    guess.syncFactor.high = 1;
    guess.syncFactor.low  = (RCSwitchB::timings[0] + base/2) / base; // heuristic: long low after initial high
    guess.invertedSignal = false;

    // decode attempt helper using guessed mapping of which key => zero/one
    auto tryDecodeWithMapping = [&](unsigned int key_zero, unsigned int key_one)->bool {
        unsigned long code = 0;
        unsigned int bitlen = 0;
        const unsigned int delay = base;
        const unsigned int delayTolerance = delay * RCSwitchB::nReceiveTolerance / 100;

        // fill guess.zero/one
        guess.zero.high = (key_zero >> 8) & 0xFF;
        guess.zero.low  = (key_zero & 0xFF);
        guess.one.high  = (key_one >> 8) & 0xFF;
        guess.one.low   = (key_one & 0xFF);

        // attempt to parse pairs
        for (unsigned int i = 1; i < changeCount - 1; i += 2) {
            unsigned int h = (RCSwitchB::timings[i] + base/2) / base;
            unsigned int l = (RCSwitchB::timings[i+1] + base/2) / base;

            // match zero?
            if ( (h == guess.zero.high && l == guess.zero.low) ) {
                code <<= 1; // zero
            } else if ( (h == guess.one.high && l == guess.one.low) ) {
                code <<= 1; code |= 1;
            } else {
                // allow +/-1 tolerance for multiples
                if ( (abs((int)h - (int)guess.zero.high) <= 1 && abs((int)l - (int)guess.zero.low) <= 1) ) {
                    code <<= 1;
                } else if ( (abs((int)h - (int)guess.one.high) <= 1 && abs((int)l - (int)guess.one.low) <= 1) ) {
                    code <<= 1; code |= 1;
                } else {
                    return false; // mismatch
                }
            }
            bitlen++;
        }

        if (bitlen > 8) { // reasonable min bitlength
            RCSwitchB::nReceivedValue = code;
            RCSwitchB::nReceivedBitlength = bitlen;
            RCSwitchB::nReceivedDelay = base;
            // we cannot set a protocol index here — mark as 0 for 'dynamic'
            RCSwitchB::nReceivedProtocol = 0; 
            return true;
        }
        return false;
    };

    // Try both guess orders: (most frequent -> zero) and swapped
    if (tryDecodeWithMapping(key0, key1)) return true;
    if (tryDecodeWithMapping(key1, key0)) return true;

    // last resort: try inverted signal (flip logic)
    guess.invertedSignal = true;
    if (tryDecodeWithMapping(key0, key1)) return true;
    if (tryDecodeWithMapping(key1, key0)) return true;

    return false;
}
