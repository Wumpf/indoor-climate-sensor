// We could do this stuff in the yaml file. But this is sooo much easier and more compact :)

// -----------------------

// relevant range for the led, outside we clamp
constexpr float min_expected_co2 = 600;
constexpr float max_expected_co2 = 1100;

struct ColorF { float r, g, b; };
ColorF color_good = { 0.0, 1.0, 0.0 };
ColorF color_meh =  { 0.8, 0.4, 0.0 };
ColorF color_bad =  { 1.0, 0.0, 0.0 };

// -----------------------

// Hack to make VSCode autocomplete happy
#ifndef USE_ESP32
    #include ".esphome/build/indoor-climate-one/src/esphome.h"
#endif

ColorF lerp(float completion, ColorF start, ColorF end)
{
    return ColorF {
        start.r + (end.r - start.r) * completion,
        start.g + (end.g - start.g) * completion,
        start.b + (end.b - start.b) * completion
    };
}

static void set_light_color_to_gradient(float gradient, uint32_t transition_ms, esphome::light::AddressableLightState* light)
{
    auto color = lerp(max(0.0f, gradient * 2.0f - 1.0f), lerp(min(1.0f, gradient * 2.0f), color_good, color_meh), color_bad);

    auto call = light->turn_on();
    call.set_transition_length(transition_ms);
    call.set_brightness(1.0);
    call.set_color_mode(ColorMode::RGB);
    call.set_rgb(color.r, color.g, color.b);
    call.perform();
}

static float get_normalized_co2(float co2)
{
    return std::min(1.0f, std::max(0.0f, co2 - min_expected_co2) / (max_expected_co2 - min_expected_co2));
}

static void set_light_color_to_co2(float co2, light::AddressableLightState* light)
{
    float normalized_co2 = get_normalized_co2(co2);
    set_light_color_to_gradient(normalized_co2, light->get_default_transition_length(), light);
}
