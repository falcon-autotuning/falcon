// =============================================================================
// Fake Multimeter Instrument Plugin for 1D Trace Integration Test
// =============================================================================

#include "plugin-helpers.h"
#include <instrument-data.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CHANNELS 8
#define DEFAULT_TRACE_LEN 201

static int g_initialized = 0;
static char g_instrument_name[PLUGIN_MAX_STRING_LEN] = "Meter1";
static int g_num_bins[MAX_CHANNELS] = {1, 1, 1, 1, 1, 1, 1, 1};
static int g_sample_rate[MAX_CHANNELS] = {1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000};
static double *g_trace_data[MAX_CHANNELS] = {NULL};
static int g_trace_count[MAX_CHANNELS] = {0};
static int g_trace_index[MAX_CHANNELS] = {0};

static int load_trace(int ch, const char *filepath) {
  if (g_trace_data[ch]) {
    free(g_trace_data[ch]);
    g_trace_data[ch] = NULL;
    g_trace_count[ch] = 0;
  }

  FILE *f = fopen(filepath, "r");
  if (!f) {
    // Generate linear 0.0 -> 1.0 trace fallback
    g_trace_count[ch] = DEFAULT_TRACE_LEN;
    g_trace_data[ch] = (double *)malloc(DEFAULT_TRACE_LEN * sizeof(double));
    if (!g_trace_data[ch]) return -1;
    for (int i = 0; i < DEFAULT_TRACE_LEN; i++) {
      g_trace_data[ch][i] = (double)i / (DEFAULT_TRACE_LEN - 1);
    }
    return 0;
  }

  char line[256];
  int count = 0;
  while (fgets(line, sizeof(line), f)) {
    if (line[0] != '#' && line[0] != '\n' && strlen(line) > 1) {
      count++;
    }
  }
  if (count == 0) {
    fclose(f);
    return -1;
  }

  g_trace_data[ch] = (double *)malloc(count * sizeof(double));
  if (!g_trace_data[ch]) {
    fclose(f);
    return -1;
  }

  rewind(f);
  int idx = 0;
  while (fgets(line, sizeof(line), f) && idx < count) {
    if (line[0] != '#' && line[0] != '\n' && strlen(line) > 1) {
      g_trace_data[ch][idx++] = strtod(line, NULL);
    }
  }
  fclose(f);
  g_trace_count[ch] = idx;
  return 0;
}

INSTRUMENT_PLUGIN_API PluginMetadata plugin_get_metadata(void) {
  PluginMetadata meta = {};
  meta.api_version = INSTRUMENT_PLUGIN_API_VERSION;
  strncpy(meta.name, "Mock Multimeter 1D", PLUGIN_MAX_STRING_LEN - 1);
  strncpy(meta.version, "2.0.0", PLUGIN_MAX_STRING_LEN - 1);
  strncpy(meta.protocol_type, "MockMultimeter", PLUGIN_MAX_STRING_LEN - 1);
  strncpy(meta.description, "Mock multimeter delivering 1D trace data", PLUGIN_MAX_STRING_LEN - 1);
  return meta;
}

INSTRUMENT_PLUGIN_API uint8_t plugin_initialize(const PluginConfig *config) {
  if (config && config->instrument_name[0] != '\0') {
    strncpy(g_instrument_name, config->instrument_name, PLUGIN_MAX_STRING_LEN - 1);
  }

  const char *data_file = getenv("MOCK_MULTIMETER_DATA_FILE");
  if (!data_file) {
    data_file = "/opt/instrument-controller/linear-1d.txt";
  }

  for (int ch = 0; ch < MAX_CHANNELS; ch++) {
    load_trace(ch, data_file);
    g_num_bins[ch] = 1;
    g_sample_rate[ch] = 1000;
    g_trace_index[ch] = 0;
  }

  g_initialized = 1;
  return 0;
}

INSTRUMENT_PLUGIN_API uint8_t plugin_execute_command(const PluginCommand *cmd, PluginResponse *resp) {
  if (!g_initialized) {
    return PLUGIN_INITIALIZATION_ERROR;
  }

  if (strcmp(cmd->command, "SET_BINS") == 0) {
    int bins_idx, ch_idx;
    if (get_param_index(cmd, "bins", &bins_idx) != 0 ||
        get_param_index(cmd, "analog", &ch_idx) != 0) {
      return MISSING_PARAMETERS_ERROR;
    }
    int bins = read_int_param(cmd, bins_idx);
    int ch = read_int_param(cmd, ch_idx) - 1;
    if (ch >= 0 && ch < MAX_CHANNELS && bins > 0) {
      g_num_bins[ch] = bins;
      return 0;
    }
    return 1;
  }

  if (strcmp(cmd->command, "SET_SAMPLE_RATE") == 0) {
    int rate_idx, ch_idx;
    if (get_param_index(cmd, "sample_rate", &rate_idx) != 0 ||
        get_param_index(cmd, "analog", &ch_idx) != 0) {
      return MISSING_PARAMETERS_ERROR;
    }
    int rate = read_int_param(cmd, rate_idx);
    int ch = read_int_param(cmd, ch_idx) - 1;
    if (ch >= 0 && ch < MAX_CHANNELS && rate > 0) {
      g_sample_rate[ch] = rate;
      return 0;
    }
    return 1;
  }

  if (strcmp(cmd->command, "MEASURE_STREAM") == 0) {
    int ch_idx;
    if (get_param_index(cmd, "analog", &ch_idx) != 0) {
      return MISSING_PARAMETERS_ERROR;
    }
    int ch = read_int_param(cmd, ch_idx) - 1;
    if (ch < 0 || ch >= MAX_CHANNELS) {
      return 1;
    }

    int bins = g_num_bins[ch] > 0 ? g_num_bins[ch] : 1;
    double *samples = (double *)malloc(bins * sizeof(double));
    if (!samples) {
      return MEMORY_ALLOCATION_ERROR;
    }

    for (int i = 0; i < bins; i++) {
      if (g_trace_count[ch] > 0) {
        if (g_trace_index[ch] >= g_trace_count[ch]) {
          g_trace_index[ch] = 0;
        }
        samples[i] = g_trace_data[ch][g_trace_index[ch]++];
      } else {
        samples[i] = 0.0;
      }
    }

    const char *buffer_id = data_manager_create_buffer(
        g_instrument_name, cmd->id, INST_DATA_FLOAT64, bins, samples);
    free(samples);

    if (!buffer_id) {
      return MEMORY_ALLOCATION_ERROR;
    }

    return push_buffer_response(resp, "stream", buffer_id);
  }

  if (strcmp(cmd->command, "RESET") == 0) {
    for (int ch = 0; ch < MAX_CHANNELS; ch++) {
      g_trace_index[ch] = 0;
    }
    return 0;
  }

  return 0;
}

INSTRUMENT_PLUGIN_API void plugin_shutdown(void) {
  for (int ch = 0; ch < MAX_CHANNELS; ch++) {
    if (g_trace_data[ch]) {
      free(g_trace_data[ch]);
      g_trace_data[ch] = NULL;
    }
    g_trace_count[ch] = 0;
    g_trace_index[ch] = 0;
  }
  g_initialized = 0;
}
