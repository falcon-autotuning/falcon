#include "plugin-helpers.h"
#include <instrument-data.h>
#include <stdio.h>
#include <stdlib.h>

static const int MAX_CHANNEL = 8;
static const int MIN_CHANNEL = 1;
static const int NULL_CHANNEL = -1;
static const int NULL_PARAM_INDEX = -1;
static const int NULL_BINS = 0;
static const int NULL_RATE = 0;
static const float NULL_SLOPE = 0.0;
static const float NULL_TRIGGER_LEVEL = 0.0;
static const char *SET_RATE = "SET_SAMPLE_RATE";
static const char *GET_RATE = "GET_SAMPLE_RATE";
static const char *SET_BINS = "SET_BINS";
static const char *GET_BINS = "GET_BINS";
static const char *GET_DATAPOINT = "GET_DATAPOINT";
static const char *MEASURE_STREAM = "MEASURE_STREAM";
static const char *RESET = "RESET";
static const char *RATE_IO_NAME = "sample_rate";
static const char *BINS_IO_NAME = "bins";
static const char *SLOPE_IO_NAME = "slope";
static const char *ANALOG_IO_NAME = "analog";
static const char *VOLTAGE_IO_NAME = "voltage";
static const char *STREAM_IO_NAME = "stream";
static const int BINS_OUT_OF_RANGE_ERROR = 5;
static const int INVALID_RATE_ERROR = 6;
static const int NO_DATA_ERROR = 7;
static const int CHANNEL_OUT_OF_RANGE_ERROR = 8;
static const int SAMPLE_RATE_OUT_OF_RANGE_ERROR = 9;
static const int UNKNOWN_COMMAND_ERROR = 10;

// Global state for the mock multimeter
static char g_data_file_path[8][PLUGIN_MAX_STRING_LEN] = {{0}};
static int g_num_bins[8] = {0};
static int g_sample_rate[8] = {0};
static double *g_data_buffer[8] = {NULL};
static int g_data_count[8] = {0};
static int g_current_index[8] = {0};
static int g_initialized = 0;
static char g_instrument_name[PLUGIN_MAX_STRING_LEN] = {0};

static int getArrayIndex(int channel) {
  if (channel < MIN_CHANNEL || channel > MAX_CHANNEL) {
    return NULL_CHANNEL;
  }
  return channel - MIN_CHANNEL;
}

static int load_data_from_file(void) {
  const char *env = getenv("MOCK_MULTIMETER_DATA_FILE");
  if (!env) {
    return -1;
  }

  char paths_buf[PLUGIN_MAX_STRING_LEN * MAX_CHANNEL];
  strncpy(paths_buf, env, sizeof(paths_buf) - 1);
  paths_buf[sizeof(paths_buf) - 1] = '\0';

  const char *filepaths[8] = {0};
  int num_paths = 0;
  char *token = strtok(paths_buf, ";");
  while (token && num_paths < MAX_CHANNEL) {
    filepaths[num_paths++] = token;
    token = strtok(NULL, ";");
  }
  if (num_paths == 0) {
    return -1;
  }

  for (int ch = 0; ch < MAX_CHANNEL; ++ch) {
    const char *filepath = filepaths[(ch < num_paths) ? ch : (num_paths - 1)];
    FILE *f = fopen(filepath, "r");
    if (!f) {
      return -1;
    }
    int count = 0;
    char line[256];
    while (fgets(line, sizeof(line), f)) {
      if (line[0] != '#' && line[0] != '\n' && strlen(line) > 1) {
        count++;
      }
    }
    if (count == 0) {
      fclose(f);
      return -1;
    }
    if (g_data_buffer[ch])
      free(g_data_buffer[ch]);
    g_data_buffer[ch] = (double *)malloc(count * sizeof(double));
    if (!g_data_buffer[ch]) {
      fclose(f);
      return -1;
    }
    rewind(f);
    g_data_count[ch] = 0;
    while (fgets(line, sizeof(line), f) && g_data_count[ch] < count) {
      if (line[0] != '#' && line[0] != '\n' && strlen(line) > 1) {
        g_data_buffer[ch][g_data_count[ch]] = strtod(line, NULL);
        g_data_count[ch]++;
      }
    }
    fclose(f);
    strncpy(g_data_file_path[ch], filepath, PLUGIN_MAX_STRING_LEN - 1);
    g_data_file_path[ch][PLUGIN_MAX_STRING_LEN - 1] = '\0';
  }
  return 0;
}

static int handle_set_bins(const PluginCommand *cmd, PluginResponse *resp) {
  (void)resp;
  int bins_idx = 0;
  int channel_idx = 0;
  int err = check_param_count(cmd, 2);
  if (!err) {
    err = get_param_index(cmd, BINS_IO_NAME, &bins_idx);
  }
  if (!err) {
    err = get_param_index(cmd, ANALOG_IO_NAME, &channel_idx);
  }
  if (!err) {
    err = check_param_type(cmd, BINS_IO_NAME, bins_idx, PARAM_TYPE_INT64);
  }
  if (!err) {
    err = check_param_type(cmd, ANALOG_IO_NAME, channel_idx, PARAM_TYPE_INT64);
  }
  if (err) {
    return err;
  }
  int bins = read_int_param(cmd, bins_idx);
  int channel = read_int_param(cmd, channel_idx);
  if (bins <= NULL_BINS) {
    return BINS_OUT_OF_RANGE_ERROR;
  }
  int index = getArrayIndex(channel);
  if (index == NULL_CHANNEL) {
    return CHANNEL_OUT_OF_RANGE_ERROR;
  }
  g_num_bins[index] = bins;
  if (g_num_bins[index] > g_data_count[index]) {
    g_num_bins[index] = g_data_count[index];
  }
  return 0;
}

static int handle_get_bins(const PluginCommand *cmd, PluginResponse *resp) {
  (void)resp;
  int channel_idx = 0;
  int err = check_param_count(cmd, 1);
  if (!err) {
    err = get_param_index(cmd, ANALOG_IO_NAME, &channel_idx);
  }
  if (!err) {
    err = check_param_type(cmd, ANALOG_IO_NAME, channel_idx, PARAM_TYPE_INT64);
  }
  if (err) {
    return err;
  }
  int channel = read_int_param(cmd, channel_idx);
  int index = getArrayIndex(channel);
  if (index == NULL_CHANNEL) {
    return CHANNEL_OUT_OF_RANGE_ERROR;
  }
  return push_int_response(resp, "bins", g_num_bins[index]);
}

static int handle_set_sample_rate(const PluginCommand *cmd,
                                  PluginResponse *resp) {
  (void)resp;
  int rate_idx = 0;
  int channel_idx = 0;
  int err = check_param_count(cmd, 2);
  if (!err) {
    err = get_param_index(cmd, RATE_IO_NAME, &rate_idx);
  }
  if (!err) {
    err = get_param_index(cmd, ANALOG_IO_NAME, &channel_idx);
  }
  if (!err) {
    err = check_param_type(cmd, RATE_IO_NAME, rate_idx, PARAM_TYPE_INT64);
  }
  if (!err) {
    err = check_param_type(cmd, ANALOG_IO_NAME, channel_idx, PARAM_TYPE_INT64);
  }
  if (err) {
    return err;
  }
  int sample_rate = read_int_param(cmd, rate_idx);
  int channel = read_int_param(cmd, channel_idx);
  if (sample_rate <= NULL_RATE) {
    return SAMPLE_RATE_OUT_OF_RANGE_ERROR;
  }
  int index = getArrayIndex(channel);
  if (index == NULL_CHANNEL) {
    return CHANNEL_OUT_OF_RANGE_ERROR;
  }
  g_sample_rate[index] = sample_rate;
  return 0;
}

static int handle_get_sample_rate(const PluginCommand *cmd,
                                  PluginResponse *resp) {
  (void)resp;
  int channel_idx = 0;
  int err = check_param_count(cmd, 1);
  if (!err) {
    err = get_param_index(cmd, ANALOG_IO_NAME, &channel_idx);
  }
  if (!err) {
    err = check_param_type(cmd, ANALOG_IO_NAME, channel_idx, PARAM_TYPE_INT64);
  }
  if (err) {
    return err;
  }
  int channel = read_int_param(cmd, channel_idx);
  int index = getArrayIndex(channel);
  if (index == NULL_CHANNEL) {
    return CHANNEL_OUT_OF_RANGE_ERROR;
  }
  return push_int_response(resp, "sample_rate", g_sample_rate[channel]);
}

static int handle_set_float_setting(const PluginCommand *cmd,
                                    PluginResponse *resp, const char *io_name,
                                    float *values) {
  (void)resp;
  int value_idx = 0;
  int channel_idx = 0;
  int err = check_param_count(cmd, 2);
  if (!err) {
    err = get_param_index(cmd, io_name, &value_idx);
  }
  if (!err) {
    err = get_param_index(cmd, ANALOG_IO_NAME, &channel_idx);
  }
  if (!err) {
    err = check_param_type(cmd, io_name, value_idx, PARAM_TYPE_DOUBLE);
  }
  if (!err) {
    err = check_param_type(cmd, ANALOG_IO_NAME, channel_idx, PARAM_TYPE_INT64);
  }
  if (err) {
    return err;
  }

  int channel = read_int_param(cmd, channel_idx);
  int index = getArrayIndex(channel);
  if (index == NULL_CHANNEL) {
    return CHANNEL_OUT_OF_RANGE_ERROR;
  }
  values[index] = read_float_param(cmd, value_idx);
  return 0;
}

static int handle_get_float_setting(const PluginCommand *cmd,
                                    PluginResponse *resp, const char *io_name,
                                    float *values) {
  int channel_idx = 0;
  int err = check_param_count(cmd, 1);
  if (!err) {
    err = get_param_index(cmd, ANALOG_IO_NAME, &channel_idx);
  }
  if (!err) {
    err = check_param_type(cmd, ANALOG_IO_NAME, channel_idx, PARAM_TYPE_INT64);
  }
  if (err) {
    return err;
  }

  int channel = read_int_param(cmd, channel_idx);
  int index = getArrayIndex(channel);
  if (index == NULL_CHANNEL) {
    return CHANNEL_OUT_OF_RANGE_ERROR;
  }
  return push_double_response(resp, io_name, values[index]);
}

static int handle_datapoint(const PluginCommand *cmd, PluginResponse *resp) {
  int channel_idx = 0;
  int err = check_param_count(cmd, 1);
  if (!err) {
    err = get_param_index(cmd, ANALOG_IO_NAME, &channel_idx);
  }
  if (!err) {
    err = check_param_type(cmd, ANALOG_IO_NAME, channel_idx, PARAM_TYPE_INT64);
  }
  if (err) {
    return err;
  }
  int channel = read_int_param(cmd, channel_idx);
  int index = getArrayIndex(channel);
  if (index == NULL_CHANNEL) {
    return CHANNEL_OUT_OF_RANGE_ERROR;
  }
  if (g_data_count[index] == 0) {
    return NO_DATA_ERROR;
  }

  if (g_current_index[index] >= g_data_count[index]) {
    g_current_index[index] = 0;
  }
  double value = g_data_buffer[index][g_current_index[index]];
  g_current_index[index]++;
  return push_double_response(resp, VOLTAGE_IO_NAME, value);
}

static int handle_stream(const PluginCommand *cmd, PluginResponse *resp) {
  int channel_idx = 0;
  int err = check_param_count(cmd, 1);
  if (!err) {
    err = get_param_index(cmd, ANALOG_IO_NAME, &channel_idx);
  }
  if (!err) {
    err = check_param_type(cmd, ANALOG_IO_NAME, channel_idx, PARAM_TYPE_INT64);
  }
  if (err) {
    return err;
  }
  int channel = read_int_param(cmd, channel_idx);
  int index = getArrayIndex(channel);
  if (index == NULL_CHANNEL) {
    return CHANNEL_OUT_OF_RANGE_ERROR;
  }
  if (g_data_count[index] == 0) {
    return NO_DATA_ERROR;
  }

  double *result = (double *)malloc(g_num_bins[index] * sizeof(double));
  if (!result) {
    return MEMORY_ALLOCATION_ERROR;
  }
  for (int i = 0; i < g_num_bins[index]; i++) {
    if (g_current_index[index] >= g_data_count[index]) {
      g_current_index[index] = 0;
    }
    result[i] = g_data_buffer[index][g_current_index[index]];
    g_current_index[index]++;
  }
  const char *buffer_id = data_manager_create_buffer(
      g_instrument_name, cmd->id, INST_DATA_FLOAT64, g_num_bins[index], result);
  free(result);
  if (!buffer_id) {
    return MEMORY_ALLOCATION_ERROR;
  }
  return push_buffer_response(resp, STREAM_IO_NAME, buffer_id);
}

static int handle_reset(const PluginCommand *cmd) {
  if (check_param_count(cmd, 0)) {
    return MISSING_PARAMETERS_ERROR;
  }
  for (int i = 0; i < MAX_CHANNEL - MIN_CHANNEL + 1; ++i) {
    g_current_index[i] = 0;
    g_sample_rate[i] = 0;
  }
  return 0;
}

INSTRUMENT_PLUGIN_API PluginMetadata plugin_get_metadata(void) {
  PluginMetadata meta = {};
  meta.api_version = INSTRUMENT_PLUGIN_API_VERSION;
  strncpy(meta.name, "Mock Multimeter", PLUGIN_MAX_STRING_LEN - 1);
  strncpy(meta.version, "1.0.0", PLUGIN_MAX_STRING_LEN - 1);
  strncpy(meta.protocol_type, "MockMultimeter", PLUGIN_MAX_STRING_LEN - 1);
  strncpy(meta.description, "Mock multimeter plugin",
          PLUGIN_MAX_STRING_LEN - 1);
  return meta;
}

INSTRUMENT_PLUGIN_API uint8_t plugin_initialize(const PluginConfig *config) {
  if (load_data_from_file() != 0) {
    return PLUGIN_INITIALIZATION_ERROR;
  }
  if (config != NULL) {
    payloadCopy(g_instrument_name, config->instrument_name);
  }
  for (int i = 0; i < MAX_CHANNEL - MIN_CHANNEL + 1; ++i) {
    // Start with one sample per buffer; SET_BINS can enable averaging.
    g_num_bins[i] = 1;
    g_current_index[i] = 0;
    g_sample_rate[i] = 0;
  }
  g_initialized = 1;
  return 0;
}

INSTRUMENT_PLUGIN_API uint8_t plugin_execute_command(const PluginCommand *cmd,
                                                     PluginResponse *resp) {
  if (!g_initialized) {
    return PLUGIN_INITIALIZATION_ERROR;
  }
  if (strcmp(cmd->command, SET_RATE) == 0) {
    return handle_set_sample_rate(cmd, resp);
  }
  if (strcmp(cmd->command, GET_RATE) == 0) {
    return handle_get_sample_rate(cmd, resp);
  }
  if (strcmp(cmd->command, SET_BINS) == 0) {
    return handle_set_bins(cmd, resp);
  }
  if (strcmp(cmd->command, GET_BINS) == 0) {
    return handle_get_bins(cmd, resp);
  }
  if (strcmp(cmd->command, MEASURE_STREAM) == 0) {
    return handle_stream(cmd, resp);
  }
  if (strcmp(cmd->command, GET_DATAPOINT) == 0) {
    return handle_datapoint(cmd, resp);
  }
  if (strcmp(cmd->command, RESET) == 0) {
    return handle_reset(cmd);
  }
  return (uint8_t)UNKNOWN_COMMAND_ERROR;
}

INSTRUMENT_PLUGIN_API void plugin_shutdown(void) {
  for (int i = 0; i < (MAX_CHANNEL - MIN_CHANNEL + 1); ++i) {
    if (g_data_buffer[i]) {
      free(g_data_buffer[i]);
      g_data_buffer[i] = NULL;
    }
    g_num_bins[i] = 0;
    g_sample_rate[i] = 0;
    g_data_count[i] = 0;
    g_current_index[i] = 0;
    g_data_file_path[i][0] = '\0';
  }
  g_instrument_name[0] = '\0';
  g_initialized = 0;
}
