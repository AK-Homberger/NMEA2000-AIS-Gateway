/*
  This code is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  This code is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include <Arduino.h>
#include "ais_decoder.h"
#include "default_sentence_parser.h"

const double pi = 3.1415926535897932384626433832795;
const double knToms = 1852.0 / 3600.0;
const double degToRad = pi / 180.0;
const double nmTom = 1.852 * 1000;

uint16_t DaysSince1970 = 0;

class MyAisDecoder : public AIS::AisDecoder
{
  public:
    MyAisDecoder()
    {}

  protected:
    virtual void onType123(unsigned int _uMsgType, unsigned int _uMmsi, unsigned int _uNavstatus,
                           int _iRot, unsigned int _uSog, bool _bPosAccuracy,
                           long _iPosLon, long _iPosLat, int _iCog, int _iHeading, int _Repeat, bool _Raim,
                           unsigned int _timestamp, unsigned int _maneuver_i) override {

      // Serial.println("123");

      tN2kMsg N2kMsg;

      // PGN129038
      SetN2kAISClassAPosition(N2kMsg, _uMsgType, (tN2kAISRepeat)_Repeat, _uMmsi,
                              _iPosLat / 600000.0, _iPosLon / 600000.0,
                              _bPosAccuracy, _Raim, _timestamp,
                              _iCog * degToRad, _uSog * knToms / 10.0,
                              _iHeading * degToRad, _iRot, (tN2kAISNavStatus)_uNavstatus);

      NMEA2000.SendMsg(N2kMsg);
    }

    virtual void onType411(unsigned int , unsigned int , unsigned int , unsigned int , unsigned int , unsigned int , unsigned int , unsigned int , bool , int , int ) override {
      //Serial.println("411");
    }

    virtual void onType5(unsigned int _uMsgType, unsigned int _uMmsi, unsigned int _uImo, const std::string &_strCallsign,
                         const std::string &_strName,
                         unsigned int _uType, unsigned int _uToBow, unsigned int _uToStern,
                         unsigned int _uToPort, unsigned int _uToStarboard, unsigned int _uFixType,
                         unsigned int _uEtaMonth, unsigned int _uEtaDay, unsigned int _uEtaHour,
                         unsigned int _uEtaMinute, unsigned int _uDraught,
                         const std::string &_strDestination, unsigned int _ais_version,
                         unsigned int _repeat, bool _dte) override {

      // Serial.println("5");

      // Necessary due to conflict with TimeLib.h (redefinition of tmElements_t)

      time_t t = DaysSince1970 * (24UL * 3600UL);
      tmElements_t tm;

      tNMEA0183Msg::breakTime(t, tm);

      //tNMEA0183Msg::SetYear(tm, 2020);
      tNMEA0183Msg::SetMonth(tm, _uEtaMonth);
      tNMEA0183Msg::SetDay(tm, _uEtaDay);
      tNMEA0183Msg::SetHour(tm, 0);
      tNMEA0183Msg::SetMin(tm, 0);
      tNMEA0183Msg::SetSec(tm, 0);

      uint16_t eta_days = tNMEA0183Msg::makeTime(tm) / (24UL * 3600UL);

      tN2kMsg N2kMsg;
      char CS[30];
      char Name[30];
      char Dest[30];

      strncpy(CS, _strCallsign.c_str(), sizeof(CS));
      strncpy(Name, _strName.c_str(), sizeof(Name));
      strncpy(Dest, _strDestination.c_str(), sizeof(Dest));

      // PGN129794
      SetN2kAISClassAStatic(N2kMsg, _uMsgType, (tN2kAISRepeat) _repeat, _uMmsi,
                            _uImo, CS, Name, _uType, _uToBow + _uToStern,
                            _uToPort + _uToStarboard, _uToStarboard, _uToBow, eta_days,
                            (_uEtaHour * 3600) + (_uEtaMinute * 60), _uDraught / 10.0, Dest,
                            (tN2kAISVersion) _ais_version, (tN2kGNSStype) _uFixType,
                            (tN2kAISDTE) _dte, (tN2kAISTranceiverInfo) _ais_version);

      NMEA2000.SendMsg(N2kMsg);
    }

    virtual void onType9(unsigned int , unsigned int , bool , int , int , int , unsigned int ) override {
      //Serial.println("9");
    }

    virtual void onType14(unsigned int _repeat, unsigned int _uMmsi,
                          const std::string &_strText, int _iPayloadSizeBits) override {

      tN2kMsg N2kMsg;
      char Text[162];
      strncpy(Text, _strText.c_str(), sizeof(Text));

      N2kMsg.SetPGN(129802UL);
      N2kMsg.Priority = 4;
      N2kMsg.Destination = 255;  // Redundant, PGN129802 is broadcast by default.
      N2kMsg.AddByte((_repeat & 0x03) << 6 | (14 & 0x3f));
      N2kMsg.Add4ByteUInt(_uMmsi);
      N2kMsg.AddByte(0);

      if (strlen(Text) == 0) {
        N2kMsg.AddByte(0x03); N2kMsg.AddByte(0x01); N2kMsg.AddByte(0x00);
      } else {
        N2kMsg.AddByte(strlen(Text) + 2); N2kMsg.AddByte(0x01);
        for (int i = 0; i < strlen(Text); i++)
          N2kMsg.AddByte(Text[i]);
      }

      NMEA2000.SendMsg(N2kMsg);
    }

    virtual void onType18(unsigned int _uMsgType, unsigned int _uMmsi, unsigned int _uSog, bool _bPosAccuracy,
                          long _iPosLon, long _iPosLat, int _iCog, int _iHeading, bool _raim, unsigned int _repeat,
                          bool _unit, bool _diplay, bool _dsc, bool _band, bool _msg22, bool _assigned,
                          unsigned int _timestamp, bool _state ) override {
      //Serial.println("18");

      tN2kMsg N2kMsg;

      // PGN129039
      SetN2kAISClassBPosition(N2kMsg, _uMsgType, (tN2kAISRepeat) _repeat, _uMmsi,
                              _iPosLat / 600000.0, _iPosLon / 600000.0, _bPosAccuracy, _raim,
                              _timestamp, _iCog * degToRad, _uSog * knToms / 10.0,
                              _iHeading * degToRad, (tN2kAISUnit) _unit,
                              _diplay, _dsc, _band, _msg22, (tN2kAISMode) _assigned, _state);

      NMEA2000.SendMsg(N2kMsg);
    }

    virtual void onType19(unsigned int _uMmsi, unsigned int _uSog, bool _bPosAccuracy, int _iPosLon, int _iPosLat,
                          int _iCog, int _iHeading, const std::string &_strName, unsigned int _uType,
                          unsigned int _uToBow, unsigned int _uToStern, unsigned int _uToPort,
                          unsigned int _uToStarboard, unsigned int _timestamp, unsigned int _fixtype,
                          bool _dte, bool _assigned, unsigned int _repeat, bool _raim) override {

      //Serial.println("19");
      tN2kMsg N2kMsg;

      // PGN129040

      char Name[21] = "";
      strncpy(Name, _strName.c_str(), sizeof(Name));

      N2kMsg.SetPGN(129040UL);
      N2kMsg.Priority = 4;
      N2kMsg.AddByte((_repeat & 0x03) << 6 | (19 & 0x3f));
      N2kMsg.Add4ByteUInt(_uMmsi);
      N2kMsg.Add4ByteDouble(_iPosLon / 600000.0, 1e-07);
      N2kMsg.Add4ByteDouble(_iPosLat / 600000.0, 1e-07);
      N2kMsg.AddByte((_timestamp & 0x3f) << 2 | (_raim & 0x01) << 1 | (_bPosAccuracy & 0x01));
      N2kMsg.Add2ByteUDouble(_iCog * degToRad, 1e-04);
      N2kMsg.Add2ByteUDouble(_uSog * knToms / 10.0, 0.01);
      N2kMsg.AddByte(0xff); // Regional Application
      N2kMsg.AddByte(0xff); // Regional Application
      N2kMsg.AddByte(_uType );
      N2kMsg.Add2ByteUDouble(_iHeading * degToRad, 1e-04);
      N2kMsg.AddByte(_fixtype << 4);
      N2kMsg.Add2ByteDouble(_uToBow + _uToStern, 0.1);
      N2kMsg.Add2ByteDouble(_uToPort + _uToStarboard, 0.1);
      N2kMsg.Add2ByteDouble(_uToStarboard, 0.1);
      N2kMsg.Add2ByteDouble(_uToBow, 0.1);
      N2kMsg.AddStr(Name, 20);
      N2kMsg.AddByte((_dte & 0x01) | (_assigned & 0x01) << 1) ;
      N2kMsg.AddByte(0);
      
      NMEA2000.SendMsg(N2kMsg);      
    }


    virtual void onType21(unsigned int , unsigned int , const std::string &, bool , int , int , unsigned int , unsigned int , unsigned int , unsigned int ) override {
      //Serial.println("21");
    }

    virtual void onType24A(unsigned int _uMsgType, unsigned int _repeat, unsigned int _uMmsi,
                           const std::string &_strName) override {
      //Serial.println("24A");

      tN2kMsg N2kMsg;
      char Name[30];
      strncpy(Name, _strName.c_str(), sizeof(Name));

      // PGN129809
      SetN2kAISClassBStaticPartA(N2kMsg, _uMsgType, (tN2kAISRepeat) _repeat, _uMmsi, Name);

      NMEA2000.SendMsg(N2kMsg);
    }

    virtual void onType24B(unsigned int _uMsgType, unsigned int _repeat, unsigned int _uMmsi,
                           const std::string &_strCallsign, unsigned int _uType,
                           unsigned int _uToBow, unsigned int _uToStern, unsigned int _uToPort,
                           unsigned int _uToStarboard, const std::string &_strVendor) override {

      // Serial.println("24B");


      tN2kMsg N2kMsg;
      char CS[30];
      char Vendor[30];

      strncpy(CS, _strCallsign.c_str(), sizeof(CS));
      strncpy(Vendor, _strVendor.c_str(), sizeof(Vendor));

      // PGN129810
      SetN2kAISClassBStaticPartB(N2kMsg, _uMsgType, (tN2kAISRepeat)_repeat, _uMmsi,
                                 _uType, Vendor, CS, _uToBow + _uToStern, _uToPort + _uToStarboard,
                                 _uToStarboard, _uToBow, _uMmsi);

      NMEA2000.SendMsg(N2kMsg);
    }

    virtual void onType27(unsigned int , unsigned int , unsigned int , bool , int , int , int ) override {
      //Serial.println("27");
    }

    virtual void onSentence(const AIS::StringRef &_Stc) override {
      //Serial.printf("Sentence: %s\n", _Stc);
    }

    virtual void onMessage(const AIS::StringRef &, const AIS::StringRef &, const AIS::StringRef &) override {}

    virtual void onNotDecoded(const AIS::StringRef &, int ) override {}

    virtual void onDecodeError(const AIS::StringRef &_strMessage, const std::string &_strError) override {
      std::string msg(_strMessage.data(), _strMessage.size());
      AIS::stripTrailingWhitespace(msg);

      Serial.printf("%s [%s]\n", _strError.c_str(), msg.c_str());
    }

    virtual void onParseError(const AIS::StringRef &_strMessage, const std::string &_strError) override {
      std::string msg(_strMessage.data(), _strMessage.size());
      AIS::stripTrailingWhitespace(msg);

      Serial.printf("%s [%s]\n", _strError.c_str(), msg.c_str());
    }
};
