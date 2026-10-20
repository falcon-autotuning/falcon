#include "plugin-helpers.h"
#include <instrument-plugin.h>
#include <plugin-api.h>
#include <stdbool.h>
#include <stdint.h>

static const int MAX_CHANNEL = 32;
static const int MIN_CHANNEL = 1;
static const int NULL_CHANNEL = -1;
static const float NULL_VOLTAGE = 0.0;
static const char *SET = "SET_VOLTAGE";
static const char *GET = "GET_VOLTAGE";
static const char *RESET = "RESET";
static const char *VOLTAGE_IO_NAME = "voltage";
static const char *ANALOG_IO_NAME = "analog";

static const int CHANNEL_OUT_OF_RANGE_ERROR = 5;
static const int UNKNOWN_COMMAND_ERROR = 6;

static float g_stored_voltage[32] = {NULL_VOLTAGE};
static int g_initialized = 0;

static int getArrayIndex(int channel) {
  if (channel < MIN_CHANNEL || channel > MAX_CHANNEL) {
    return NULL_CHANNEL;
  }
  return channel - MIN_CHANNEL;
}

static int handle_set(const PluginCommand *cmd, PluginResponse *resp) {
  int voltage_idx, channel_idx;
  int err = check_param_count(cmd, 2);
  if (!err)
    err = get_param_index(cmd, VOLTAGE_IO_NAME, &voltage_idx);
  if (!err)
    err = get_param_index(cmd, ANALOG_IO_NAME, &channel_idx);
  if (!err)
    err =
        check_param_type(cmd, VOLTAGE_IO_NAME, voltage_idx, PARAM_TYPE_DOUBLE);
  if (!err)
    err = check_param_type(cmd, ANALOG_IO_NAME, channel_idx, PARAM_TYPE_INT64);
  if (err)
    return err;
  float voltage = read_float_param(cmd, voltage_idx);
  int channel = read_int_param(cmd, channel_idx);
  int index = getArrayIndex(channel);
  if (index == NULL_CHANNEL) {
    return CHANNEL_OUT_OF_RANGE_ERROR;
  }
  g_stored_voltage[index] = voltage;
  return 0;
}

static int handle_get(const PluginCommand *cmd, PluginResponse *resp) {
  int channel_idx;
  int err = check_param_count(cmd, 1);
  if (!err)
    err = get_param_index(cmd, ANALOG_IO_NAME, &channel_idx);
  if (!err)
    err = check_param_type(cmd, ANALOG_IO_NAME, channel_idx, PARAM_TYPE_INT64);
  if (err)
    return err;
  int channel = read_int_param(cmd, channel_idx);
  int index = getArrayIndex(channel);
  if (index == NULL_CHANNEL) {
    return CHANNEL_OUT_OF_RANGE_ERROR;
  }
  return push_double_response(resp, VOLTAGE_IO_NAME, g_stored_voltage[index]);
}

static int handle_reset(const PluginCommand *cmd, PluginResponse *resp) {
  if (check_param_count(cmd, 0)) {
    return MISSING_PARAMETERS_ERROR;
  }
  for (int i = 0; i < MAX_CHANNEL - MIN_CHANNEL; ++i) {
    g_stored_voltage[i] = NULL_VOLTAGE;
  }
  return 0;
}

INSTRUMENT_PLUGIN_API PluginMetadata plugin_get_metadata(void) {
  PluginMetadata meta = {};
  meta.api_version = INSTRUMENT_PLUGIN_API_VERSION;
  strncpy(meta.name, "Mock Voltage Source", PLUGIN_MAX_STRING_LEN - 1);
  strncpy(meta.version, "1.0.0", PLUGIN_MAX_STRING_LEN - 1);
  strncpy(meta.protocol_type, "MockVoltageSource", PLUGIN_MAX_STRING_LEN - 1);
  strncpy(meta.description, "Mock voltage source plugin",
          PLUGIN_MAX_STRING_LEN - 1);
  return meta;
}

INSTRUMENT_PLUGIN_API uint8_t plugin_initialize(const PluginConfig *config) {
  // Initialize with zero voltage
  for (int i = 0; i < MAX_CHANNEL - MIN_CHANNEL; ++i) {
    g_stored_voltage[i] = NULL_VOLTAGE;
  }
  g_initialized = 1;
  (void)config;
  return 0;
}

INSTRUMENT_PLUGIN_API uint8_t plugin_execute_command(const PluginCommand *cmd,
                                                     PluginResponse *resp) {
  if (!g_initialized) {
    return PLUGIN_INITIALIZATION_ERROR;
  }
  if (strcmp(cmd->command, SET) == 0) {
    return (uint8_t)handle_set(cmd, resp);
  } else if (strcmp(cmd->command, GET) == 0) {
    return (uint8_t)handle_get(cmd, resp);
  } else if (strcmp(cmd->command, RESET) == 0) {
    return (uint8_t)handle_reset(cmd, resp);
  } else {
    return (uint8_t)UNKNOWN_COMMAND_ERROR;
  }
}

INSTRUMENT_PLUGIN_API void plugin_shutdown(void) { g_initialized = 0; }
