#include "platform/safe_output.hpp"
#include "board_profile.hpp"
#include "driver/gpio.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_err.h"
namespace platform {
bool EspBoardProfile::matches() {
    esp_chip_info_t chip{};
    esp_chip_info(&chip);
    uint32_t bytes = 0;
    uint32_t flash_id = 0;
    return chip.model == CHIP_ESP32C3 && BRIDGE_PROFILE_PWM_PIN == 0 && BRIDGE_PROFILE_RC_PIN == 3 &&
           esp_flash_get_physical_size(nullptr, &bytes) == ESP_OK && bytes == 4194304 &&
           esp_flash_read_id(nullptr, &flash_id) == ESP_OK && flash_id == 0x204016;
}
OutputApplyResult EspSafeOutput::apply_low() {
    if (!profile_.matches()) return {false};
    const auto pin = static_cast<gpio_num_t>(BRIDGE_PROFILE_PWM_PIN);
    if (gpio_set_level(pin, 0) != ESP_OK) return {false};
    if (gpio_set_direction(pin, GPIO_MODE_OUTPUT) != ESP_OK) return {false};
    return {true};
}
}
