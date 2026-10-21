#include "falcon-core/communications/messages/SettingRequest.hpp"
#include <falcon-core/CerealRegistry.hpp>
#include "falcon-core/instrument_interfaces/names/Ports.hpp"
#include "falcon-core/instrument_interfaces/names/InstrumentPort.hpp"
#include "falcon-core/math/Quantity.hpp"
#include "falcon-core/generic/Map.hpp"
#include <falcon-typing/FFIHelpers.hpp>
#include <stdexcept>

using namespace falcon::typing;
using namespace falcon::typing::ffi::wrapper;

using SettingRequest   = falcon_core::communications::messages::SettingRequest;
using SettingRequestSP = std::shared_ptr<SettingRequest>;
using Ports            = falcon_core::instrument_interfaces::names::Ports;
using PortsSP          = std::shared_ptr<Ports>;
using InstrumentPort   = falcon_core::instrument_interfaces::names::InstrumentPort;
using InstrumentPortSP = std::shared_ptr<InstrumentPort>;
using Quantity         = falcon_core::math::Quantity;
using QuantitySP       = std::shared_ptr<Quantity>;

static void pack_sr(SettingRequestSP req, FalconResultSlot *out, int32_t *oc) {
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "SettingRequest";
  out[0].value.opaque.ptr       = new SettingRequestSP(std::move(req));
  out[0].value.opaque.deleter   = [](void *p) {
    delete static_cast<SettingRequestSP *>(p);
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
  auto getters = std::make_shared<falcon_core::instrument_interfaces::names::Ports>();
  auto setters = std::make_shared<falcon_core::generic::Map<InstrumentPort, Quantity>>();
  auto port = InstrumentPort::Setting(
      "test_port", "instr",
      falcon_core::instrument_interfaces::names::Scope::Local,
      falcon_core::instrument_interfaces::names::Access::ReadWrite,
      falcon_core::instrument_interfaces::names::InstrumentCharacteristic::None);
  auto qty = std::make_shared<Quantity>(1.5, falcon_core::physics::units::SymbolUnit::Volt());
  setters->insert(port, qty);
  SettingRequest req("test_msg", getters, setters);
  pack_results(FunctionResult{req.to_json_string()}, out, 16, oc);
}

// New(message: string, getters: Ports, setters: Map<InstrumentPort, Quantity>) -> (SettingRequest request)
void STRUCTSettingRequestNew(const FalconParamEntry *params, int32_t param_count,
                             FalconResultSlot *out, int32_t *oc) {
  auto pm      = unpack_params(params, param_count);
  auto message = std::get<std::string>(pm.at("message"));
  auto getters = get_opaque<Ports>(params, param_count, "getters");

  auto map_inst = get_map_param(params, param_count, "setters");
  auto keys_arr = get_struct_field_array(map_inst, "keys_");
  auto vals_arr = get_struct_field_array(map_inst, "values_");

  if (keys_arr->elements.size() != vals_arr->elements.size()) {
    throw std::runtime_error("STRUCTSettingRequestNew: Map keys_ and values_ have different sizes");
  }

  auto setters = std::make_shared<falcon_core::generic::Map<InstrumentPort, Quantity>>();
  for (size_t i = 0; i < keys_arr->elements.size(); ++i) {
    const RuntimeValue &kv = keys_arr->elements[i];
    if (!std::holds_alternative<std::shared_ptr<StructInstance>>(kv)) {
      throw std::runtime_error("STRUCTSettingRequestNew: Map key is not a StructInstance");
    }
    auto k_inst = std::get<std::shared_ptr<StructInstance>>(kv);
    if (!k_inst || !k_inst->native_handle.has_value()) {
      throw std::runtime_error("STRUCTSettingRequestNew: Map key StructInstance has no native_handle");
    }
    auto port = std::static_pointer_cast<InstrumentPort>(k_inst->native_handle.value());

    const RuntimeValue &vv = vals_arr->elements[i];
    if (!std::holds_alternative<std::shared_ptr<StructInstance>>(vv)) {
      throw std::runtime_error("STRUCTSettingRequestNew: Map value is not a StructInstance");
    }
    auto v_inst = std::get<std::shared_ptr<StructInstance>>(vv);
    if (!v_inst || !v_inst->native_handle.has_value()) {
      throw std::runtime_error("STRUCTSettingRequestNew: Map value StructInstance has no native_handle");
    }
    auto qty = std::static_pointer_cast<Quantity>(v_inst->native_handle.value());

    setters->insert(port, qty);
  }

  auto req = std::make_shared<SettingRequest>(message, getters, setters);
  pack_sr(std::move(req), out, oc);
}

// Getters(this: SettingRequest) -> (Ports ports)
void STRUCTSettingRequestGetters(const FalconParamEntry *params,
                                 int32_t param_count,
                                 FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<SettingRequest>(params, param_count, "this");
  auto p    = self->getters();
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Ports";
  out[0].value.opaque.ptr       = new PortsSP(p);
  out[0].value.opaque.deleter   = [](void *ptr) { delete static_cast<PortsSP *>(ptr); };
  *oc = 1;
}

// Setters(this: SettingRequest) -> (Map<InstrumentPort, Quantity> setters)
void STRUCTSettingRequestSetters(const FalconParamEntry *params,
                                 int32_t param_count,
                                 FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<SettingRequest>(params, param_count, "this");
  auto setters = self->setters();

  auto keys_array = std::make_shared<ArrayValue>("InstrumentPort");
  keys_array->elements.reserve(setters->size());

  auto values_array = std::make_shared<ArrayValue>("Quantity");
  values_array->elements.reserve(setters->size());

  for (const auto &entry : *setters->items()) {
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

// Message(this: SettingRequest) -> (string message)
void STRUCTSettingRequestMessage(const FalconParamEntry *params,
                                 int32_t param_count,
                                 FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<SettingRequest>(params, param_count, "this");
  pack_results(FunctionResult{self->message()}, out, 16, oc);
}

// Equal(this: SettingRequest, other: SettingRequest) -> (bool equal)
void STRUCTSettingRequestEqual(const FalconParamEntry *params,
                               int32_t param_count,
                               FalconResultSlot *out, int32_t *oc) {
  auto self  = get_opaque<SettingRequest>(params, param_count, "this");
  auto other = get_opaque<SettingRequest>(params, param_count, "other");
  pack_results(FunctionResult{*self == *other}, out, 16, oc);
}

// NotEqual(this: SettingRequest, other: SettingRequest) -> (bool notequal)
void STRUCTSettingRequestNotEqual(const FalconParamEntry *params,
                                  int32_t param_count,
                                  FalconResultSlot *out, int32_t *oc) {
  auto self  = get_opaque<SettingRequest>(params, param_count, "this");
  auto other = get_opaque<SettingRequest>(params, param_count, "other");
  pack_results(FunctionResult{*self != *other}, out, 16, oc);
}

// ToJSON(this: SettingRequest) -> (string json)
void STRUCTSettingRequestToJSON(const FalconParamEntry *params,
                                int32_t param_count,
                                FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<SettingRequest>(params, param_count, "this");
  pack_results(FunctionResult{self->to_json_string()}, out, 16, oc);
}

// FromJSON(json: string) -> (SettingRequest request)
void STRUCTSettingRequestFromJSON(const FalconParamEntry *params,
                                  int32_t param_count,
                                  FalconResultSlot *out, int32_t *oc) {
  auto pm   = unpack_params(params, param_count);
  auto json = std::get<std::string>(pm.at("json"));
  auto req  = SettingRequest::from_json_string<SettingRequest>(json);
  pack_sr(std::move(req), out, oc);
}

} // extern "C"
