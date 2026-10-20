#include <instrument-plugin.h>
#include <plugin-api.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

static const int PLUGIN_INITIALIZATION_ERROR = 1;
static const int MISSING_PARAMETERS_ERROR = 2;
static const int INVALID_PARAMETER_TYPE_ERROR = 3;
static const int MEMORY_ALLOCATION_ERROR = 4;

static int check_param_count(const PluginCommand *cmd, const int expected) {
  uint8_t count = param_storage_count(cmd->params);
  if (count == expected) {
    return 0;
  }
  return MISSING_PARAMETERS_ERROR;
}

// Copies a message into the target buffer with proper null-termination,
// ensuring it does not exceed the maximum payload size
static void payloadCopy(char *target, const char *message) {
  strncpy(target, message, PLUGIN_MAX_STRING_LEN - 1);
  target[PLUGIN_MAX_STRING_LEN - 1] = '\0';
}
static int setPluginError(PluginResponse *resp, const char *msg,
                          int error_code) {
  (void)resp;
  (void)msg;
  return error_code;
}
static int push_int_response(PluginResponse *resp, const char *name,
                             int64_t value) {
  Variable out = {0};
  out.type = PARAM_TYPE_INT64;
  payloadCopy(out.name, name);
  out.value.i64_val = value;
  return plugin_response_push(resp, &out) == 0 ? 0 : MEMORY_ALLOCATION_ERROR;
}

static int push_double_response(PluginResponse *resp, const char *name,
                                double value) {
  Variable out = {0};
  out.type = PARAM_TYPE_DOUBLE;
  payloadCopy(out.name, name);
  out.value.d_val = value;
  return plugin_response_push(resp, &out) == 0 ? 0 : MEMORY_ALLOCATION_ERROR;
}

static int push_buffer_response(PluginResponse *resp, const char *name,
                                const char *buffer_id) {
  Variable out = {0};
  out.type = PARAM_TYPE_BUFFER;
  payloadCopy(out.name, name);
  payloadCopy(out.value.str_val, buffer_id);
  return plugin_response_push(resp, &out) == 0 ? 0 : MEMORY_ALLOCATION_ERROR;
}
// Finds the index of the parameter with the given name.
// Returns 0 on success and sets *out_index
static int get_param_index(const PluginCommand *cmd, const char *param_name,
                           int *out_index) {
  uint8_t count = param_storage_count(cmd->params);
  for (uint8_t i = 0; i < count; i++) {
    const Variable *param = param_storage_get(cmd->params, i);
    if (param != NULL && strcmp(param->name, param_name) == 0) {
      *out_index = (int)i;
      return 0;
    }
  }
  return MISSING_PARAMETERS_ERROR;
}

// Checks that the parameter at cmd->params[idx] has the expected type.
// Returns 0 if the type matches, or sets an error in resp and returns
// error_code const char *param_name is used for error messages to indicate
// which parameter had the wrong type. idx is the index of the parameter to
// check in cmd->params expected_type is the expected PARAM_TYPE_* value for the
// parameter
static int check_param_type(const PluginCommand *cmd, const char *param_name,
                            const int idx, const int expected_type) {
  const Variable *param = param_storage_get(cmd->params, (uint8_t)idx);
  if (param == NULL) {
    return INVALID_PARAMETER_TYPE_ERROR;
  }
  int actual = param->type;
  if (expected_type == PARAM_TYPE_DOUBLE) {
    if (actual == PARAM_TYPE_DOUBLE) {
      return 0;
    }
  }
  if (expected_type == PARAM_TYPE_INT64) {
    if (actual == PARAM_TYPE_INT64 || actual == PARAM_TYPE_DOUBLE) {
      return 0;
    }
  }
  if (actual != expected_type) {
    return INVALID_PARAMETER_TYPE_ERROR;
  }
  return 0;
}

static int read_int_param(const PluginCommand *cmd, const int idx) {
  const Variable *param = param_storage_get(cmd->params, (uint8_t)idx);
  if (param == NULL)
    return 0;
  if (param->type == PARAM_TYPE_INT64)
    return (int)param->value.i64_val;
  if (param->type == PARAM_TYPE_DOUBLE)
    return (int)param->value.d_val;
  return 0;
}
// Read a floating point param.
static float read_float_param(const PluginCommand *cmd, const int idx) {
  const Variable *param = param_storage_get(cmd->params, (uint8_t)idx);
  if (param == NULL)
    return 0.0f;
  if (param->type == PARAM_TYPE_DOUBLE)
    return (float)param->value.d_val;
  if (param->type == PARAM_TYPE_INT64)
    return (float)param->value.i64_val;
  return 0.0f;
}
