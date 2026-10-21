#include "falcon-core/physics/device_structures/Connection.hpp"
#include "falcon-core/physics/device_structures/Impedance.hpp"
#include "falcon-core/physics/device_structures/Impedances.hpp"
#include <falcon-core/CerealRegistry.hpp>
#include <falcon-typing/FFIHelpers.hpp>
#include <stdexcept>
#include <vector>

using namespace falcon::typing;
using namespace falcon::typing::ffi::wrapper;

using Impedance = falcon_core::physics::device_structures::Impedance;
using ImpedanceSP = std::shared_ptr<Impedance>;
using Impedances = falcon_core::physics::device_structures::Impedances;
using ImpedancesSP = std::shared_ptr<Impedances>;
using Connection = falcon_core::physics::device_structures::Connection;
using ConnectionSP = std::shared_ptr<Connection>;

// ── helpers ─────────────────────────────────────────────────────────────────

static void pack_opaque_impedance(ImpedanceSP imp, FalconResultSlot *out,
                                  int32_t *oc) {
  out[0] = {};
  out[0].tag = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Impedance";
  out[0].value.opaque.ptr = new ImpedanceSP(std::move(imp));
  out[0].value.opaque.deleter = [](void *p) {
    delete static_cast<ImpedanceSP *>(p);
  };
  *oc = 1;
}

static void pack_opaque_impedances(ImpedancesSP imps, FalconResultSlot *out,
                                   int32_t *oc) {
  out[0] = {};
  out[0].tag = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Impedances";
  out[0].value.opaque.ptr = new ImpedancesSP(std::move(imps));
  out[0].value.opaque.deleter = [](void *p) {
    delete static_cast<ImpedancesSP *>(p);
  };
  *oc = 1;
}

static void pack_opaque_connection(ConnectionSP conn, FalconResultSlot *out,
                                   int32_t *oc) {
  out[0] = {};
  out[0].tag = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Connection";
  out[0].value.opaque.ptr = new ConnectionSP(std::move(conn));
  out[0].value.opaque.deleter = [](void *p) {
    delete static_cast<ConnectionSP *>(p);
  };
  *oc = 1;
}

static RuntimeValue wrap_imp_as_struct(ImpedanceSP imp) {
  auto inst = std::make_shared<StructInstance>("Impedance");
  inst->native_handle = std::static_pointer_cast<void>(imp);
  return inst;
}

static void pack_array_result(std::shared_ptr<ArrayValue> arr,
                              FalconResultSlot *out, int32_t *oc) {
  out[0] = {};
  out[0].tag = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Array";
  out[0].value.opaque.ptr =
      new std::shared_ptr<void>(std::static_pointer_cast<void>(arr));
  out[0].value.opaque.deleter = [](void *p) {
    delete static_cast<std::shared_ptr<void> *>(p);
  };
  *oc = 1;
}

static void pack_nil(FalconResultSlot *out, int32_t *oc) {
  out[0] = {};
  out[0].tag = FALCON_TYPE_NIL;
  *oc = 1;
}

static std::shared_ptr<ArrayValue>
get_array_from_params(const FalconParamEntry *entries, int32_t count,
                      const char *key) {
  for (int32_t i = 0; i < count; ++i) {
    if (std::strcmp(entries[i].key, key) != 0) {
      continue;
    }
    const FalconParamEntry &e = entries[i];
    if (e.tag != FALCON_TYPE_OPAQUE) {
      throw std::runtime_error(std::string("Parameter '") + key +
                               "' is not OPAQUE");
    }
    std::string tn = e.value.opaque.type_name ? e.value.opaque.type_name : "";
    if (tn == "Array") {
      auto sv = *static_cast<std::shared_ptr<void> *>(e.value.opaque.ptr);
      return std::static_pointer_cast<ArrayValue>(sv);
    }
    if (tn == "ArrayValue") {
      return *static_cast<std::shared_ptr<ArrayValue> *>(e.value.opaque.ptr);
    }
    throw std::runtime_error(std::string("Parameter '") + key +
                             "' has unexpected type_name '" + tn + "'");
  }
  throw std::runtime_error(std::string("Parameter '") + key + "' not found");
}

extern "C" {

// ── Impedance ───────────────────────────────────────────────────────────────

void STRUCTImpedanceNew(const FalconParamEntry *params, int32_t param_count,
                        FalconResultSlot *out, int32_t *oc) {
  auto conn = get_opaque<Connection>(params, param_count, "conn");
  auto pm = unpack_params(params, param_count);
  double r = std::get<double>(pm.at("resistance"));
  double c = std::get<double>(pm.at("capacitance"));
  pack_opaque_impedance(std::make_shared<Impedance>(conn, r, c), out, oc);
}

void STRUCTImpedanceGetConnection(const FalconParamEntry *params,
                                  int32_t param_count, FalconResultSlot *out,
                                  int32_t *oc) {
  auto self = get_opaque<Impedance>(params, param_count, "this");
  pack_opaque_connection(self->connection(), out, oc);
}

void STRUCTImpedanceResistance(const FalconParamEntry *params,
                               int32_t param_count, FalconResultSlot *out,
                               int32_t *oc) {
  auto self = get_opaque<Impedance>(params, param_count, "this");
  pack_results(FunctionResult{self->resistance()}, out, 16, oc);
}

void STRUCTImpedanceCapacitance(const FalconParamEntry *params,
                                int32_t param_count, FalconResultSlot *out,
                                int32_t *oc) {
  auto self = get_opaque<Impedance>(params, param_count, "this");
  pack_results(FunctionResult{self->capacitance()}, out, 16, oc);
}

void STRUCTImpedanceEqual(const FalconParamEntry *params, int32_t param_count,
                          FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Impedance>(params, param_count, "this");
  auto other = get_opaque<Impedance>(params, param_count, "other");
  pack_results(FunctionResult{*self == *other}, out, 16, oc);
}

void STRUCTImpedanceNotEqual(const FalconParamEntry *params,
                             int32_t param_count, FalconResultSlot *out,
                             int32_t *oc) {
  auto self = get_opaque<Impedance>(params, param_count, "this");
  auto other = get_opaque<Impedance>(params, param_count, "other");
  pack_results(FunctionResult{*self != *other}, out, 16, oc);
}

void STRUCTImpedanceToJSON(const FalconParamEntry *params, int32_t param_count,
                           FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Impedance>(params, param_count, "this");
  pack_results(FunctionResult{self->to_json_string()}, out, 16, oc);
}

void STRUCTImpedanceFromJSON(const FalconParamEntry *params,
                             int32_t param_count, FalconResultSlot *out,
                             int32_t *oc) {
  auto pm = unpack_params(params, param_count);
  auto json = std::get<std::string>(pm.at("json"));
  auto imp = Impedance::from_json_string<Impedance>(json);
  pack_opaque_impedance(std::make_shared<Impedance>(*imp), out, oc);
}

// ── Impedances ──────────────────────────────────────────────────────────────

void STRUCTImpedancesNew(const FalconParamEntry *params, int32_t param_count,
                         FalconResultSlot *out, int32_t *oc) {
  auto arr_val = get_array_from_params(params, param_count, "imps");
  std::vector<ImpedanceSP> vec;
  vec.reserve(arr_val->elements.size());
  for (const auto &elem : arr_val->elements) {
    if (!std::holds_alternative<std::shared_ptr<StructInstance>>(elem)) {
      throw std::runtime_error("STRUCTImpedancesNew: element is not StructInstance");
    }
    auto inst = std::get<std::shared_ptr<StructInstance>>(elem);
    if (!inst || !inst->native_handle.has_value()) {
      throw std::runtime_error("STRUCTImpedancesNew: StructInstance has no native_handle");
    }
    vec.push_back(std::static_pointer_cast<Impedance>(inst->native_handle.value()));
  }
  pack_opaque_impedances(std::make_shared<Impedances>(vec), out, oc);
}

void STRUCTImpedancesSize(const FalconParamEntry *params, int32_t param_count,
                          FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Impedances>(params, param_count, "this");
  pack_results(FunctionResult{static_cast<int64_t>(self->size())}, out, 16, oc);
}

void STRUCTImpedancesIsEmpty(const FalconParamEntry *params,
                             int32_t param_count, FalconResultSlot *out,
                             int32_t *oc) {
  auto self = get_opaque<Impedances>(params, param_count, "this");
  pack_results(FunctionResult{self->empty()}, out, 16, oc);
}

void STRUCTImpedancesGetIndex(const FalconParamEntry *params,
                              int32_t param_count, FalconResultSlot *out,
                              int32_t *oc) {
  auto self = get_opaque<Impedances>(params, param_count, "this");
  auto pm = unpack_params(params, param_count);
  int64_t idx = std::get<int64_t>(pm.at("index"));
  pack_opaque_impedance((*self)[idx], out, oc);
}

void STRUCTImpedancesPushBack(const FalconParamEntry *params,
                              int32_t param_count, FalconResultSlot *out,
                              int32_t *oc) {
  auto self = get_opaque<Impedances>(params, param_count, "this");
  auto val = get_opaque<Impedance>(params, param_count, "value");
  self->push_back(val);
  pack_nil(out, oc);
}

void STRUCTImpedancesInsert(const FalconParamEntry *params, int32_t param_count,
                            FalconResultSlot *out, int32_t *oc) {
  auto pm = unpack_params(params, param_count);
  auto self = get_opaque<Impedances>(params, param_count, "this");
  int64_t idx = std::get<int64_t>(pm.at("index"));
  auto val = get_opaque<Impedance>(params, param_count, "value");
  std::vector<ImpedanceSP> tmp{val};
  auto it = self->begin();
  std::advance(it, static_cast<size_t>(idx));
  self->insert(it, tmp.begin(), tmp.end());
  pack_nil(out, oc);
}

void STRUCTImpedancesContains(const FalconParamEntry *params,
                              int32_t param_count, FalconResultSlot *out,
                              int32_t *oc) {
  auto self = get_opaque<Impedances>(params, param_count, "this");
  auto val = get_opaque<Impedance>(params, param_count, "value");
  pack_results(FunctionResult{self->contains(val)}, out, 16, oc);
}

void STRUCTImpedancesIndex(const FalconParamEntry *params, int32_t param_count,
                           FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Impedances>(params, param_count, "this");
  auto val = get_opaque<Impedance>(params, param_count, "value");
  pack_results(FunctionResult{static_cast<int64_t>(self->index(val))}, out, 16, oc);
}

void STRUCTImpedancesItems(const FalconParamEntry *params, int32_t param_count,
                           FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Impedances>(params, param_count, "this");
  auto arr = std::make_shared<ArrayValue>();
  arr->elements.reserve(self->size());
  for (const auto &item : *self) {
    arr->elements.push_back(wrap_imp_as_struct(item));
  }
  pack_array_result(arr, out, oc);
}

void STRUCTImpedancesClear(const FalconParamEntry *params, int32_t param_count,
                           FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Impedances>(params, param_count, "this");
  self->clear();
  pack_nil(out, oc);
}

void STRUCTImpedancesErase(const FalconParamEntry *params, int32_t param_count,
                           FalconResultSlot *out, int32_t *oc) {
  auto pm = unpack_params(params, param_count);
  auto self = get_opaque<Impedances>(params, param_count, "this");
  int64_t idx = std::get<int64_t>(pm.at("index"));
  self->erase_at(static_cast<size_t>(idx));
  pack_nil(out, oc);
}

void STRUCTImpedancesEqual(const FalconParamEntry *params, int32_t param_count,
                           FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Impedances>(params, param_count, "this");
  auto other = get_opaque<Impedances>(params, param_count, "other");
  pack_results(FunctionResult{*self == *other}, out, 16, oc);
}

void STRUCTImpedancesNotEqual(const FalconParamEntry *params,
                              int32_t param_count, FalconResultSlot *out,
                              int32_t *oc) {
  auto self = get_opaque<Impedances>(params, param_count, "this");
  auto other = get_opaque<Impedances>(params, param_count, "other");
  pack_results(FunctionResult{*self != *other}, out, 16, oc);
}

void STRUCTImpedancesToJSON(const FalconParamEntry *params, int32_t param_count,
                            FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Impedances>(params, param_count, "this");
  pack_results(FunctionResult{self->to_json_string()}, out, 16, oc);
}

void STRUCTImpedancesFromJSON(const FalconParamEntry *params,
                              int32_t param_count, FalconResultSlot *out,
                              int32_t *oc) {
  auto pm = unpack_params(params, param_count);
  auto json = std::get<std::string>(pm.at("json"));
  auto imps = Impedances::from_json_string<Impedances>(json);
  pack_opaque_impedances(std::make_shared<Impedances>(*imps), out, oc);
}

} // extern "C"
