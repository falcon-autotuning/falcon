#include "falcon-core/communications/messages/SettingResponse.hpp"
#include <falcon-core/CerealRegistry.hpp>
#include "falcon-core/instrument_interfaces/names/InstrumentPort.hpp"
#include "falcon-core/math/Quantity.hpp"
#include "falcon-core/generic/Map.hpp"
#include <falcon-typing/FFIHelpers.hpp>
#include <stdexcept>

using namespace falcon::typing;
using namespace falcon::typing::ffi::wrapper;

using SettingResponse   = falcon_core::communications::messages::SettingResponse;
using SettingResponseSP = std::shared_ptr<SettingResponse>;
using InstrumentPort    = falcon_core::instrument_interfaces::names::InstrumentPort;
using InstrumentPortSP  = std::shared_ptr<InstrumentPort>;
using Quantity          = falcon_core::math::Quantity;
using QuantitySP        = std::shared_ptr<Quantity>;

static void pack_sr(SettingResponseSP resp, FalconResultSlot *out, int32_t *oc) {
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "SettingResponse";
  out[0].value.opaque.ptr       = new SettingResponseSP(std::move(resp));
  out[0].value.opaque.deleter   = [](void *p) {
    delete static_cast<SettingResponseSP *>(p);
  };
  *oc = 1;
}

static std::shared_ptr<StructInstance>
get_map_param(const FalconParamEntry *params, int32_t count, const char *key) {
  for (int32_t i = 0; i < count; ++i) {
    if (std::strcmp(params[i].key, key) != 0)
      continue;
    const FalconParamEntry &e = params[i];
    if (e.tag != FALCON_TYPE_OPAQUE)
      throw std::runtime_error(std::string("get_map_param: parameter '") + key +
                               "' is not OPAQUE");
    auto sv = *static_cast<std::shared_ptr<void> *>(e.value.opaque.ptr);
    return std::static_pointer_cast<StructInstance>(sv);
  }
  throw std::runtime_error(std::string("get_map_param: parameter '") + key +
                           "' not found");
}

static std::shared_ptr<ArrayValue>
get_struct_field_array(const std::shared_ptr<StructInstance> &inst,
                       const std::string &field_name) {
  if (!inst || !inst->fields)
    throw std::runtime_error("get_struct_field_array: null struct instance or fields");
  auto it = inst->fields->find(field_name);
  if (it == inst->fields->end())
    throw std::runtime_error("get_struct_field_array: field '" + field_name + "' not found");

  const RuntimeValue &val = it->second;
  if (!std::holds_alternative<std::shared_ptr<StructInstance>>(val))
    throw std::runtime_error("get_struct_field_array: field '" + field_name + "' is not a StructInstance");

  auto arr_inst = std::get<std::shared_ptr<StructInstance>>(val);
  if (!arr_inst || !arr_inst->native_handle.has_value())
    throw std::runtime_error("get_struct_field_array: field '" + field_name + "' has no native_handle");

  return std::static_pointer_cast<ArrayValue>(arr_inst->native_handle.value());
}

extern "C" {

void SampleJSON(const FalconParamEntry *, int32_t,
                FalconResultSlot *out, int32_t *oc) {
  auto getters = std::make_shared<falcon_core::generic::Map<InstrumentPort, Quantity>>();
  auto port = InstrumentPort::Setting(
      "test_port", "instr",
      falcon_core::instrument_interfaces::names::Scope::Local,
      falcon_core::instrument_interfaces::names::Access::ReadWrite,
      falcon_core::instrument_interfaces::names::InstrumentCharacteristic::None);
  auto qty = std::make_shared<Quantity>(2.5, falcon_core::physics::units::SymbolUnit::Volt());
  getters->insert(port, qty);
  SettingResponse resp("test_resp", getters);
  pack_results(FunctionResult{resp.to_json_string()}, out, 16, oc);
}

// New(message: string, getters: Map<InstrumentPort, Quantity>) -> (SettingResponse response)
void STRUCTSettingResponseNew(const FalconParamEntry *params, int32_t param_count,
                              FalconResultSlot *out, int32_t *oc) {
  auto pm      = unpack_params(params, param_count);
  auto message = std::get<std::string>(pm.at("message"));

  auto map_inst = get_map_param(params, param_count, "getters");
  auto keys_arr = get_struct_field_array(map_inst, "keys_");
  auto vals_arr = get_struct_field_array(map_inst, "values_");

  if (keys_arr->elements.size() != vals_arr->elements.size()) {
    throw std::runtime_error("STRUCTSettingResponseNew: Map keys_ and values_ have different sizes");
  }

  auto getters = std::make_shared<falcon_core::generic::Map<InstrumentPort, Quantity>>();
  for (size_t i = 0; i < keys_arr->elements.size(); ++i) {
    const RuntimeValue &kv = keys_arr->elements[i];
    if (!std::holds_alternative<std::shared_ptr<StructInstance>>(kv)) {
      throw std::runtime_error("STRUCTSettingResponseNew: Map key is not a StructInstance");
    }
    auto k_inst = std::get<std::shared_ptr<StructInstance>>(kv);
    if (!k_inst || !k_inst->native_handle.has_value()) {
      throw std::runtime_error("STRUCTSettingResponseNew: Map key StructInstance has no native_handle");
    }
    auto port = std::static_pointer_cast<InstrumentPort>(k_inst->native_handle.value());

    const RuntimeValue &vv = vals_arr->elements[i];
    if (!std::holds_alternative<std::shared_ptr<StructInstance>>(vv)) {
      throw std::runtime_error("STRUCTSettingResponseNew: Map value is not a StructInstance");
    }
    auto v_inst = std::get<std::shared_ptr<StructInstance>>(vv);
    if (!v_inst || !v_inst->native_handle.has_value()) {
      throw std::runtime_error("STRUCTSettingResponseNew: Map value StructInstance has no native_handle");
    }
    auto qty = std::static_pointer_cast<Quantity>(v_inst->native_handle.value());

    getters->insert(port, qty);
  }

  auto resp = std::make_shared<SettingResponse>(message, getters);
  pack_sr(std::move(resp), out, oc);
}

// Getters(this: SettingResponse) -> (Map<InstrumentPort, Quantity> getters)
void STRUCTSettingResponseGetters(const FalconParamEntry *params,
                                  int32_t param_count,
                                  FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<SettingResponse>(params, param_count, "this");
  auto getters = self->getters();

  auto keys_array = std::make_shared<ArrayValue>("InstrumentPort");
  keys_array->elements.reserve(getters->size());

  auto values_array = std::make_shared<ArrayValue>("Quantity");
  values_array->elements.reserve(getters->size());

  for (const auto &entry : *getters->items()) {
    auto key_inst = std::make_shared<StructInstance>("InstrumentPort");
    key_inst->native_handle = std::static_pointer_cast<void>(entry->first());
    keys_array->elements.push_back(key_inst);

    auto value_inst = std::make_shared<StructInstance>("Quantity");
    value_inst->native_handle = std::static_pointer_cast<void>(entry->second());
    values_array->elements.push_back(value_inst);
  }

  auto keys_field = std::make_shared<StructInstance>("Array");
  keys_field->native_handle = std::static_pointer_cast<void>(keys_array);

  auto values_field = std::make_shared<StructInstance>("Array");
  values_field->native_handle = std::static_pointer_cast<void>(values_array);

  auto map_inst = std::make_shared<StructInstance>("Map");
  map_inst->fields->insert(std::make_pair("keys_", keys_field));
  map_inst->fields->insert(std::make_pair("values_", values_field));

  FunctionResult result;
  result.push_back(map_inst);
  pack_results(result, out, 1, oc);
}

// Message(this: SettingResponse) -> (string message)
void STRUCTSettingResponseMessage(const FalconParamEntry *params,
                                  int32_t param_count,
                                  FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<SettingResponse>(params, param_count, "this");
  pack_results(FunctionResult{self->message()}, out, 16, oc);
}

// Equal(this: SettingResponse, other: SettingResponse) -> (bool equal)
void STRUCTSettingResponseEqual(const FalconParamEntry *params,
                                int32_t param_count,
                                FalconResultSlot *out, int32_t *oc) {
  auto self  = get_opaque<SettingResponse>(params, param_count, "this");
  auto other = get_opaque<SettingResponse>(params, param_count, "other");
  pack_results(FunctionResult{*self == *other}, out, 16, oc);
}

// NotEqual(this: SettingResponse, other: SettingResponse) -> (bool notequal)
void STRUCTSettingResponseNotEqual(const FalconParamEntry *params,
                                   int32_t param_count,
                                   FalconResultSlot *out, int32_t *oc) {
  auto self  = get_opaque<SettingResponse>(params, param_count, "this");
  auto other = get_opaque<SettingResponse>(params, param_count, "other");
  pack_results(FunctionResult{*self != *other}, out, 16, oc);
}

// ToJSON(this: SettingResponse) -> (string json)
void STRUCTSettingResponseToJSON(const FalconParamEntry *params,
                                 int32_t param_count,
                                 FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<SettingResponse>(params, param_count, "this");
  pack_results(FunctionResult{self->to_json_string()}, out, 16, oc);
}

// FromJSON(json: string) -> (SettingResponse response)
void STRUCTSettingResponseFromJSON(const FalconParamEntry *params,
                                   int32_t param_count,
                                   FalconResultSlot *out, int32_t *oc) {
  auto pm   = unpack_params(params, param_count);
  auto json = std::get<std::string>(pm.at("json"));
  auto resp = SettingResponse::from_json_string<SettingResponse>(json);
  pack_sr(std::move(resp), out, oc);
}

} // extern "C"
