/***************************************************************************//**
 * @file
 * @brief Core application logic.
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include <stdbool.h>
#include "em_common.h"
#include "em_emu.h"
#include "em_gpio.h"
#include "sl_status.h"
#include "sl_simple_button_instances.h"
#include "sl_app_log.h"
#include "sl_app_assert.h"
#include "sl_bluetooth.h"
#include "sl_simple_timer.h"
#include "gatt_db.h"
#include "advertise.h"
#include "sl_power_supply.h"
#include "board.h"
#include "app.h"
#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif // SL_COMPONENT_CATALOG_PRESENT
#ifdef SL_CATALOG_GATT_SERVICE_AIO_PRESENT
#include "sl_gatt_service_aio.h"
#endif // SL_CATALOG_GATT_SERVICE_AIO_PRESENT
#ifdef SL_CATALOG_GATT_SERVICE_BATTERY_PRESENT
#include "sl_gatt_service_battery.h"
#endif // SL_CATALOG_GATT_SERVICE_BATTERY_PRESENT
#ifdef SL_CATALOG_GATT_SERVICE_HALL_PRESENT
#include "sl_gatt_service_hall.h"
#endif // SL_CATALOG_GATT_SERVICE_HALL_PRESENT
#ifdef SL_CATALOG_SENSOR_HALL_PRESENT
#include "sl_sensor_hall.h"
#endif // SL_CATALOG_SENSOR_HALL_PRESENT
#ifdef SL_CATALOG_GATT_SERVICE_LIGHT_PRESENT
#include "sl_gatt_service_light.h"
#endif // SL_CATALOG_GATT_SERVICE_LIGHT_PRESENT
#ifdef SL_CATALOG_SENSOR_LIGHT_PRESENT
#include "sl_sensor_light.h"
#endif // SL_CATALOG_SENSOR_LIGHT_PRESENT
#ifdef SL_CATALOG_GATT_SERVICE_LUX_PRESENT
#include "sl_gatt_service_lux.h"
#endif // SL_CATALOG_GATT_SERVICE_LUX_PRESENT
#ifdef SL_CATALOG_SENSOR_LUX_PRESENT
#include "sl_sensor_lux.h"
#endif // SL_CATALOG_SENSOR_LUX_PRESENT
#ifdef SL_CATALOG_GATT_SERVICE_RHT_PRESENT
#include "sl_gatt_service_rht.h"
#endif // SL_CATALOG_GATT_SERVICE_RHT_PRESENT
#ifdef SL_CATALOG_SENSOR_RHT_PRESENT
#include "sl_sensor_rht.h"
#endif // SL_CATALOG_SENSOR_RHT_PRESENT
#ifdef SL_CATALOG_GATT_SERVICE_IMU_PRESENT
#include "sl_gatt_service_imu.h"
#endif // SL_CATALOG_GATT_SERVICE_IMU_PRESENT
#ifdef SL_CATALOG_SENSOR_IMU_PRESENT
#include "sl_sensor_imu.h"
#endif // SL_CATALOG_SENSOR_IMU_PRESENT
#ifdef SL_CATALOG_GATT_SERVICE_RGB_PRESENT
#include "sl_gatt_service_rgb.h"
#endif // SL_CATALOG_GATT_SERVICE_RGB_PRESENT
#ifdef SL_CATALOG_GATT_SERVICE_PRESSURE_PRESENT
#include "sl_gatt_service_pressure.h"
#endif // SL_CATALOG_GATT_SERVICE_PRESSURE_PRESENT
#ifdef SL_CATALOG_SENSOR_PRESSURE_PRESENT
#include "sl_sensor_pressure.h"
#endif // SL_CATALOG_SENSOR_PRESSURE_PRESENT
#ifdef SL_CATALOG_GATT_SERVICE_GAS_PRESENT
#include "sl_gatt_service_gas.h"
#endif // SL_CATALOG_GATT_SERVICE_GAS_PRESENT
#ifdef SL_CATALOG_SENSOR_GAS_PRESENT
#include "sl_sensor_gas.h"
#endif // SL_CATALOG_SENSOR_GAS_PRESENT
#ifdef SL_CATALOG_GATT_SERVICE_SOUND_PRESENT
#include "sl_gatt_service_sound.h"
#endif // SL_CATALOG_GATT_SERVICE_SOUND_PRESENT
#ifdef SL_CATALOG_SENSOR_SOUND_PRESENT
#include "sl_sensor_sound.h"
#endif // SL_CATALOG_SENSOR_SOUND_PRESENT

// -----------------------------------------------------------------------------
// Configuration

#define SHUTDOWN_TIMEOUT_MS     60000

// -----------------------------------------------------------------------------
// Private variables

// Timer
static sl_simple_timer_t shutdown_timer;

// -----------------------------------------------------------------------------
// Private function declarations

static void shutdown_start_timer(void);
static void shutdown_stop_timer(void);
static void shutdown(sl_simple_timer_t *timer, void *data);
static void sensor_init(void);
static void sensor_deinit(void);

// -----------------------------------------------------------------------------
// Public function definitions

void app_init(void)
{
  sl_app_log("Thuderboard demo initialised\n");
  sl_power_supply_probe();
  shutdown_start_timer();
#ifdef BOARD_RGBLED_PRESENT
  rgb_led_init();
#endif // BOARD_RGBLED_PRESENT
}

SL_WEAK void app_process_action(void)
{
  /////////////////////////////////////////////////////////////////////////////
  // Put your additional application code here!                              //
  // This is called infinitely.                                              //
  // Do not call blocking functions from here!                               //
  /////////////////////////////////////////////////////////////////////////////
}

// -----------------------------------------------------------------------------
// Bluetooth event handler

void sl_bt_on_event(sl_bt_msg_t *evt)
{
  sl_status_t sc;
  bd_addr address;
  uint8_t address_type;
  uint32_t unique_id;
  uint8_t system_id[8];

  switch (SL_BT_MSG_ID(evt->header)) {
    // -------------------------------
    case sl_bt_evt_system_boot_id:
      // Print boot message.
      sl_app_log("Bluetooth stack booted: v%d.%d.%d-b%d\n",
                 evt->data.evt_system_boot.major,
                 evt->data.evt_system_boot.minor,
                 evt->data.evt_system_boot.patch,
                 evt->data.evt_system_boot.build);
      sc = sl_bt_system_get_identity_address(&address, &address_type);
      sl_app_assert(sc == SL_STATUS_OK,
                    "[E: 0x%04x] Failed to get Bluetooth address\n",
                    (int)sc);
      sl_app_log("Bluetooth %s address: %02X:%02X:%02X:%02X:%02X:%02X\n",
                 address_type ? "static random" : "public device",
                 address.addr[5],
                 address.addr[4],
                 address.addr[3],
                 address.addr[2],
                 address.addr[1],
                 address.addr[0]);
      unique_id = 0xFFFFFF & *((uint32_t*) address.addr);

      // Pad and reverse unique ID to get System ID
      system_id[0] = address.addr[5];
      system_id[1] = address.addr[4];
      system_id[2] = address.addr[3];
      system_id[3] = 0xFF;
      system_id[4] = 0xFE;
      system_id[5] = address.addr[2];
      system_id[6] = address.addr[1];
      system_id[7] = address.addr[0];

      sc = sl_bt_gatt_server_write_attribute_value(gattdb_system_id,
                                                   0,
                                                   sizeof(system_id),
                                                   system_id);
      sl_app_assert(sc == SL_STATUS_OK,
                    "[E: 0x%04x] Failed to get Bluetooth address\n",
                    (int)sc);
      advertise_init(unique_id);
      break;

    // -------------------------------
    case sl_bt_evt_connection_opened_id:
      sl_app_log("Connection opened\n");
      advertise_stop();
      shutdown_stop_timer();
      sensor_init();
      break;

    // -------------------------------
    case sl_bt_evt_connection_closed_id:
      sl_app_log("Connection closed\n");
      advertise_start();
      shutdown_start_timer();
      sensor_deinit();
      break;

    // -------------------------------
    default:
      break;
  }
}

// -----------------------------------------------------------------------------
// Push button event handler

void sl_button_on_change(const sl_button_t *handle)
{
  (void)handle;
#ifdef SL_CATALOG_GATT_SERVICE_AIO_PRESENT
  sl_gatt_service_aio_on_change();
#endif // SL_CATALOG_GATT_SERVICE_AIO_PRESENT
}

// -----------------------------------------------------------------------------
// Shutdown feature function definitions

static void shutdown(sl_simple_timer_t *timer, void *data)
{
  (void)timer;
  (void)data;
  EMU_EM4Init_TypeDef em4_init = EMU_EM4INIT_DEFAULT;
  em4_init.pinRetentionMode = emuPinRetentionEm4Exit;
  EMU_EM4Init(&em4_init);
  // Set up for EM4 wakeup from button 0. Need to enable glitch filter
  sl_simple_button_context_t *button = sl_button_btn0.context;
  GPIO_PinModeSet(button->port, button->pin, gpioModeInputPullFilter, 1);
  GPIO_EM4EnablePinWakeup( (BOARD_BUTTON0_EM4WUEN_MASK << _GPIO_EM4WUEN_EM4WUEN_SHIFT), 0);

  advertise_stop();

  EMU_EnterEM4();
}

static void shutdown_start_timer(void)
{
  sl_status_t sc;
  if (sl_power_supply_is_low_power()) {
    sc = sl_simple_timer_start(&shutdown_timer, SHUTDOWN_TIMEOUT_MS, shutdown, NULL, false);
    sl_app_assert(sc == SL_STATUS_OK,
                  "[E: 0x%04x] Failed to start shutdown timer\n",
                  (int)sc);
  }
}

static void shutdown_stop_timer(void)
{
  sl_status_t sc;
  sc = sl_simple_timer_stop(&shutdown_timer);
  sl_app_assert(sc == SL_STATUS_OK,
                "[E: 0x%04x] Failed to stop shutdown timer\n",
                (int)sc);
}

// -----------------------------------------------------------------------------
// Sensor batch init/deinit

static void sensor_init(void)
{
#ifdef SL_CATALOG_SENSOR_HALL_PRESENT
  sl_sensor_hall_init();
#endif // SL_CATALOG_SENSOR_HALL_PRESENT
#ifdef SL_CATALOG_SENSOR_LIGHT_PRESENT
  sl_sensor_light_init();
#endif // SL_CATALOG_SENSOR_LIGHT_PRESENT
#ifdef SL_CATALOG_SENSOR_LUX_PRESENT
  sl_sensor_lux_init();
#endif // SL_CATALOG_SENSOR_LUX_PRESENT
#ifdef SL_CATALOG_SENSOR_RHT_PRESENT
  sl_sensor_rht_init();
#endif // SL_CATALOG_SENSOR_RHT_PRESENT
#ifdef SL_CATALOG_SENSOR_IMU_PRESENT
  sl_sensor_imu_init();
#endif // SL_CATALOG_SENSOR_IMU_PRESENT
#ifdef SL_CATALOG_SENSOR_PRESSURE_PRESENT
  sl_sensor_pressure_init();
#endif // SL_CATALOG_SENSOR_PRESSURE_PRESENT
#ifdef SL_CATALOG_SENSOR_GAS_PRESENT
  if (!sl_power_supply_is_low_power()) {
    sl_sensor_gas_init();
  }
#endif // SL_CATALOG_SENSOR_GAS_PRESENT
#ifdef SL_CATALOG_SENSOR_SOUND_PRESENT
  sl_sensor_sound_init();
#endif // SL_CATALOG_SENSOR_SOUND_PRESENT
}

static void sensor_deinit(void)
{
#ifdef SL_CATALOG_SENSOR_HALL_PRESENT
  sl_sensor_hall_deinit();
#endif // SL_CATALOG_SENSOR_HALL_PRESENT
#ifdef SL_CATALOG_SENSOR_LIGHT_PRESENT
  sl_sensor_light_deinit();
#endif // SL_CATALOG_SENSOR_LIGHT_PRESENT
#ifdef SL_CATALOG_SENSOR_LUX_PRESENT
  sl_sensor_lux_deinit();
#endif // SL_CATALOG_SENSOR_LUX_PRESENT
#ifdef SL_CATALOG_SENSOR_RHT_PRESENT
  sl_sensor_rht_deinit();
#endif // SL_CATALOG_SENSOR_RHT_PRESENT
#ifdef SL_CATALOG_SENSOR_IMU_PRESENT
  sl_sensor_imu_deinit();
#endif // SL_CATALOG_SENSOR_IMU_PRESENT
#ifdef BOARD_RGBLED_PRESENT
  rgb_led_deinit();
#endif // BOARD_RGBLED_PRESENT
#ifdef SL_CATALOG_SENSOR_PRESSURE_PRESENT
  sl_pressure_deinit();
#endif // SL_CATALOG_SENSOR_PRESSURE_PRESENT
#ifdef SL_CATALOG_SENSOR_GAS_PRESENT
  if (!sl_power_supply_is_low_power()) {
    sl_sensor_gas_deinit();
  }
#endif // SL_CATALOG_SENSOR_GAS_PRESENT
#ifdef SL_CATALOG_SENSOR_SOUND_PRESENT
  sl_sensor_sound_deinit();
#endif // SL_CATALOG_SENSOR_SOUND_PRESENT
}

// -----------------------------------------------------------------------------
// Connect GATT services with sensors by overriding weak functions

#ifdef SL_CATALOG_GATT_SERVICE_BATTERY_PRESENT
uint8_t sl_gatt_service_battery_get_level(void)
{
  return sl_power_supply_get_battery_level();
}

uint8_t sl_gatt_service_battery_get_type(void)
{
  return sl_power_supply_get_type();
}
#endif // SL_CATALOG_GATT_SERVICE_BATTERY_PRESENT

#if defined(SL_CATALOG_GATT_SERVICE_HALL_PRESENT) && defined(SL_CATALOG_SENSOR_HALL_PRESENT)
sl_status_t sl_gatt_service_hall_get(float *field_strength, bool *alert, bool *tamper)
{
  sl_status_t sc;
  sc = sl_sensor_hall_get(field_strength, alert, tamper);
  if (SL_STATUS_OK != sc) {
    sl_app_log("[E: %#04x] Hall sensor measurement failed\n", sc);
  }
  return sc;
}
#endif

#if defined(SL_CATALOG_GATT_SERVICE_LIGHT_PRESENT) && defined(SL_CATALOG_SENSOR_LIGHT_PRESENT)
sl_status_t sl_gatt_service_light_get(float *lux, float *uvi)
{
  sl_status_t sc;
  sc = sl_sensor_light_get(lux, uvi);
  if (SL_STATUS_OK != sc) {
    sl_app_log("[E: %#04x] Light sensor measurement failed\n", sc);
  }
  return sc;
}
#endif

#if defined(SL_CATALOG_GATT_SERVICE_LUX_PRESENT) && defined(SL_CATALOG_SENSOR_LUX_PRESENT)
sl_status_t sl_gatt_service_lux_get(float *lux)
{
  sl_status_t sc;
  sc = sl_sensor_lux_get(lux);
  if (SL_STATUS_OK != sc) {
    sl_app_log("[E: %#04x] Light sensor measurement failed\n", sc);
  }
  return sc;
}
#endif

#if defined(SL_CATALOG_GATT_SERVICE_RHT_PRESENT) && defined(SL_CATALOG_SENSOR_RHT_PRESENT)
sl_status_t sl_gatt_service_rht_get(uint32_t *rh, int32_t *t)
{
  sl_status_t sc;
  sc = sl_sensor_rht_get(rh, t);
  if (SL_STATUS_OK != sc) {
    sl_app_log("[E: %#04x] RHT sensor measurement failed\n", sc);
  }
  return sc;
}
#endif

#if defined(SL_CATALOG_GATT_SERVICE_IMU_PRESENT) && defined(SL_CATALOG_SENSOR_IMU_PRESENT)
sl_status_t sl_gatt_service_imu_get(int16_t ovec[3], int16_t avec[3])
{
  return sl_sensor_imu_get(ovec, avec);
}

sl_status_t sl_gatt_service_imu_calibrate(void)
{
  sl_status_t sc;
  sc = sl_sensor_imu_calibrate();
  sl_app_log("IMU calibration status: %d\n", sc);
  return sc;
}

void sl_gatt_service_imu_enable(bool enable)
{
  sl_app_log("IMU %sable\n", enable ? "en" : "dis");
  sl_sensor_imu_enable(enable);
}
#endif

#if defined(SL_CATALOG_GATT_SERVICE_RGB_PRESENT) && defined(BOARD_RGBLED_PRESENT)
void sl_gatt_service_rgb_set_led(uint8_t m, uint8_t r, uint8_t g, uint8_t b)
{
  if (!sl_power_supply_is_low_power()) {
    rgb_led_set(m, r, g, b);
  }
}
#endif

#if defined(SL_CATALOG_GATT_SERVICE_PRESSURE_PRESENT) && defined(SL_CATALOG_SENSOR_PRESSURE_PRESENT)
sl_status_t sl_gatt_service_pressure_get(float *pressure)
{
  sl_status_t sc;
  sc = sl_pressure_get(pressure);
  if (SL_STATUS_OK != sc) {
    sl_app_log("[E: %#04x] Pressure sensor measurement failed\n", sc);
  }
  return sc;
}
#endif

#if defined(SL_CATALOG_GATT_SERVICE_GAS_PRESENT) && defined(SL_CATALOG_SENSOR_GAS_PRESENT)
sl_status_t sl_gatt_service_gas_get(uint16_t *eco2, uint16_t *tvoc)
{
  sl_status_t sc = SL_STATUS_NOT_AVAILABLE;
  if (!sl_power_supply_is_low_power()) {
    sc = sl_sensor_gas_get(eco2, tvoc);
    if ((SL_STATUS_OK != sc) && (SL_STATUS_NOT_READY != sc)) {
      sl_app_log("[E: %#04x] Air quality sensor measurement failed\n", sc);
    }
  }
  return sc;
}
#endif

#if defined(SL_CATALOG_GATT_SERVICE_SOUND_PRESENT) && defined(SL_CATALOG_SENSOR_SOUND_PRESENT)
sl_status_t sl_gatt_service_sound_get(float *sound_level)
{
  sl_status_t sc;
  sc = sl_sensor_sound_get(sound_level);
  if (SL_STATUS_OK != sc) {
    sl_app_log("[E: %#04x] Sound level measurement failed\n", sc);
  }
  return sc;
}
#endif
