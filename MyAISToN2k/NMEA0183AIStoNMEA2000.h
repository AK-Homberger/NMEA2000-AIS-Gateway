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

#include "ais_decoder.h"
#include "default_sentence_parser.h"

const double pi = 3.1415926535897932384626433832795;
const double knToms = 1852.0 / 3600.0;
const double degToRad = pi / 180.0;
const double nmTom = 1.852 * 1000;

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

      tN2kMsg N2kMsg;
      char CS[30];
      char Name[30];
      char Dest[30];

      strcpy(CS, _strCallsign.c_str());
      strcpy(Name, _strName.c_str());
      strcpy(Dest, _strDestination.c_str());

      /*
        unsigned long tNMEA0183Msg::DaysToNMEA0183Date(unsigned long val) {
        if ( val!=NMEA0183UInt32NA  ) {
        tmElements_t tm;
        time_t t=val*SECS_PER_DAY; //daysToTime_t((val));
        breakTime(t, tm);
        val=tm.Day*10000+(tm.Month)*100+(tm.Year+1970-2000);
        }

        return val;
        }
      */

      // PGN129794
      SetN2kAISClassAStatic(N2kMsg, _uMsgType, (tN2kAISRepeat) _repeat, _uMmsi,
                            _uImo, CS, Name, _uType, _uToBow + _uToStern,
                            _uToPort + _uToStarboard, _uToStarboard, _uToBow, (_uEtaMonth * 30) + _uEtaDay,
                            (_uEtaHour * 3600) + (_uEtaMinute * 60), _uDraught / 10.0, Dest,
                            (tN2kAISVersion) _ais_version, (tN2kGNSStype) _uFixType,
                            (tN2kAISDTE) _dte, (tN2kAISTranceiverInfo) _ais_version);

      NMEA2000.SendMsg(N2kMsg);
    }

    virtual void onType9(unsigned int , unsigned int , bool , int , int , int , unsigned int ) override {
      //Serial.println("9");
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

    virtual void onType19(unsigned int , unsigned int , bool , int , int , int , int , const std::string &, unsigned int , unsigned int , unsigned int , unsigned int , unsigned int ) override {
      //Serial.println("19");
    }

    virtual void onType21(unsigned int , unsigned int , const std::string &, bool , int , int , unsigned int , unsigned int , unsigned int , unsigned int ) override {
      //Serial.println("21");
    }

    virtual void onType24A(unsigned int _uMsgType, unsigned int _repeat, unsigned int _uMmsi,
                           const std::string &_strName) override {
      //Serial.println("24A");

      tN2kMsg N2kMsg;
      char Name[30];
      strcpy(Name, _strName.c_str());

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

      strcpy(CS, _strCallsign.c_str());
      strcpy(Vendor, _strVendor.c_str());

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
