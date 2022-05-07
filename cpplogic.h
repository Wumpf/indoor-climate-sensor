// We could do this stuff in the yaml file. But this is sooo much easier and more compact :)

// Hack to make VSCode autocomplete happy. NOT actually for not using esp32!
#ifndef USE_ESP32
    #include ".esphome/build/indoor-climate-one/src/esphome.h"
#else
    #include "esphome.h"
#endif

// -------------------------------------------------------------------------------------------------
// Interface visible to yaml file!
// -------------------------------------------------------------------------------------------------

void on_loop(const float& target_led_gradientval, esphome::light::AddressableLightState* light, bool silent_mode);
void on_new_co2(float co2, float& target_led_gradient_value, esphome::rtttl::Rtttl* rtttl);
