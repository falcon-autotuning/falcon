#include "falcon-core/instrument_interfaces/names/InstrumentPort.hpp"
#include "falcon-core/instrument_interfaces/names/Ports.hpp"
#include "falcon-core/physics/device_structures/Connection.hpp"
#include "falcon-core/physics/units/SymbolUnit.hpp"
#include <falcon-core/CerealRegistry.hpp>
#include <falcon-typing/FFIHelpers.hpp>
#include <optional>
#include <stdexcept>

using namespace falcon::typing;
using namespace falcon::typing::ffi::wrapper;

using Ports = falcon_core::instrument_interfaces::names::Ports;
using PortsSP = std::shared_ptr<Ports>;
using InstrumentPort =
    falcon_core::instrument_interfaces::names::InstrumentPort;
using InstrumentPortSP = std::shared_ptr<InstrumentPort>;
using Connection = falcon_core::physics::device_structures::Connection;
using ConnectionSP = std::shared_ptr<Connection>;
using SymbolUnit = falcon_core::physics::units::SymbolUnit;
using SymbolUnitSP = std::shared_ptr<SymbolUnit>;

using Scope = falcon_core::instrument_interfaces::names::Scope;
using Access = falcon_core::instrument_interfaces::names::Access;
using Instrument = falcon_core::instrument_interfaces::names::Instrument;
using Characteristic =
    falcon_core::instrument_interfaces::names::InstrumentCharacteristic;

struct ScopeWrapper {
  Scope value;
};

struct AccessWrapper {
  Access value;
};

struct InstrumentWrapper {
  Instrument value;
};

struct CharacteristicWrapper {
  Characteristic value;
};

using ScopeWrapperSP = std::shared_ptr<ScopeWrapper>;
using AccessWrapperSP = std::shared_ptr<AccessWrapper>;
using InstrumentWrapperSP = std::shared_ptr<InstrumentWrapper>;
using CharacteristicWrapperSP = std::shared_ptr<CharacteristicWrapper>;

struct PortQuery {
  std::optional<std::string> default_name;
  ConnectionSP connection;
  SymbolUnitSP unit;
  std::optional<std::string> instrument_name;
  std::optional<Access> access;
  std::optional<Scope> scope;
  std::optional<Instrument> instrument;
  std::optional<Characteristic> characteristic;
};

using PortQuerySP = std::shared_ptr<PortQuery>;
static bool matches_query(const PortQuery &query,
                          const InstrumentPortSP &port) {

  if (query.default_name && port->default_name() != *query.default_name)
    return false;

  if (query.instrument_name &&
      port->instrument_name() != *query.instrument_name)
    return false;

  if (query.instrument && port->instrument_type() != *query.instrument)
    return false;

  if (query.scope && port->scope() != *query.scope)
    return false;

  if (query.access && port->access() != *query.access)
    return false;

  if (query.characteristic && port->characteristic() != *query.characteristic)
    return false;

  if (query.connection && *port->pseudo_name() != *query.connection)
    return false;

  if (query.unit && *port->units() != *query.unit)
    return false;

  return true;
}

// ── helpers
// ───────────────────────────────────────────────────────────────────
static void pack_port_query(PortQuerySP query, FalconResultSlot *out,
                            int32_t *oc) {

  out[0] = {};
  out[0].tag = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "PortQuery";
  out[0].value.opaque.ptr = new PortQuerySP(std::move(query));
  out[0].value.opaque.deleter = [](void *p) {
    delete static_cast<PortQuerySP *>(p);
  };
  *oc = 1;
}
static void pack_opaque_ports(PortsSP ports, FalconResultSlot *out,
                              int32_t *oc) {
  out[0] = {};
  out[0].tag = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Ports";
  out[0].value.opaque.ptr = new PortsSP(std::move(ports));
  out[0].value.opaque.deleter = [](void *p) {
    delete static_cast<PortsSP *>(p);
  };
  *oc = 1;
}

// Wrap an InstrumentPortSP as a StructInstance with native_handle.
static RuntimeValue wrap_port_as_struct(InstrumentPortSP port) {
  auto inst = std::make_shared<StructInstance>("InstrumentPort");
  inst->native_handle = std::static_pointer_cast<void>(port);
  return inst;
}

// Wrap a ConnectionSP the same way.
static RuntimeValue wrap_conn_as_struct(ConnectionSP conn) {
  auto inst = std::make_shared<StructInstance>("Connection");
  inst->native_handle = std::static_pointer_cast<void>(conn);
  return inst;
}

// ---------------------------------------------------------------------------
// pack_array_result
//
// Pack a shared_ptr<ArrayValue> as a result slot using the same convention as
// array-wrapper.cpp: type_name="Array", ptr = new shared_ptr<ArrayValue>*.
//
// The engine's unpack_results will see type_name="Array" (not a known built-in
// tag), hit the else branch, reinterpret ptr as shared_ptr<void>* to obtain
// the shared ownership of the ArrayValue, and store it as a StructInstance
// with native_handle pointing at the ArrayValue — exactly the same
// representation as returned by Array.New() / PushBack etc.
// This means all subsequent Array method calls (Size, GetIndex, …) will work.
// ---------------------------------------------------------------------------
static void pack_array_result(std::shared_ptr<ArrayValue> arr,
                              FalconResultSlot *out, int32_t *oc) {
  out[0] = {};
  out[0].tag = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Array";
  // Store as shared_ptr<void>* — the same layout the engine expects when it
  // later calls *reinterpret_cast<shared_ptr<void>*>(ptr).
  out[0].value.opaque.ptr =
      new std::shared_ptr<void>(std::static_pointer_cast<void>(arr));
  out[0].value.opaque.deleter = [](void *p) {
    delete static_cast<std::shared_ptr<void> *>(p);
  };
  *oc = 1;
}

// ---------------------------------------------------------------------------
// get_array_from_params
//
// Recover the shared_ptr<ArrayValue> from a FalconParamEntry for a parameter
// that holds an Array<T> value.
//
// The engine's pack_params path for a native StructInstance (is_native==true)
// does:
//   ptr = new shared_ptr<void>( val->native_handle.value() )
//   type_name = val->type_name   (= "Array")
//
// So ptr is a heap-allocated shared_ptr<void>*, and .get() points at the
// ArrayValue.  We recover it via static_pointer_cast<ArrayValue>.
// ---------------------------------------------------------------------------
static std::shared_ptr<ArrayValue>
get_array_from_params(const FalconParamEntry *entries, int32_t count,
                      const char *key) {
  for (int32_t i = 0; i < count; ++i) {
    if (std::strcmp(entries[i].key, key) != 0)
      continue;

    const FalconParamEntry &e = entries[i];
    if (e.tag != FALCON_TYPE_OPAQUE) {
      throw std::runtime_error(
          std::string("STRUCTPortsNew: parameter '") + key +
          "' is not OPAQUE (tag=" + std::to_string(e.tag) + ")");
    }

    std::string tn = e.value.opaque.type_name ? e.value.opaque.type_name : "";

    // Normal path: engine packed a native StructInstance ("Array") as
    //   ptr = new shared_ptr<void>* aliasing the ArrayValue
    if (tn == "Array") {
      auto sv = *static_cast<std::shared_ptr<void> *>(e.value.opaque.ptr);
      return std::static_pointer_cast<ArrayValue>(sv);
    }

    // Legacy path: someone passed a bare ArrayValue opaque directly
    if (tn == "ArrayValue") {
      return *static_cast<std::shared_ptr<ArrayValue> *>(e.value.opaque.ptr);
    }

    throw std::runtime_error(std::string("STRUCTPortsNew: parameter '") + key +
                             "' has unexpected opaque type_name='" + tn + "'");
  }
  throw std::runtime_error(std::string("STRUCTPortsNew: parameter '") + key +
                           "' not found");
}

static void pack_instrument_port(InstrumentPortSP ld, FalconResultSlot *out,
                                 int32_t *oc) {
  out[0] = {};
  out[0].tag = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "InstrumentPort";
  out[0].value.opaque.ptr = new InstrumentPortSP(std::move(ld));
  out[0].value.opaque.deleter = [](void *p) {
    delete static_cast<InstrumentPortSP *>(p);
  };
  *oc = 1;
}

extern "C" {

// ── Constructor
// ───────────────────────────────────────────────────────────────
void STRUCTPortQueryNew(const FalconParamEntry *, int32_t,
                        FalconResultSlot *out, int32_t *oc) {
  pack_port_query(std::make_shared<PortQuery>(), out, oc);
}
// New(ports: Array<InstrumentPort>) -> (Ports ports)
void STRUCTPortsNew(const FalconParamEntry *params, int32_t param_count,
                    FalconResultSlot *out, int32_t *oc) {
  auto arr_val = get_array_from_params(params, param_count, "ports");

  std::vector<InstrumentPortSP> vec;
  vec.reserve(arr_val->elements.size());

  for (const auto &elem : arr_val->elements) {
    // Each element was packed by instrumentport-wrapper.cpp as a StructInstance
    // with native_handle holding the InstrumentPortSP.
    // However unpack_results on the PushBack call stored it as a
    // StructInstance directly in the RuntimeValue — so we get a
    // shared_ptr<StructInstance> here, not a shared_ptr<void>.
    if (!std::holds_alternative<std::shared_ptr<StructInstance>>(elem)) {
      throw std::runtime_error("STRUCTPortsNew: array element is not a "
                               "StructInstance (variant index=" +
                               std::to_string(elem.index()) + ")");
    }
    auto inst = std::get<std::shared_ptr<StructInstance>>(elem);
    if (!inst) {
      throw std::runtime_error(
          "STRUCTPortsNew: StructInstance element is null");
    }
    if (!inst->native_handle.has_value()) {
      throw std::runtime_error(
          "STRUCTPortsNew: InstrumentPort StructInstance has no native_handle "
          "(type=" +
          inst->type_name + ")");
    }
    auto port =
        std::static_pointer_cast<InstrumentPort>(inst->native_handle.value());
    if (!port) {
      throw std::runtime_error(
          "STRUCTPortsNew: static_pointer_cast<InstrumentPort> returned null");
    }
    vec.push_back(std::move(port));
  }

  pack_opaque_ports(std::make_shared<Ports>(vec), out, oc);
}

// ── Accessors
// ─────────────────────────────────────────────────────────────────

void STRUCTPortQuerySetDefaultName(const FalconParamEntry *params,
                                   int32_t param_count, FalconResultSlot *out,
                                   int32_t *oc) {

  auto query = get_opaque<PortQuery>(params, param_count, "this");

  auto pm = unpack_params(params, param_count);

  query->default_name = std::get<std::string>(pm.at("defaultName"));

  out[0] = {};
  out[0].tag = FALCON_TYPE_NIL;
  *oc = 1;
}
void STRUCTPortQuerySetConnection(const FalconParamEntry *params,
                                  int32_t param_count, FalconResultSlot *out,
                                  int32_t *oc) {

  auto query = get_opaque<PortQuery>(params, param_count, "this");

  query->connection = get_opaque<Connection>(params, param_count, "conn");

  out[0] = {};
  out[0].tag = FALCON_TYPE_NIL;
  *oc = 1;
}
// SetUnit(this: PortQuery, unit: SymbolUnit) -> ()
void STRUCTPortQuerySetUnit(const FalconParamEntry *params, int32_t param_count,
                            FalconResultSlot *out, int32_t *oc) {

  auto query = get_opaque<PortQuery>(params, param_count, "this");

  query->unit = get_opaque<SymbolUnit>(params, param_count, "unit");

  out[0] = {};
  out[0].tag = FALCON_TYPE_NIL;
  *oc = 1;
}
// SetInstrumentName(this: PortQuery, instrumentName: string) -> ()
void STRUCTPortQuerySetInstrumentName(const FalconParamEntry *params,
                                      int32_t param_count,
                                      FalconResultSlot *out, int32_t *oc) {

  auto query = get_opaque<PortQuery>(params, param_count, "this");

  auto pm = unpack_params(params, param_count);

  query->instrument_name = std::get<std::string>(pm.at("instrumentName"));

  out[0] = {};
  out[0].tag = FALCON_TYPE_NIL;
  *oc = 1;
}
// SetAccess(this: PortQuery, access: Access) -> ()
void STRUCTPortQuerySetAccess(const FalconParamEntry *params,
                              int32_t param_count, FalconResultSlot *out,
                              int32_t *oc) {

  auto query = get_opaque<PortQuery>(params, param_count, "this");

  auto access = get_opaque<AccessWrapper>(params, param_count, "access");

  query->access = access->value;

  out[0] = {};
  out[0].tag = FALCON_TYPE_NIL;
  *oc = 1;
}
// SetScope(this: PortQuery, scope: Scope) -> ()
void STRUCTPortQuerySetScope(const FalconParamEntry *params,
                             int32_t param_count, FalconResultSlot *out,
                             int32_t *oc) {

  auto query = get_opaque<PortQuery>(params, param_count, "this");

  auto scope = get_opaque<ScopeWrapper>(params, param_count, "scope");

  query->scope = scope->value;

  out[0] = {};
  out[0].tag = FALCON_TYPE_NIL;
  *oc = 1;
}
// SetInstrument(this: PortQuery, inst: Instrument) -> ()
void STRUCTPortQuerySetInstrument(const FalconParamEntry *params,
                                  int32_t param_count, FalconResultSlot *out,
                                  int32_t *oc) {

  auto query = get_opaque<PortQuery>(params, param_count, "this");

  auto inst = get_opaque<InstrumentWrapper>(params, param_count, "inst");

  query->instrument = inst->value;

  out[0] = {};
  out[0].tag = FALCON_TYPE_NIL;
  *oc = 1;
}
// SetCharacteristic(this: PortQuery, char: Characteristic) -> ()
void STRUCTPortQuerySetCharacteristic(const FalconParamEntry *params,
                                      int32_t param_count,
                                      FalconResultSlot *out, int32_t *oc) {

  auto query = get_opaque<PortQuery>(params, param_count, "this");

  auto characteristic =
      get_opaque<CharacteristicWrapper>(params, param_count, "char");

  query->characteristic = characteristic->value;

  out[0] = {};
  out[0].tag = FALCON_TYPE_NIL;
  *oc = 1;
}
// Ports(this: Ports) -> (Array<InstrumentPort> ports)
void STRUCTPortsPorts(const FalconParamEntry *params, int32_t param_count,
                      FalconResultSlot *out, int32_t *oc) {
  auto ports_obj = get_opaque<Ports>(params, param_count, "this");
  std::vector<RuntimeValue> elements;
  auto ports = *ports_obj->ports();
  for (const auto &port : ports) {
    elements.push_back(wrap_port_as_struct(port));
  }
  auto arr_val =
      std::make_shared<ArrayValue>("InstrumentPort", std::move(elements));
  pack_array_result(arr_val, out, oc);
}

// GetDefaultNames(this: Ports) -> (Array<string> names)
//
// Keep the ListSP alive as a shared_ptr; never copy List<T> by value.
void STRUCTPortsGetDefaultNames(const FalconParamEntry *params,
                                int32_t param_count, FalconResultSlot *out,
                                int32_t *oc) {
  auto ports_obj = get_opaque<Ports>(params, param_count, "this");
  auto name_list = ports_obj->get_default_names();
  std::vector<RuntimeValue> elements;
  for (const auto &name : *name_list) {
    elements.push_back(name);
  }
  auto arr_val = std::make_shared<ArrayValue>("string", std::move(elements));
  pack_array_result(arr_val, out, oc);
}

// GetPsuedoNames(this: Ports) -> (Array<Connection> connections)
void STRUCTPortsGetPsuedoNames(const FalconParamEntry *params,
                               int32_t param_count, FalconResultSlot *out,
                               int32_t *oc) {
  auto ports_obj = get_opaque<Ports>(params, param_count, "this");
  auto conn_list = ports_obj->get_pseudo_names();
  std::vector<RuntimeValue> elements;
  for (const auto &conn : *conn_list) {
    elements.push_back(wrap_conn_as_struct(conn));
  }
  auto arr_val =
      std::make_shared<ArrayValue>("Connection", std::move(elements));
  pack_array_result(arr_val, out, oc);
}

// IsKnobs(this: Ports) -> (Array<bool> is_knobs)
void STRUCTPortsIsKnobs(const FalconParamEntry *params, int32_t param_count,
                        FalconResultSlot *out, int32_t *oc) {
  auto ports_obj = get_opaque<Ports>(params, param_count, "this");
  pack_results(FunctionResult{ports_obj->is_knobs()}, out, 16, oc);
}

// IsMeters(this: Ports) -> (Array<bool> is_meters)
void STRUCTPortsIsMeters(const FalconParamEntry *params, int32_t param_count,
                         FalconResultSlot *out, int32_t *oc) {
  auto ports_obj = get_opaque<Ports>(params, param_count, "this");
  pack_results(FunctionResult{ports_obj->is_meters()}, out, 16, oc);
}

// IsSettings(this: Ports) -> (bool is_settings)
void STRUCTPortsIsSettings(const FalconParamEntry *params, int32_t param_count,
                           FalconResultSlot *out, int32_t *oc) {
  auto ports_obj = get_opaque<Ports>(params, param_count, "this");
  pack_results(FunctionResult{ports_obj->is_settings()}, out, 16, oc);
}

// ── Equality
// ──────────────────────────────────────────────────────────────────

// Equal(this: Ports, other: Ports) -> (bool equal)
void STRUCTPortsEqual(const FalconParamEntry *params, int32_t param_count,
                      FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Ports>(params, param_count, "this");
  auto other = get_opaque<Ports>(params, param_count, "other");
  pack_results(FunctionResult{*self == *other}, out, 16, oc);
}

// NotEqual(this: Ports, other: Ports) -> (bool notequal)
void STRUCTPortsNotEqual(const FalconParamEntry *params, int32_t param_count,
                         FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Ports>(params, param_count, "this");
  auto other = get_opaque<Ports>(params, param_count, "other");
  pack_results(FunctionResult{*self != *other}, out, 16, oc);
}

// ── JSON
// ──────────────────────────────────────────────────────────────────────

// ToJSON(this: Ports) -> (string json)
void STRUCTPortsToJSON(const FalconParamEntry *params, int32_t param_count,
                       FalconResultSlot *out, int32_t *oc) {
  auto ports_obj = get_opaque<Ports>(params, param_count, "this");
  pack_results(FunctionResult{ports_obj->to_json_string()}, out, 16, oc);
}

// FromJSON(json: string) -> (Ports ports)
void STRUCTPortsFromJSON(const FalconParamEntry *params, int32_t param_count,
                         FalconResultSlot *out, int32_t *oc) {
  auto pm = unpack_params(params, param_count);
  auto json = std::get<std::string>(pm.at("json"));
  auto p = Ports::from_json_string<Ports>(json);
  pack_opaque_ports(std::make_shared<Ports>(*p), out, oc);
}

// ── List accessors
// ────────────────────────────────────────────────────────────

// Size(this) -> (int size)
void STRUCTPortsSize(const FalconParamEntry *params, int32_t param_count,
                     FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Ports>(params, param_count, "this");
  pack_results(FunctionResult{static_cast<int64_t>(self->size())}, out, 16, oc);
}

// IsEmpty(this) -> (bool empty)
void STRUCTPortsIsEmpty(const FalconParamEntry *params, int32_t param_count,
                        FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Ports>(params, param_count, "this");
  pack_results(FunctionResult{self->empty()}, out, 16, oc);
}

// GetIndex(this, index: int) -> (InstrumentPort value)
void STRUCTPortsGetIndex(const FalconParamEntry *params, int32_t param_count,
                         FalconResultSlot *out, int32_t *oc) {
  auto pm = unpack_params(params, param_count);
  auto self = get_opaque<Ports>(params, param_count, "this");
  int64_t idx = std::get<int64_t>(pm.at("index"));
  auto ld = self->at(static_cast<size_t>(idx));
  pack_instrument_port(ld, out, oc);
}

// PushBack(this, value: InstrumentPort) -> ()
void STRUCTPortsPushBack(const FalconParamEntry *params, int32_t param_count,
                         FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Ports>(params, param_count, "this");
  auto ld = get_opaque<InstrumentPort>(params, param_count, "value");
  self->push_back(ld);
  out[0] = {};
  out[0].tag = FALCON_TYPE_NIL;
  *oc = 1;
}

// Insert(this, index: int, value: InstrumentPort) -> ()
void STRUCTPortsInsert(const FalconParamEntry *params, int32_t param_count,
                       FalconResultSlot *out, int32_t *oc) {
  auto pm = unpack_params(params, param_count);
  auto self = get_opaque<Ports>(params, param_count, "this");
  int64_t idx = std::get<int64_t>(pm.at("index"));
  auto ld = get_opaque<InstrumentPort>(params, param_count, "value");
  std::vector<InstrumentPortSP> tmp{ld};
  auto it = self->begin();
  std::advance(it, static_cast<size_t>(idx));
  self->insert(it, tmp.begin(), tmp.end());
  out[0] = {};
  out[0].tag = FALCON_TYPE_NIL;
  *oc = 1;
}

// Contains(this, value: InstrumentPort) -> (bool found)
void STRUCTPortsContains(const FalconParamEntry *params, int32_t param_count,
                         FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Ports>(params, param_count, "this");
  auto ld = get_opaque<InstrumentPort>(params, param_count, "value");
  pack_results(FunctionResult{self->contains(ld)}, out, 16, oc);
}

// Index(this, value: InstrumentPort) -> (int index)
void STRUCTPortsIndex(const FalconParamEntry *params, int32_t param_count,
                      FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Ports>(params, param_count, "this");
  auto ld = get_opaque<InstrumentPort>(params, param_count, "value");
  pack_results(FunctionResult{static_cast<int64_t>(self->index(ld))}, out, 16,
               oc);
}

// Clear(this) -> ()
void STRUCTPortsClear(const FalconParamEntry *params, int32_t param_count,
                      FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Ports>(params, param_count, "this");
  self->clear();
  out[0] = {};
  out[0].tag = FALCON_TYPE_NIL;
  *oc = 1;
}

// Erase(this, index: int) -> ()
void STRUCTPortsErase(const FalconParamEntry *params, int32_t param_count,
                      FalconResultSlot *out, int32_t *oc) {
  auto pm = unpack_params(params, param_count);
  auto self = get_opaque<Ports>(params, param_count, "this");
  int64_t idx = std::get<int64_t>(pm.at("index"));
  self->erase_at(static_cast<size_t>(idx));
  out[0] = {};
  out[0].tag = FALCON_TYPE_NIL;
  *oc = 1;
}

// IsQueryMatch(this: Ports, query: PortQuery) -> (Error err)
void STRUCTPortsIsQueryMatch(const FalconParamEntry *params,
                             int32_t param_count, FalconResultSlot *out,
                             int32_t *oc) {

  try {
    auto ports = get_opaque<Ports>(params, param_count, "this");

    auto query = get_opaque<PortQuery>(params, param_count, "query");

    bool found = false;

    auto raw_ports = *ports->ports();
    for (const auto &port : raw_ports) {
      if (matches_query(*query, port)) {
        found = true;
        break;
      }
    }

    if (found) {
      out[0] = {};
      out[0].tag = FALCON_TYPE_NIL;
      *oc = 1;
      return;
    }

    pack_results(
        FunctionResult{nullptr,
                       ErrorObject{"No InstrumentPort matches query", false}},
        out, 16, oc);

  } catch (const std::exception &e) {

    pack_results(FunctionResult{nullptr, ErrorObject{e.what(), false}}, out, 16,
                 oc);
  }
}

void STRUCTPortsFindQueryMatch(const FalconParamEntry *params,
                               int32_t param_count, FalconResultSlot *out,
                               int32_t *oc) {

  auto ports = get_opaque<Ports>(params, param_count, "this");

  auto query = get_opaque<PortQuery>(params, param_count, "query");
  auto raw_ports = *ports->ports();
  for (const auto &port : raw_ports) {
    if (matches_query(*query, port)) {
      pack_instrument_port(port, out, oc);
      return;
    }
  }

  throw std::runtime_error("No InstrumentPort matched PortQuery");
}

} // extern "C"
