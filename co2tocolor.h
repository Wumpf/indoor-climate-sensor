// We could do this stuff in the yaml file. But this is sooo much easier and more compact :)

// -----------------------

// relevant range for the led, outside we clamp
constexpr float min_expected_co2 = 600;
constexpr float max_expected_co2 = 1100;

float color_good[] = { 0.0, 1.0, 0.0 };
float color_meh[] =  { 1.0, 0.3, 0.0 };
float color_bad[] =  { 1.0, 0.0, 0.0 };

// -----------------------

// Hack to make VSCode autocomplete happy
#ifndef USE_ESP32
    #include ".esphome/build/indoor-climate-one/src/esphome.h"
#endif


static float get_normalized_co2(float co2)
{
    return std::min(1.0f, std::max(0.0f, co2 - min_expected_co2) / (max_expected_co2 - min_expected_co2));
}

template<int component>
static float co2_to_color_component(float co2)
{
    float normalized_co2 = get_normalized_co2(co2);
    float gradient_part1 = lerp(min(1.0f, normalized_co2 * 2.0f), color_good[component], color_meh[component]);
    float result = lerp(max(0.0f, normalized_co2 * 2.0f - 1.0f), gradient_part1, color_bad[component]);
    //esph_log_i("led", "component %i: %f", component, result);
    return result;
}
