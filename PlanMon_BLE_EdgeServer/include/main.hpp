#ifndef MAIN_HPP
#define MAIN_HPP

//External libraries


//Dependency libraries
#include "advertiseTime.h"
#include "dataStorage.h"
#include "scanClientData.h"
#include "initialDeviceSetup.h"
#include "dataAlerts.h"
#include "dataVisualization.h"

// your declarations (and certain types of definitions) here
#define BUTTON_PIN        4
#define BACKOFF_DELAY     2000 // Initial delay of 2 seconds between retries.
#define CONNECT_TIMEOUT   20   // Maximum time (in seconds) to wait for a WiFi connection during each attempt.
#define MAX_RETRIES       5    // Maximum number of retries

#endif