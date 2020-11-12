/*
	Copyright (C) 2020 Joe Tinker

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "modules/dev-console.h"
#include "EEPROM.h"
#include "config.h"

namespace {
	const char* sBadCommand = "Bad command! Type ? for help.";
	const char* sInvalidParameter = "Invalid parameter!";
	const char* sSaveToFlash = "The configuration has been saved to the flash.";
	const char* sOK = "OK";

	String inpString;             // buffer for reading line from console
	const byte maxInput = 100;    // max imput length
	// virtual EEPROM address map:
	const int aId = 0;
	const int aKey = 10;
	const int aUrl = 60;
	const int aFiatCurrency = 120;
	const int aDefaultDescription = 150;

	bool EEwriteStr(int addr, byte maxLen, String text){
	    if (text.length() <  maxLen) {
		EEPROM.writeString(addr, text); return true;
	    } else return false;
	}

	// returns the first parameter from cmdStr and removes it from cmdStr at the same time
	String parseCmd(String &cmdStr) {
	  String cmd = "";
	  cmdStr.trim();
	  if (cmdStr.indexOf(' ')>=0) {
	    cmd = cmdStr.substring(0,cmdStr.indexOf(' '));
	    cmdStr = cmdStr.substring(cmdStr.indexOf(' ')+1); 
	    return(cmd); 
	  }
	  else {
	    cmd = cmdStr; 
	    cmdStr = "";
	    return (cmd);
	  }
	}

	// sets the selected parameter
	bool setConf(String param) {
	  String item;
	  item = parseCmd(param);
	  if (devConsole::compare(item, "id", 2))   {
		if (EEwriteStr(aId,10,param)) {config::apiKeyId = std::string(param.c_str()); return(true); };
	  };
	  if (devConsole::compare(item, "key", 2))  {
		if (EEwriteStr(aKey,50,param)) {config::apiKeySecret = std::string(param.c_str()); return(true); };
	  };
	  if (devConsole::compare(item, "url", 2))  {
		if (EEwriteStr(aUrl,60,param)) {config::callbackUrl = std::string(param.c_str()); return(true); };
	  };
	  if (devConsole::compare(item, "fiatCurrency", 2)) {
		if (EEwriteStr(aFiatCurrency,5,param)) {config::fiatCurrency = std::string(param.c_str()); return(true); };
	  };
	  if (devConsole::compare(item, "defaultDescription", 2)) {
		if (EEwriteStr(aDefaultDescription,20,param)) {config::defaultDescription = std::string(param.c_str());return(true); };
	  };
	  Serial.println(sInvalidParameter); return(false);
	}
}

namespace devConsole {

	void init() {
	  inpString.reserve(maxInput);
	  if (!EEPROM.begin(256)) {
	    Serial.println("Failed to initialise EEPROM");
	    Serial.println("Restarting...");
	    delay(2000);
	    ESP.restart();
	  }

	  // init if EEprom if is empty or load configuration
	  if (EEPROM.readByte(0) == 0xFF) {
	    writeDefault(); EEPROM.commit();
	  } else {
	    loadConfig();
	  }
	}

	// reads a chars from the serial port
	bool serialInput() {    // true is EOL
	  char inChar;
	  static bool remToEOL = false;   // remove chracters to the end of line
	  bool isChar = false;
	  while (Serial.available()) {
	      inChar = Serial.read();
	      if (inChar == '\n' || inChar == '\r' ) {Serial.println(); remToEOL = false; return(true); };
	      if (inChar == 9) remToEOL = true;
	      if (inChar == 127) {inpString = inpString.substring(0,inpString.length()-1); Serial.print(inChar); };
	      if ((byte)inChar >= 32 && (byte)inChar < 127) isChar = true;
	      if (isChar && !remToEOL) {inpString += inChar; Serial.print(inChar); };
	  }
	  return(false);
	}

	// compares the match of strings with at least length len
	bool compare(String A, String B, byte len) {
	  if (A.length() < len) return(false);
	  if (A == B.substring(0,A.length())) return true;
	  else return false;
	}

	// evaluates entered command
	void selectCmd() {
	  String maincmd;
	  maincmd = parseCmd(inpString);
	  if (devConsole::compare(maincmd, "set", 2))  {
		if (setConf(inpString)) Serial.println(sOK); inpString=""; return; };
	  inpString="";
	  if (devConsole::compare(maincmd, "show", 2)) {show(); return; };
	  if (maincmd == "?") {showHelp(); return; };
	  if (devConsole::compare(maincmd, "help", 2)) {showHelp(); return; };
	  if (devConsole::compare(maincmd, "save", 2)) {EEPROM.commit(); Serial.println(sSaveToFlash); return; };
	  if (devConsole::compare(maincmd, "reboot", 2)) {ESP.restart(); return; };
	  if (maincmd == "default") {writeDefault(); Serial.println(sOK); return; };
	  Serial.println(sBadCommand); 
	}

	// displays help
	void showHelp() {
	  Serial.println("Supported commands:");
	  Serial.println(" show - displays the current settings");
	  Serial.println(" set [id | key | url | fiatCurency | defaultDescription]");
	  Serial.println(" save - saves current settings to the flash");
	  Serial.println(" default - restores the default settings");
	  Serial.println(" reboot - restart device");
	  Serial.println(" ? | help");
	}

	void show() {
	  Serial.print("Api id: "); Serial.println(config::apiKeyId.c_str());
	  Serial.print("Api key: "); Serial.println(config::apiKeySecret.c_str());
	  Serial.print("Server url: "); Serial.println(config::callbackUrl.c_str());
	  Serial.print("Fiat currency: "); Serial.println(config::fiatCurrency.c_str());
	  Serial.print("Default description: "); Serial.println(config::defaultDescription.c_str());
	}

	// loads the default configuration to the RAM
	void writeDefault(){
	  config::init();
	  EEwriteStr(aId,10,config::apiKeyId.c_str());
	  EEwriteStr(aKey,50,config::apiKeySecret.c_str());
	  EEwriteStr(aUrl,60,config::callbackUrl.c_str());
	  EEwriteStr(aFiatCurrency,5,config::fiatCurrency.c_str());
	  EEwriteStr(aDefaultDescription,20,config::defaultDescription.c_str());
	}

	// loads saved configuration to the RAM
	void loadConfig(){
	  config::apiKeyId     = std::string(EEPROM.readString(aId).c_str());
	  config::apiKeySecret = std::string(EEPROM.readString(aKey).c_str());
	  config::callbackUrl  = std::string(EEPROM.readString(aUrl).c_str());
	  config::fiatCurrency = std::string(EEPROM.readString(aFiatCurrency).c_str());
	  config::defaultDescription = std::string(EEPROM.readString(aDefaultDescription).c_str());
	  logger::write("apiKeyId: " + config::apiKeyId);
	  logger::write("apiKeySecret: " + config::apiKeySecret);
	  logger::write("callbackUrl: " + config::callbackUrl);
	  logger::write("fiatCurrency: " + config::fiatCurrency);
	  logger::write("defaultDescription: " + config::defaultDescription);
	}

}
