/// @file jbwoprwifi.h
/// @author Jonny Bergdahl
/// @brief Source file for the JBWOPR library.
/// @details Contains declarations for the JBWoprWiFiDevice class.
/// This code is distributed under the MIT License. See LICENSE for details.
/// @date Created: 2023-10-25
/// @copyright Copyright© 2023, Jonny Bergdahl
///
#include "jbwoprwifi.h"
#include "jbwoprhelpers.h"
#include <string>
#include <sstream>
#include <FS.h>
#include <LittleFS.h>
#include <ESPmDNS.h>

// ====================================================================
// General
//
JBWoprWiFiDevice::JBWoprWiFiDevice() :
	JBWoprDevice(),
	_wifiConfig { "", WIFI_NTP_SERVER, "", true }
{
	_wifiConfig.hostName = _getInitialDeviceName();
	_log = new JBLogger("woprwifi", LogLevel::LOG_LEVEL_TRACE);
}

bool JBWoprWiFiDevice::begin(JBWoprBoardVariant variant) {
	return JBWoprDevice::begin(variant);
}

bool JBWoprWiFiDevice::begin(JBWoprBoardVariant variant, JBWoprBoardPins pins) {
	JBWoprDevice::begin(variant, pins);

	_log->trace("Init file system");
	displayShowText("Load config");
	defconLedsSetColor(0xFFFF00);
	if(!LittleFS.begin(true)){
		_log->error("LittleFS Mount Failed");
		return false;
	}
	else {
		// Load configuration
		_loadConfiguration();
	}

	JBTimeHelper::configure(_log, _wifiConfig.ntpServer, _wifiConfig.timeOffsetString);
	_setupWiFiManager();

	displayShowText("Start WiFi");
	defconLedsSetColor(0x00FFFF);
	_log->trace("Start WiFi");
	if (!startWiFi()) {
		// We should never end up here, as the startWiFi() function
		// blocks until restart if AP mode is activated.
		_log->error("Failed to start WiFi");
		return false;
	}

	displayShowText("Setup MDNS");
	defconLedsSetColor(0xFF00FF);
	_log->trace("Setup MDNS");
	if (MDNS.begin(_wifiConfig.hostName.c_str())) {
		MDNS.addService("http", "tcp", 80);
		_log->info("MDNS started: %s.local", _wifiConfig.hostName.c_str());
	}

	if (_wifiConfig.useWebPortal) {
		_log->debug("Start web portal");
		webPortalStart();
	}

	tm info;
	_log->trace("Get time");
	displayShowText("Get time");
	if (!JBTimeHelper::getTime(&info)) {
		_log->error("Failed to get time");
		return false;
	}

	return true;
}

void JBWoprWiFiDevice::loop() {
	JBWoprDevice::loop();

	_wifiManager->process();

	if (_shouldSaveConfig) {
		_shouldSaveConfig = false;
		_saveConfiguration();
		esp_restart();
	}
}

// ====================================================================
// Logger
//
void JBWoprWiFiDevice::setLogLevel(LogLevel level) {
	JBWoprDevice::setLogLevel(level);
	_log->setLogLevel(level);
}

LogLevel JBWoprWiFiDevice::getLogLevel() {
	return _log->getLogLevel();
}

// ====================================================================
// WiFi
//
bool JBWoprWiFiDevice::startWiFi()
{
	if (!_wifiManager->autoConnect(_getDeviceName().c_str(), WEB_PORTAL_PASSWORD)) {
		// If WiFiManager is unable to connect to WiFi, it starts AP mode
		// and we end up here.
		std::string text = "AP " + _apName + ", IP " + "192.168.4.1";
		auto effect = new JBWoprScrollTextDisplayEffect(this, text);
		defconLedsSetColor(0xFF0000);
		effectsStartEffect(effect);
		_log->info("WiFi Manager started in AP Mode: %s", _apName.c_str());
		while (true)
		{
			// We run a separate loop here to allow effects to run while in
			// AP mode.
			if (_shouldSaveConfig) {
				_shouldSaveConfig = false;
				_saveConfiguration();
				_wifiManager->reboot();
			}
			loop();
			delay(100);
		}
		return false;
	}
	defconLedsSetColor(0x00FF00);
	_log->debug("Connected to WiFi");
	return true;
}

void JBWoprWiFiDevice::webPortalStart()
{
	std::vector<const char *> menu = {"param", "sep", "info", "update", "erase", "sep", "restart"};
	_wifiManager->setMenu(menu);
	_wifiManager->startWebPortal();
}

void JBWoprWiFiDevice::webPortalStop()
{
	_log->trace("Stopping web portal");
	_wifiManager->stopWebPortal();
}

// ====================================================================
// Configuration
//
void JBWoprWiFiDevice::_loadConfiguration()
{
	_log->trace("Load configuration");
	if (_wifiConfig.hostName.empty()) {
		_wifiConfig.hostName = _getInitialDeviceName();
	}

	File settingsFile = LittleFS.open(CONFIG_FILE_NAME, "r");
	if (!settingsFile) {
		_log->warning("Failed to open configuration file for reading!");
		return;
	}

	JsonDocument jsonDoc;
	DeserializationError error = deserializeJson(jsonDoc, settingsFile);
	settingsFile.close();
	if (error) {
		_log->error("Error parsing settings JSON file!");
		return;
	}

	if (jsonDoc.isNull()) {
		_log->error("Settings JSON file is empty!");
		return;
	}
	_setConfigFromJsonDocument(jsonDoc);
	_dumpConfig();
}

void JBWoprWiFiDevice::_saveConfiguration()
{
	_log->trace("Saving configuration");
	// Create a JSON document to hold the settings
	JsonDocument jsonDoc;  // Adjust the size according to your needs

	// Set the values in the JSON document
	_setJsonDocumentFromConfig(jsonDoc);
	_dumpConfig();
	File settingsFile = LittleFS.open(CONFIG_FILE_NAME, "w");
	if (!settingsFile) {
		_log->error("Failed to open configuration file for writing!");
		return;
	}
	serializeJson(jsonDoc, settingsFile);
	settingsFile.close();
}

void JBWoprWiFiDevice::_setConfigFromJsonDocument(const JsonDocument &jsonDoc) {
	_log->trace("JBWoprWiFiDevice: Setting configuration from JSON document");
	if (!jsonDoc[JSON_KEY_TIME_FORMAT].isNull()) {
		_config.timeFormat = jsonDoc[JSON_KEY_TIME_FORMAT].as<std::string>();
	}
	if (!jsonDoc[JSON_KEY_DATE_FORMAT].isNull()) {
		_config.dateFormat = jsonDoc[JSON_KEY_DATE_FORMAT].as<std::string>();
	}
	if (!jsonDoc[JSON_KEY_DEFCON_BRIGHTNESS].isNull()) {
		_config.defconLedsBrightness = jsonDoc[JSON_KEY_DEFCON_BRIGHTNESS].as<uint8_t>();
	}
	if (!jsonDoc[JSON_KEY_DISPLAY_BRIGHTNESS].isNull()) {
		_config.displayBrightness = jsonDoc[JSON_KEY_DISPLAY_BRIGHTNESS].as<uint8_t>();
	}
	if (!jsonDoc[JSON_KEY_EFFECTS_TIMEOUT].isNull()) {
		_config.effectsTimeout = jsonDoc[JSON_KEY_EFFECTS_TIMEOUT].as<uint8_t>();
	}
	if (!jsonDoc[JSON_KEY_WIFI_HOST_NAME].isNull()) {
		_wifiConfig.hostName = jsonDoc[JSON_KEY_WIFI_HOST_NAME].as<std::string>();
	}
	if (!jsonDoc[JSON_KEY_WIFI_NTP_SERVER].isNull()) {
		_wifiConfig.ntpServer = jsonDoc[JSON_KEY_WIFI_NTP_SERVER].as<std::string>();
	}
	if (!jsonDoc[JSON_KEY_WIFI_TIME_OFFSET].isNull()) {
		_wifiConfig.timeOffsetString = jsonDoc[JSON_KEY_WIFI_TIME_OFFSET].as<std::string>();
	}
	if (!jsonDoc[JSON_KEY_WIFI_USE_WEB_PORTAL].isNull()) {
		_wifiConfig.useWebPortal = jsonDoc[JSON_KEY_WIFI_USE_WEB_PORTAL].as<bool>();
	}
}

void JBWoprWiFiDevice::_setJsonDocumentFromConfig(JsonDocument &jsonDoc) {
	jsonDoc[JSON_KEY_TIME_FORMAT] = _config.timeFormat;
	jsonDoc[JSON_KEY_DATE_FORMAT] = _config.dateFormat;
	jsonDoc[JSON_KEY_DEFCON_BRIGHTNESS] = _config.defconLedsBrightness;
	jsonDoc[JSON_KEY_DISPLAY_BRIGHTNESS] = _config.displayBrightness;
	jsonDoc[JSON_KEY_EFFECTS_TIMEOUT] = _config.effectsTimeout;
	jsonDoc[JSON_KEY_WIFI_HOST_NAME] = _wifiConfig.hostName;
	jsonDoc[JSON_KEY_WIFI_NTP_SERVER] = _wifiConfig.ntpServer;
	jsonDoc[JSON_KEY_WIFI_TIME_OFFSET] = _wifiConfig.timeOffsetString;
	jsonDoc[JSON_KEY_WIFI_USE_WEB_PORTAL] = _wifiConfig.useWebPortal;
}

void JBWoprWiFiDevice::_dumpConfig() {
	_log->trace("Current configuration");
	_log->trace("  Time format: %s", _config.timeFormat.c_str());
	_log->trace("  Date format: %s", _config.dateFormat.c_str());
	_log->trace("  DEFCON LEDs brightness: %u", _config.defconLedsBrightness);
	_log->trace("  Display brightness: %u", _config.displayBrightness);
	_log->trace("  Effects timeout: %u", _config.effectsTimeout);
	_log->trace("  Host name: %s", _wifiConfig.hostName.c_str());
	_log->trace("  NTP server: %s", _wifiConfig.ntpServer.c_str());
	_log->trace("  Time offset string: %s", _wifiConfig.timeOffsetString.c_str());
	_log->trace("  Use web portal: %s", _wifiConfig.useWebPortal ? "true" : "false");
}

// ====================================================================
// WiFi
//
std::string JBWoprWiFiDevice::_getDeviceName() {
	return _wifiConfig.hostName;
}

std::string JBWoprWiFiDevice::_getInitialDeviceName() {
	std::stringstream sstream;
	sstream << "wopr-" << std::hex << (uint32_t)ESP.getEfuseMac();
	return sstream.str();
}

WiFiManager* JBWoprWiFiDevice::_getWiFiManager() {
	return _wifiManager;
}

void JBWoprWiFiDevice::_setupWiFiManager() {
	_log->debug("Setup WiFiManager");
	_wifiManager = new WiFiManager();
	_wifiManager->setConfigPortalBlocking(false);
	_wifiManager->setHostname(_wifiConfig.hostName.c_str());

	snprintf(_defconLedsBrightnessValue, sizeof(_defconLedsBrightnessValue), "%u", _config.defconLedsBrightness);
	snprintf(_displayBrightnessValue, sizeof(_displayBrightnessValue), "%u", _config.displayBrightness);
	snprintf(_effectsTimeoutValue, sizeof(_effectsTimeoutValue), "%u", _config.effectsTimeout);

	_woprTitleParam = new WiFiManagerParameter(HTML_WOPR_TITLE);
	_networkTitleParam = new WiFiManagerParameter(HTML_NETWORK_TITLE);
	_breakParam = new WiFiManagerParameter("<br/>");

	_timeFormatParam = new WiFiManagerParameter(JSON_KEY_TIME_FORMAT, "Time format", _config.timeFormat.c_str(), 10);
	_dateFormatParam = new WiFiManagerParameter(JSON_KEY_DATE_FORMAT, "Date format", _config.dateFormat.c_str(), 10);
	_displayBrightnessParam = new WiFiManagerParameter(JSON_KEY_DISPLAY_BRIGHTNESS, "Display brightness (%)", _displayBrightnessValue, 3);
	_defconLedsBrightnessParam = new WiFiManagerParameter(JSON_KEY_DEFCON_BRIGHTNESS, "DEFCON LEDs brightness (%)", _defconLedsBrightnessValue, 3);
	_effectsTimeoutParam = new WiFiManagerParameter(JSON_KEY_EFFECTS_TIMEOUT, "Effects timeout (s)", _effectsTimeoutValue, 3);

	_hostNameParam = new WiFiManagerParameter(JSON_KEY_WIFI_HOST_NAME, "Host name", _wifiConfig.hostName.c_str(), 40);
	_ntpServerNameParam = new WiFiManagerParameter(JSON_KEY_WIFI_NTP_SERVER, "NTP server", _wifiConfig.ntpServer.c_str(), 40);
	_timeOffsetParam = new WiFiManagerParameter(JSON_KEY_WIFI_TIME_OFFSET, "Time offset (leave empty for auto)", _wifiConfig.timeOffsetString.c_str(), 5);
	_useWebPortalParam = new WiFiManagerParameter(JSON_KEY_WIFI_USE_WEB_PORTAL, "Use web portal", "T", 2, _wifiConfig.useWebPortal ? HTML_CHECKBOX_TRUE : HTML_CHECKBOX_FALSE, WFM_LABEL_AFTER);

	_wifiManager->addParameter(_woprTitleParam);
	_wifiManager->addParameter(_timeFormatParam);
	_wifiManager->addParameter(_dateFormatParam);
	_wifiManager->addParameter(_displayBrightnessParam);
	_wifiManager->addParameter(_defconLedsBrightnessParam);
	_wifiManager->addParameter(_effectsTimeoutParam);
	_wifiManager->addParameter(_networkTitleParam);
	_wifiManager->addParameter(_hostNameParam);
	_wifiManager->addParameter(_ntpServerNameParam);
	_wifiManager->addParameter(_timeOffsetParam);
	_wifiManager->addParameter(_useWebPortalParam);

	_wifiManager->setAPCallback(std::bind(&JBWoprWiFiDevice::_apCallback, this, std::placeholders::_1));
	_wifiManager->setWebServerCallback(std::bind(&JBWoprWiFiDevice::_webServerCallback, this));
	_wifiManager->setSaveParamsCallback(std::bind(&JBWoprWiFiDevice::_saveParamsCallback, this));
}

void JBWoprWiFiDevice::_saveParamsCallback () {
	_log->trace("JBWoprWiFiDevice Callback: Save params");
	_config.timeFormat = _timeFormatParam->getValue();
	_config.dateFormat = _dateFormatParam->getValue();
	_config.displayBrightness = atoi(_displayBrightnessParam->getValue());
	_config.defconLedsBrightness = atoi(_defconLedsBrightnessParam->getValue());
	_config.effectsTimeout = atoi(_effectsTimeoutParam->getValue());
	_wifiConfig.hostName = _hostNameParam->getValue();
	_wifiConfig.ntpServer = _ntpServerNameParam->getValue();
	_wifiConfig.timeOffsetString = _timeOffsetParam->getValue();
	_wifiConfig.useWebPortal = strncmp(_useWebPortalParam->getValue(), "T", 1) == 0;
	_shouldSaveConfig = true;
}

void JBWoprWiFiDevice::_apCallback(WiFiManager* wifiManager)
{
	_apName = wifiManager->getConfigPortalSSID().c_str();
	_log->trace("Callback: Access point started, %s", _apName.c_str());
}

void JBWoprWiFiDevice::_webServerCallback() {
	_log->trace("Callback: Webserver started");
}

