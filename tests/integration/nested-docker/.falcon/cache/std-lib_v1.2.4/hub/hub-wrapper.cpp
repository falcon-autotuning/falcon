#include <falcon-typing/FFIHelpers.hpp>
#include <falcon-routine/hub.hpp>
#include <falcon-core/communications/messages/MeasurementRequest.hpp>
#include <falcon-core/communications/messages/MeasurementResponse.hpp>
#include <falcon-core/communications/messages/SettingRequest.hpp>
#include <falcon-core/communications/messages/SettingResponse.hpp>
#include <falcon-core/communications/messages/VoltageStatesResponse.hpp>
#include <falcon-core/communications/voltage_states/DeviceVoltageStates.hpp>
#include <falcon-core/instrument_interfaces/names/Ports.hpp>
#include <falcon-core/physics/config/core/Config.hpp>
#include <falcon-core/physics/device_structures/Connections.hpp>
#include <falcon-core/math/domains/CoupledLabelledDomain.hpp>
#include <falcon-core/math/Point.hpp>
#include <iostream>
#include <stdexcept>
#include <cstring>
#include <memory>

using namespace falcon::typing;
using namespace falcon::typing::ffi::wrapper;

namespace {

// ── Generic Opaque Extract Helper ───────────────────────────────────────────
template <typename T>
std::shared_ptr<T> extract_opaque_handle(const FalconParamEntry *entries, int32_t count, const char *key, const char *expected_type) {
  for (int32_t i = 0; i < count; ++i) {
    if (std::strcmp(entries[i].key, key) != 0) continue;
    const FalconParamEntry &e = entries[i];
    if (e.tag != FALCON_TYPE_OPAQUE) {
      throw std::runtime_error(std::string("parameter '") + key + "' is not OPAQUE");
    }
    std::string tn = e.value.opaque.type_name ? e.value.opaque.type_name : "";
    if (tn == expected_type) {
      return *static_cast<std::shared_ptr<T> *>(e.value.opaque.ptr);
    }
    auto raw_ptr = *static_cast<std::shared_ptr<void> *>(e.value.opaque.ptr);
    return std::static_pointer_cast<T>(raw_ptr);
  }
  throw std::runtime_error(std::string("parameter '") + key + "' not found");
}

// ── Generic Opaque Pack Helper ──────────────────────────────────────────────
template <typename T>
void pack_opaque(std::shared_ptr<T> obj, FalconResultSlot *slot, const char *type_name) {
  slot->tag = FALCON_TYPE_OPAQUE;
  slot->value.opaque.type_name = type_name;
  slot->value.opaque.ptr = new std::shared_ptr<T>(std::move(obj));
  slot->value.opaque.deleter = [](void *p) {
    delete static_cast<std::shared_ptr<T> *>(p);
  };
}

int32_t extract_int_param(const FalconParamEntry *entries, int32_t count, const char *key) {
  for (int32_t i = 0; i < count; ++i) {
    if (std::strcmp(entries[i].key, key) == 0) {
      if (entries[i].tag == FALCON_TYPE_INT) {
        return static_cast<int32_t>(entries[i].value.int_val);
      }
      throw std::runtime_error(std::string("parameter '") + key + "' is not INT");
    }
  }
  throw std::runtime_error(std::string("parameter '") + key + "' not found");
}

double extract_float_param(const FalconParamEntry *entries, int32_t count, const char *key) {
  for (int32_t i = 0; i < count; ++i) {
    if (std::strcmp(entries[i].key, key) == 0) {
      if (entries[i].tag == FALCON_TYPE_FLOAT) {
        return entries[i].value.float_val;
      }
      if (entries[i].tag == FALCON_TYPE_INT) {
        return static_cast<double>(entries[i].value.int_val);
      }
      throw std::runtime_error(std::string("parameter '") + key + "' is not FLOAT");
    }
  }
  throw std::runtime_error(std::string("parameter '") + key + "' not found");
}

} // namespace

extern "C" {

// ── 1. RequestDeviceState ───────────────────────────────────────────────────
void RequestDeviceState(const FalconParamEntry *param_entries, int32_t param_count,
                        FalconResultSlot *out_slots, int32_t *out_count) {
  int32_t timeout_ms = extract_int_param(param_entries, param_count, "timeout_ms");
  auto resp = falcon::routine::request_device_state(timeout_ms);
  pack_opaque(resp, &out_slots[0], "VoltageStatesResponse");
  *out_count = 1;
}

// ── 2. RequestMeasurement ───────────────────────────────────────────────────
void RequestMeasurement(const FalconParamEntry *param_entries, int32_t param_count,
                        FalconResultSlot *out_slots, int32_t *out_count) {
  auto req = extract_opaque_handle<falcon_core::communications::messages::MeasurementRequest>(
      param_entries, param_count, "req", "MeasurementRequest");
  int32_t timeout_ms = extract_int_param(param_entries, param_count, "timeout_ms");
  auto resp = falcon::routine::request_measurement(req, timeout_ms);
  pack_opaque(resp, &out_slots[0], "MeasurementResponse");
  *out_count = 1;
}

// ── 2b. RequestSetting ──────────────────────────────────────────────────────
void RequestSetting(const FalconParamEntry *param_entries, int32_t param_count,
                    FalconResultSlot *out_slots, int32_t *out_count) {
  auto req = extract_opaque_handle<falcon_core::communications::messages::SettingRequest>(
      param_entries, param_count, "req", "SettingRequest");
  int32_t timeout_ms = extract_int_param(param_entries, param_count, "timeout_ms");
  auto resp = falcon::routine::request_setting(req, timeout_ms);
  pack_opaque(resp, &out_slots[0], "SettingResponse");
  *out_count = 1;
}

// ── 3. RequestConfig ────────────────────────────────────────────────────────
void RequestConfig(const FalconParamEntry *param_entries, int32_t param_count,
                   FalconResultSlot *out_slots, int32_t *out_count) {
  int32_t timeout_ms = extract_int_param(param_entries, param_count, "timeout_ms");
  auto cfg = falcon::routine::request_config(timeout_ms);
  pack_opaque(cfg, &out_slots[0], "Config");
  *out_count = 1;
}

// ── 4. RequestPortPayload ───────────────────────────────────────────────────
void RequestPortPayload(const FalconParamEntry *param_entries, int32_t param_count,
                        FalconResultSlot *out_slots, int32_t *out_count) {
  int32_t timeout_ms = extract_int_param(param_entries, param_count, "timeout_ms");
  auto [knobs_val, meters_val, settings_val] = falcon::routine::request_port_payload(timeout_ms);
  auto knobs = std::make_shared<falcon_core::instrument_interfaces::names::Ports>(std::move(knobs_val));
  auto meters = std::make_shared<falcon_core::instrument_interfaces::names::Ports>(std::move(meters_val));
  auto settings = std::make_shared<falcon_core::instrument_interfaces::names::Ports>(std::move(settings_val));
  pack_opaque(knobs, &out_slots[0], "Ports");
  pack_opaque(meters, &out_slots[1], "Ports");
  pack_opaque(settings, &out_slots[2], "Ports");
  *out_count = 3;
}

// ── 5. CacheDeviceVoltages ──────────────────────────────────────────────────
void CacheDeviceVoltages(const FalconParamEntry *param_entries, int32_t param_count,
                         FalconResultSlot *out_slots, int32_t *out_count) {
  (void)out_slots;
  auto voltages = extract_opaque_handle<falcon_core::communications::voltage_states::DeviceVoltageStates>(
      param_entries, param_count, "voltages", "DeviceVoltageStates");
  falcon::routine::cache_device_voltages(voltages);
  *out_count = 0;
}

// ── 6. ReadDeviceVoltages ───────────────────────────────────────────────────
void ReadDeviceVoltages(const FalconParamEntry *param_entries, int32_t param_count,
                        FalconResultSlot *out_slots, int32_t *out_count) {
  int32_t timeout_ms = extract_int_param(param_entries, param_count, "timeout_ms");
  auto voltages = falcon::routine::read_device_voltages(timeout_ms);
  pack_opaque(voltages, &out_slots[0], "DeviceVoltageStates");
  *out_count = 1;
}

// ── 7. CacheConfig ──────────────────────────────────────────────────────────
void CacheConfig(const FalconParamEntry *param_entries, int32_t param_count,
                 FalconResultSlot *out_slots, int32_t *out_count) {
  (void)out_slots;
  auto cfg = extract_opaque_handle<falcon_core::physics::config::core::Config>(
      param_entries, param_count, "config", "Config");
  falcon::routine::cache_config(cfg);
  *out_count = 0;
}

// ── 8. ReadConfig ───────────────────────────────────────────────────────────
void ReadConfig(const FalconParamEntry *param_entries, int32_t param_count,
                FalconResultSlot *out_slots, int32_t *out_count) {
  int32_t timeout_ms = extract_int_param(param_entries, param_count, "timeout_ms");
  auto cfg = falcon::routine::read_config(timeout_ms);
  pack_opaque(cfg, &out_slots[0], "Config");
  *out_count = 1;
}

// ── 9. GetOhmicsConnectedToVoltageSources ───────────────────────────────────
void GetOhmicsConnectedToVoltageSources(const FalconParamEntry *param_entries, int32_t param_count,
                                        FalconResultSlot *out_slots, int32_t *out_count) {
  int32_t timeout_ms = extract_int_param(param_entries, param_count, "timeout_ms");
  auto conns = falcon::routine::get_ohmics_connected_to_voltage_sources(timeout_ms);
  pack_opaque(conns, &out_slots[0], "Connections");
  *out_count = 1;
}

// ── 10. GetVoltageBounds ────────────────────────────────────────────────────
void GetVoltageBounds(const FalconParamEntry *param_entries, int32_t param_count,
                      FalconResultSlot *out_slots, int32_t *out_count) {
  auto search_domain = extract_opaque_handle<falcon_core::instrument_interfaces::names::Ports>(
      param_entries, param_count, "search_domain", "Ports");
  int32_t timeout_ms = extract_int_param(param_entries, param_count, "timeout_ms");
  auto bounds = falcon::routine::get_voltage_bounds(search_domain, timeout_ms);
  pack_opaque(bounds, &out_slots[0], "CoupledLabelledDomain");
  *out_count = 1;
}

// ── 11. SafeVoltageChange ───────────────────────────────────────────────────
void SafeVoltageChange(const FalconParamEntry *param_entries, int32_t param_count,
                       FalconResultSlot *out_slots, int32_t *out_count) {
  auto proposed_voltages = extract_opaque_handle<falcon_core::math::Point>(
      param_entries, param_count, "proposed_voltages", "Point");
  int32_t timeout_ms = extract_int_param(param_entries, param_count, "timeout_ms");
  bool is_safe = falcon::routine::safe_voltage_change(proposed_voltages, timeout_ms);
  out_slots[0].tag = FALCON_TYPE_BOOL;
  out_slots[0].value.bool_val = is_safe;
  *out_count = 1;
}

// ── 12. Ramp ────────────────────────────────────────────────────────────────
void Ramp(const FalconParamEntry *param_entries, int32_t param_count,
          FalconResultSlot *out_slots, int32_t *out_count) {
  auto end_point = extract_opaque_handle<falcon_core::math::Point>(
      param_entries, param_count, "end_point", "Point");
  double max_ramp_rate = extract_float_param(param_entries, param_count, "max_ramp_rate");
  int32_t timeout_ms = extract_int_param(param_entries, param_count, "timeout_ms");
  bool success = falcon::routine::ramp(end_point, max_ramp_rate, timeout_ms);
  out_slots[0].tag = FALCON_TYPE_BOOL;
  out_slots[0].value.bool_val = success;
  *out_count = 1;
}

// ── 13. HasDatabaseConnection ───────────────────────────────────────────────
void HasDatabaseConnection(const FalconParamEntry *param_entries, int32_t param_count,
                           FalconResultSlot *out_slots, int32_t *out_count) {
  (void)param_entries;
  (void)param_count;
  const char *url = std::getenv("FALCON_DATABASE_URL");
  bool has_db = (url != nullptr && url[0] != '\0');
  out_slots[0].tag = FALCON_TYPE_BOOL;
  out_slots[0].value.bool_val = has_db ? 1 : 0;
  *out_count = 1;
}

} // extern "C"
