#include "WiFiConfig.h"

/// @brief Connects to a saved WiFi network, or configures WiFi
/// @param WiFiManager Pointer to the WifManager object to use
/// @param SSID SSID of setup WiFi network
/// @param Password Password of setup WiFi network
WiFiConfig::WiFiConfig (AsyncWiFiManager* WiFiManager, String SSID, String Password) {
	wifiManager = WiFiManager;
	ssid = SSID;
	password = Password;
}

/// @brief Callback notifying that the access point has started
/// @param myWiFiManager the AsyncWiFiManager making the call
void WiFiConfig::configModeCallback(AsyncWiFiManager *myWiFiManager)
{
	EventBroadcaster::broadcastEvent(EventBroadcaster::Events::WifiConfig);
	Logger.println("Access point started");
}

/// @brief Callback notifying that new settings were saved and connection successful
/// @param myWiFiManager the AsyncWiFiManager making the call
void WiFiConfig::configModeEndCallback(AsyncWiFiManager *myWiFiManager)
{
	Logger.println("Connection successful");
	Logger.print("IP address: ");
	Logger.println(WiFi.localIP());
}

/// @brief Attempts to connect to WiFi network
void WiFiConfig::connectWiFi() {    
	wifiManager->setAPCallback(std::bind(&WiFiConfig::configModeCallback, this, wifiManager));
	wifiManager->setSaveConfigCallback(std::bind(&WiFiConfig::configModeEndCallback, this, wifiManager));
	// Fetches ssid and password and tries to connect
	// if it does not connect it starts an access point with the specified name
	// and goes into a blocking loop awaiting configuration
	if (!wifiManager->autoConnect(ssid.c_str(), password.c_str(), 1, 10)) {
		Logger.println("Failed to connect, we should reset and see if it connects");
		delay(3000);
		ESP.restart();
	}
}