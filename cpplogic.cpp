#include "cpplogic.h"
// #include "esphome/core/log.h"

// using namespace esphome; // Need this to get log macros working
// static const char *const TAG = "cpplogic";

// relevant range for the led, outside we clamp
constexpr float min_expected_co2 = 600.0f;
constexpr float max_expected_co2 = 1100.0f;

struct ColorF { float r, g, b; };
constexpr ColorF color_nodata = { 0.0, 0.0, 1.0 };
constexpr ColorF color_good = { 0.0, 1.0, 0.0 };
constexpr ColorF color_meh =  { 0.8, 0.4, 0.0 };
constexpr ColorF color_bad =  { 1.0, 0.0, 0.0 };

constexpr float led_brightness = 0.2f;

static constexpr float max_color_change_time_in_seconds = 5.0f;
static constexpr float gradient_change_per_ms = 1.0f / (max_color_change_time_in_seconds * 1000.0f);

const std::string recoverysong = "Cantina:d=4, o=5, b=250:8a, 8p, 8d6, 8p, 8a, 8p, 8d6, 8p, 8a, 8d6, 8p, 8a, 8p, 8g#, a, 8a, 8g#, 8a, g, 8f#, 8g, 8f#, f., 8d., 16p, p., 8a, 8p, 8d6, 8p, 8a, 8p, 8d6, 8p, 8a, 8d6, 8p, 8a, 8p, 8g#, 8a, 8p, 8g, 8p, g., 8f#, 8g, 8p, 8c6, a#, a, g";
const std::string alarmsong = "alarm:d=4,o=5,b=100:16e6,16e6,4p,16e6,16e6,4p,16e6,16e6,4p,16e6,16e6";
// RR:d=4,o=5,b=200:8g,8a,8c6,8a,e6,8p,e6,8p,d6.,p,8p,8g,8a,8c6,8a,d6,8p,d6,8p,c6,8b,a.,8g,8a,8c6,8a,2c6,d6,b,a,g.,8p,g,2d6,2c6.,p,8g,8a,8c6,8a,e6,8p,e6,8p,d6.,p,8p,8g,8a,8c6,8a,2g6,b,c6.,8b,a,8g,8a,8c6,8a,2c6,d6,b,a,g.,8p,g,2d6,2c6
// zelda_woods:d=4,o=5,b=140:8f6,8a6,b6,8f6,8a6,b6,8f6,8a6,8b6,8e7,d7,8b6,8c7,8b6,8g6,2e6,8d6,8e6,8g6,2e6,8f6,8a6,b6,8f6,8a6,b6,8f6,8a6,8b6,8e7,d7,8b6,8c7,8e7,8b6,2g6,8b6,8g6,8d6,2e6
// MissionImp:d=16,o=6,b=95:32d,32d#,32d,32d#,32d,32d#,32d,32d#,32d,32d,32d#,32e,32f,32f#,32g,g,8p,g,8p,a#,p,c7,p,g,8p,g,8p,f,p,f#,p,g,8p,g,8p,a#,p,c7,p,g,8p,g,8p,f,p,f#,p,a#,g,2d,32p,a#,g,2c#,32p,a#,g,2c,a#5,8c,2p,32p,a#5,g5,2f#,32p,a#5,g5,2f,32p,a#5,g5,2e,d#,8d
// http://www.fodor.sk/Spectrum/rttl.htm
// https://www.end2endzone.com/how-to-convert-arduino-code-to-actual-rtttl-melodies-using-librtttl-and-anyrtttl/

// -----------------------


ColorF lerp(float completion, ColorF start, ColorF end)
{
    return ColorF {
        start.r + (end.r - start.r) * completion,
        start.g + (end.g - start.g) * completion,
        start.b + (end.b - start.b) * completion
    };
}

static void set_light_color_to_gradient(float gradient, esphome::light::AddressableLightState* light, bool force)
{
    // Only stress the light with new values if we made a significant enough change.
    static float last_set_display_gradient_val = -10.0;
    if (!force && fabs(last_set_display_gradient_val - gradient) < 0.02)
        return;

    ColorF color;
    if (gradient < 0.0)
        color = lerp(gradient + 1.0, color_nodata, color_good);
    else if (gradient < 0.5)
        color = lerp(gradient * 2.0, color_good, color_meh);
    else
        color = lerp(gradient * 2.0 - 1.0, color_meh, color_bad);

    auto call = light->turn_on();
    if (light->current_values.get_brightness() != led_brightness)
        call.set_brightness(led_brightness);
    call.set_rgb(color.r, color.g, color.b);
    call.perform();

    last_set_display_gradient_val = gradient;
}

static float get_gradientval_from_co2(float co2)
{
    return std::min(1.0f, std::max(0.0f, co2 - min_expected_co2) / (max_expected_co2 - min_expected_co2));
}

void on_loop(const float& target_led_gradientval, esphome::light::AddressableLightState* light, bool silent_mode)
{
    static uint32_t millis_last_call = esphome::millis();
    uint32_t millis_now = esphome::millis();

    static float display_gradient_val = -0.9; // Not -1.0 so we set the color in the beginning.

    if (display_gradient_val != target_led_gradientval)
    {
        auto dt = millis_now - millis_last_call;

        float gradient_change = dt * gradient_change_per_ms;
        if (display_gradient_val < 0.0) // Go faster in the negative/"no data" region
            gradient_change *= 2.0;

        if (display_gradient_val > target_led_gradientval)
            display_gradient_val = std::max(target_led_gradientval, display_gradient_val - gradient_change);
        else
            display_gradient_val = std::min(target_led_gradientval, display_gradient_val + gradient_change);

        if (!silent_mode)
            set_light_color_to_gradient(display_gradient_val, light,  display_gradient_val == target_led_gradientval);
    }

    millis_last_call = millis_now;
}

void on_new_co2(float co2, float& target_led_gradientval, esphome::rtttl::Rtttl* rtttl)
{
    target_led_gradientval = get_gradientval_from_co2(co2);

    // Handle buzzer
    static bool enable_recoverysong = false;
    if (co2 > 1100.0f)
        rtttl->play(alarmsong);

    if (co2 > 850.0f)
        enable_recoverysong = true;

    if (enable_recoverysong && co2 < 650.0f)
    {
        rtttl->play(recoverysong);
        enable_recoverysong = false;
    }
}
