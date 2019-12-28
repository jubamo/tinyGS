/*
  ConfigManager.cpp - Config Manager class
  
  Copyright (C) 2020 @G4lile0, @gmag12 y @dev_4m1g0

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

#include "ConfigManager.h"

ConfigManager::ConfigManager()
: IotWebConf2(thingName, &dnsServer, &server, initialApPassword, configVersion)
, server(80)
{
  server.on(ROOT_URL, [this]{ handleRoot(); });
  server.on(CONFIG_URL, [this]{ handleConfig(); });
  server.on(DASHBOARD_URL, [this]{ handleDashboard(); });
  setStatusPin(0);
  setConfigPin(LED_BUILTIN);
  setupUpdateServer(&httpUpdater);
  setHtmlFormatProvider(&gsConfigHtmlFormatProvider);
  formValidatorStd = std::bind(&ConfigManager::formValidator, this);
  setFormValidator(formValidatorStd);
  skipApStartup();
  

  // Customize own parameters
  getThingNameParameter()->label = "GroundStation Name";
  getApPasswordParameter()->label = "GroundStation password";

  addParameter(&latitudeParam);
  addParameter(&longitudeParam);
  addParameter(&tzParam);
  addParameter(&mqttSeparator);
  addParameter(&mqttServerParam);
  addParameter(&mqttPortParam);
  addParameter(&mqttUserParam);
  addParameter(&mqttPassParam);
  addParameter(&separatorBoard);
  addParameter(&boardParam);
}


void ConfigManager::handleRoot()
{
  // -- Let IotWebConf test and handle captive portal requests.
  if (handleCaptivePortal())
  {
    // -- Captive portal request were already served.
    return;
  }
  String s = String(FPSTR(IOTWEBCONF_HTML_HEAD));
  s += "<style>" + String(FPSTR(IOTWEBCONF_HTML_STYLE_INNER)) + "</style>";
  s += FPSTR(IOTWEBCONF_HTML_HEAD_END);
  s += FPSTR(IOTWEBCONF_HTML_BODY_INNER);
  s += String(FPSTR(LOGO)) + "<br />";
  s += "<button onclick=\"window.location.href='" + String(DASHBOARD_URL) + "';\">Station dashboard</button><br /><br />";
  s += "<button onclick=\"window.location.href='" + String(CONFIG_URL) + "';\">Configure parameters</button><br /><br />";
  s += "<button onclick=\"window.location.href='" + String(UPDATE_URL) + "';\">Upload new version</button><br /><br />";
  s += FPSTR(IOTWEBCONF_HTML_END);

  s.replace("{v}", FPSTR(TITLE_TEXT));

  server.sendHeader("Content-Length", String(s.length()));
  server.send(200, "text/html; charset=UTF-8", s);
}

void ConfigManager::handleDashboard()
{
  String s = String(FPSTR(IOTWEBCONF_HTML_HEAD));
  s += "<style>" + String(FPSTR(IOTWEBCONF_HTML_STYLE_INNER)) + "</style>";
  s += FPSTR(IOTWEBCONF_HTML_HEAD_END);
  s += FPSTR(IOTWEBCONF_HTML_BODY_INNER);
  s += String(FPSTR(LOGO)) + "<br />";
  s += "We are still working on this feature. It will be ready soon.<br /><br/>";
  s += "<button onclick=\"window.location.href='" + String(ROOT_URL) + "';\">Go Back</button><br /><br />";
  s += FPSTR(IOTWEBCONF_HTML_END);

  s.replace("{v}", FPSTR(TITLE_TEXT));

  server.sendHeader("Content-Length", String(s.length()));
  server.send(200, "text/html; charset=UTF-8", s);
}


bool ConfigManager::formValidator()
{
  Serial.println("Validating form.");
  boolean valid = true;

  int l = 4;//server.arg(stringParam.getId()).length();
  if (l < 3)
  {
    //stringParam.errorMessage = "Please provide at least 3 characters for this test!";
    valid = false;
  }

  return valid;
}

void ConfigManager::resetAPConfig() {
  getWifiSsidParameter()->valueBuffer[0] = '\0';
  getWifiPasswordParameter()->valueBuffer[0] = '\0';
  strncpy(getApPasswordParameter()->valueBuffer, initialApPassword, IOTWEBCONF_WORD_LEN);
  strncpy(getThingNameParameter()->valueBuffer, thingName, IOTWEBCONF_WORD_LEN);
  strncpy(getThingName(), thingName, IOTWEBCONF_WORD_LEN);
  configSave();
}

void ConfigManager::resetAllConfig(){
  getWifiSsidParameter()->valueBuffer[0] = '\0';
  getWifiPasswordParameter()->valueBuffer[0] = '\0';
  strncpy(getApPasswordParameter()->valueBuffer, initialApPassword, IOTWEBCONF_WORD_LEN);
  strncpy(getThingNameParameter()->valueBuffer, thingName, IOTWEBCONF_WORD_LEN);
  strncpy(getThingName(), thingName, IOTWEBCONF_WORD_LEN);
  strncpy(mqttPortParam.valueBuffer, MQTT_DEFAULT_PORT, MQTT_PORT_LENGTH);
  strncpy(mqttServerParam.valueBuffer, MQTT_DEFAULT_SERVER, MQTT_SERVER_LENGTH);
  mqttUserParam.valueBuffer[0] = '\0';
  mqttPassParam.valueBuffer[0] = '\0';

  configSave();
}

boolean ConfigManager::init() {
  boolean validConfig = IotWebConf2::init();

  // when wifi credentials are set but we are not able to connect (maybe wrong credentials)
  // we fall back to AP mode during 2 minutes after which we try to connect again and repeat.
  setApTimeoutMs(atoi(AP_TIMEOUT_MS));

  //resetAllConfig();

  return validConfig;
}