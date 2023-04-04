#ifndef _LED_CONTROLLER__H_
#define _LED_CONTROLLER__H_

#include <SinricProDevice.h>
#include <Capabilities/ModeController.h>
#include <Capabilities/ColorController.h>

class LED_Controller_ 
: public SinricProDevice
, public ModeController<LED_Controller_>
, public ColorController<LED_Controller_> {
  friend class ModeController<LED_Controller_>;
  friend class ColorController<LED_Controller_>;
public:
  LED_Controller_(const String &deviceId) : SinricProDevice(deviceId, "LED_Controller_") {};
};

#endif