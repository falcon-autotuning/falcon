#include "falcon-core/instrument_interfaces/names/InstrumentPort.hpp"
#include "falcon-core/physics/device_structures/Connection.hpp"
#include "falcon-core/physics/units/SymbolUnit.hpp"
#include <falcon-core/CerealRegistry.hpp>
#include <falcon-typing/FFIHelpers.hpp>

using namespace falcon::typing;
using namespace falcon::typing::ffi::wrapper;

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
using ScopeWrapperSP = std::shared_ptr<ScopeWrapper>;

struct AccessWrapper {
  Access value;
};
using AccessWrapperSP = std::shared_ptr<AccessWrapper>;

struct InstrumentWrapper {
  Instrument value;
};
using InstrumentWrapperSP = std::shared_ptr<InstrumentWrapper>;

struct CharacteristicWrapper {
  Characteristic value;
};
using CharacteristicWrapperSP = std::shared_ptr<CharacteristicWrapper>;

// ── helpers
// ───────────────────────────────────────────────────────────────────

static void pack_opaque_port(InstrumentPortSP port, FalconResultSlot *out,
                             int32_t *oc) {
  out[0] = {};
  out[0].tag = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "InstrumentPort";
  out[0].value.opaque.ptr = new InstrumentPortSP(std::move(port));
  out[0].value.opaque.deleter = [](void *p) {
    delete static_cast<InstrumentPortSP *>(p);
  };
  *oc = 1;
}

static void pack_scope(Scope value, FalconResultSlot *out, int32_t *oc) {

  out[0] = {};
  out[0].tag = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Scope";

  out[0].value.opaque.ptr =
      new ScopeWrapperSP(std::make_shared<ScopeWrapper>(ScopeWrapper{value}));

  out[0].value.opaque.deleter = [](void *p) {
    delete static_cast<ScopeWrapperSP *>(p);
  };

  *oc = 1;
}

static void pack_access(Access value, FalconResultSlot *out, int32_t *oc) {

  out[0] = {};
  out[0].tag = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Access";

  out[0].value.opaque.ptr = new AccessWrapperSP(
      std::make_shared<AccessWrapper>(AccessWrapper{value}));

  out[0].value.opaque.deleter = [](void *p) {
    delete static_cast<AccessWrapperSP *>(p);
  };

  *oc = 1;
}

static void pack_instrument(Instrument value, FalconResultSlot *out,
                            int32_t *oc) {
  out[0] = {};
  out[0].tag = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Instrument";

  out[0].value.opaque.ptr = new InstrumentWrapperSP(
      std::make_shared<InstrumentWrapper>(InstrumentWrapper{value}));

  out[0].value.opaque.deleter = [](void *p) {
    delete static_cast<InstrumentWrapperSP *>(p);
  };

  *oc = 1;
}

static void pack_characteristic(Characteristic value, FalconResultSlot *out,
                                int32_t *oc) {

  out[0] = {};
  out[0].tag = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Characteristic";

  out[0].value.opaque.ptr = new CharacteristicWrapperSP(
      std::make_shared<CharacteristicWrapper>(CharacteristicWrapper{value}));

  out[0].value.opaque.deleter = [](void *p) {
    delete static_cast<CharacteristicWrapperSP *>(p);
  };

  *oc = 1;
}

static ConnectionSP get_optional_connection(const FalconParamEntry *params,
                                            int32_t param_count,
                                            const char *key) {

  for (int32_t i = 0; i < param_count; ++i) {
    if (std::strcmp(params[i].key, key) != 0) {
      continue;
    }

    if (params[i].tag == FALCON_TYPE_NIL) {
      return {};
    }

    return get_opaque<Connection>(params, param_count, key);
  }

  return {};
}

extern "C" {

// ── Constructors
// ──────���───────────────────────────────────────────────────────
//
//
void STRUCTAccessNewReadAccess(const FalconParamEntry *, int32_t,
                               FalconResultSlot *out, int32_t *oc) {
  pack_access(Access::Read, out, oc);
}
void STRUCTAccessNewWriteAccess(const FalconParamEntry *, int32_t,
                                FalconResultSlot *out, int32_t *oc) {
  pack_access(Access::Write, out, oc);
}
void STRUCTAccessNewReadWriteAccess(const FalconParamEntry *, int32_t,
                                    FalconResultSlot *out, int32_t *oc) {
  pack_access(Access::ReadWrite, out, oc);
}

void STRUCTScopeNewLocalScope(const FalconParamEntry *, int32_t,
                              FalconResultSlot *out, int32_t *oc) {
  pack_scope(Scope::Local, out, oc);
}
void STRUCTScopeNewGlobalScope(const FalconParamEntry *, int32_t,
                               FalconResultSlot *out, int32_t *oc) {
  pack_scope(Scope::Global, out, oc);
}

void STRUCTInstrumentNewDcVoltageSource(const FalconParamEntry *, int32_t,
                                        FalconResultSlot *out, int32_t *oc) {
  pack_instrument(Instrument::DC_Voltage_Source, out, oc);
}
void STRUCTInstrumentNewAmnmeter(const FalconParamEntry *, int32_t,
                                 FalconResultSlot *out, int32_t *oc) {
  pack_instrument(Instrument::Amnmeter, out, oc);
}
void STRUCTInstrumentNewClock(const FalconParamEntry *, int32_t,
                              FalconResultSlot *out, int32_t *oc) {
  pack_instrument(Instrument::Clock, out, oc);
}
void STRUCTInstrumentNewDiscrete(const FalconParamEntry *, int32_t,
                                 FalconResultSlot *out, int32_t *oc) {
  pack_instrument(Instrument::Discrete, out, oc);
}

void STRUCTCharacteristicNewNone(const FalconParamEntry *, int32_t,
                                 FalconResultSlot *out, int32_t *oc) {
  pack_characteristic(Characteristic::None, out, oc);
}
void STRUCTCharacteristicNewSampleRate(const FalconParamEntry *, int32_t,
                                       FalconResultSlot *out, int32_t *oc) {
  pack_characteristic(Characteristic::Sample_Rate, out, oc);
}
void STRUCTCharacteristicNewMaxSampleRate(const FalconParamEntry *, int32_t,
                                          FalconResultSlot *out, int32_t *oc) {
  pack_characteristic(Characteristic::Max_Sample_Rate, out, oc);
}
void STRUCTCharacteristicNewMinSampleRate(const FalconParamEntry *, int32_t,
                                          FalconResultSlot *out, int32_t *oc) {
  pack_characteristic(Characteristic::Min_Sample_Rate, out, oc);
}
void STRUCTCharacteristicNewNumberOfSamples(const FalconParamEntry *, int32_t,
                                            FalconResultSlot *out,
                                            int32_t *oc) {
  pack_characteristic(Characteristic::Number_Of_Samples, out, oc);
}
void STRUCTCharacteristicNewMinNumberOfSamples(const FalconParamEntry *,
                                               int32_t, FalconResultSlot *out,
                                               int32_t *oc) {
  pack_characteristic(Characteristic::Min_Number_Of_Samples, out, oc);
}
void STRUCTCharacteristicNewMaxNumberOfSamples(const FalconParamEntry *,
                                               int32_t, FalconResultSlot *out,
                                               int32_t *oc) {
  pack_characteristic(Characteristic::Max_Number_Of_Samples, out, oc);
}
void STRUCTCharacteristicNewSourceVoltage(const FalconParamEntry *, int32_t,
                                          FalconResultSlot *out, int32_t *oc) {
  pack_characteristic(Characteristic::Source_Voltage, out, oc);
}
void STRUCTCharacteristicNewMinSourceVoltage(const FalconParamEntry *, int32_t,
                                             FalconResultSlot *out,
                                             int32_t *oc) {
  pack_characteristic(Characteristic::Min_Source_Voltage, out, oc);
}
void STRUCTCharacteristicNewMaxSourceVoltage(const FalconParamEntry *, int32_t,
                                             FalconResultSlot *out,
                                             int32_t *oc) {
  pack_characteristic(Characteristic::Max_Source_Voltage, out, oc);
}
void STRUCTCharacteristicNewVoltageRampSlope(const FalconParamEntry *, int32_t,
                                             FalconResultSlot *out,
                                             int32_t *oc) {
  pack_characteristic(Characteristic::Voltage_Ramp_Slope, out, oc);
}
void STRUCTCharacteristicNewMinVoltageRampSlope(const FalconParamEntry *,
                                                int32_t, FalconResultSlot *out,
                                                int32_t *oc) {
  pack_characteristic(Characteristic::Min_Voltage_Ramp_Slope, out, oc);
}
void STRUCTCharacteristicNewMaxVoltageRampSlope(const FalconParamEntry *,
                                                int32_t, FalconResultSlot *out,
                                                int32_t *oc) {
  pack_characteristic(Characteristic::Max_Voltage_Ramp_Slope, out, oc);
}

// NewKnob(default_name: string, conn: Connection,
//         units: SymbolUnit, description: string,
//         instrument_name: string, inst: Instrument)
//     -> (InstrumentPort port)
void STRUCTInstrumentPortNewKnob(const FalconParamEntry *params,
                                 int32_t param_count, FalconResultSlot *out,
                                 int32_t *oc) {
  auto pm = unpack_params(params, param_count);

  auto default_name = std::get<std::string>(pm.at("default_name"));
  auto conn = get_optional_connection(params, param_count, "conn");
  auto unit = get_opaque<SymbolUnit>(params, param_count, "units");
  auto description = std::get<std::string>(pm.at("description"));

  auto inst = get_opaque<InstrumentWrapper>(params, param_count, "inst");

  std::string instrument_name;
  if (pm.find("instrument_name") != pm.end()) {
    instrument_name = std::get<std::string>(pm.at("instrument_name"));
  }

  auto port = InstrumentPort::Knob(default_name, instrument_name, conn,
                                   inst->value, unit, description);

  pack_opaque_port(std::move(port), out, oc);
}

// NewMeter(default_name: string, conn: Connection,
//          units: SymbolUnit, description: string,
//          instrument_name: string, inst: Instrument)
//      -> (InstrumentPort port)
void STRUCTInstrumentPortNewMeter(const FalconParamEntry *params,
                                  int32_t param_count, FalconResultSlot *out,
                                  int32_t *oc) {
  auto pm = unpack_params(params, param_count);

  auto default_name = std::get<std::string>(pm.at("default_name"));
  auto conn = get_optional_connection(params, param_count, "conn");
  auto unit = get_opaque<SymbolUnit>(params, param_count, "units");
  auto description = std::get<std::string>(pm.at("description"));

  auto inst = get_opaque<InstrumentWrapper>(params, param_count, "inst");

  std::string instrument_name;
  if (pm.find("instrument_name") != pm.end()) {
    instrument_name = std::get<std::string>(pm.at("instrument_name"));
  }

  auto port = InstrumentPort::Meter(default_name, instrument_name, conn,
                                    inst->value, unit, description);

  pack_opaque_port(std::move(port), out, oc);
}

// NewSetting(default_name: string,
//            conn: Connection,
//            units: SymbolUnit,
//            description: string,
//            instrument_name: string,
//            inst: Instrument,
//            scope: Scope,
//            access: Access,
//            char: Characteristic)
//      -> (InstrumentPort port)
void STRUCTInstrumentPortNewSetting(const FalconParamEntry *params,
                                    int32_t param_count, FalconResultSlot *out,
                                    int32_t *oc) {
  auto pm = unpack_params(params, param_count);

  auto default_name = std::get<std::string>(pm.at("default_name"));
  auto conn = get_optional_connection(params, param_count, "conn");
  auto unit = get_opaque<SymbolUnit>(params, param_count, "units");
  auto description = std::get<std::string>(pm.at("description"));

  auto inst = get_opaque<InstrumentWrapper>(params, param_count, "inst");

  auto scope = get_opaque<ScopeWrapper>(params, param_count, "scope");

  auto access = get_opaque<AccessWrapper>(params, param_count, "access");

  auto characteristic =
      get_opaque<CharacteristicWrapper>(params, param_count, "char");

  std::string instrument_name;
  if (pm.find("instrument_name") != pm.end()) {
    instrument_name = std::get<std::string>(pm.at("instrument_name"));
  }

  auto port = InstrumentPort::Setting(
      default_name, instrument_name, scope->value, access->value,
      characteristic->value, conn, inst->value, unit, description);

  pack_opaque_port(std::move(port), out, oc);
}

// NewTimer() -> (InstrumentPort port)
void STRUCTInstrumentPortNewTimer(const FalconParamEntry *, int32_t,
                                  FalconResultSlot *out, int32_t *oc) {
  auto port = InstrumentPort::Timer();
  pack_opaque_port(std::move(port), out, oc);
}

// NewExecutionClock() -> (InstrumentPort port)
void STRUCTInstrumentPortNewExecutionClock(const FalconParamEntry *, int32_t,
                                           FalconResultSlot *out, int32_t *oc) {
  auto port = InstrumentPort::ExecutionClock();
  pack_opaque_port(std::move(port), out, oc);
}

// ── Accessors
// ─────────────────────────────────────────────────────────────────
void STRUCTAccessIsReadAccess(const FalconParamEntry *params,
                              int32_t param_count, FalconResultSlot *out,
                              int32_t *oc) {
  auto port = get_opaque<AccessWrapper>(params, param_count, "this");
  pack_results(FunctionResult{port->value == Access::Read}, out, 16, oc);
}
void STRUCTAccessIsWriteAccess(const FalconParamEntry *params,
                               int32_t param_count, FalconResultSlot *out,
                               int32_t *oc) {
  auto port = get_opaque<AccessWrapper>(params, param_count, "this");
  pack_results(FunctionResult{port->value == Access::Write}, out, 16, oc);
}
void STRUCTAccessIsReadWriteAccess(const FalconParamEntry *params,
                                   int32_t param_count, FalconResultSlot *out,
                                   int32_t *oc) {
  auto port = get_opaque<AccessWrapper>(params, param_count, "this");
  pack_results(FunctionResult{port->value == Access::ReadWrite}, out, 16, oc);
}

void STRUCTScopeIsLocalScope(const FalconParamEntry *params,
                             int32_t param_count, FalconResultSlot *out,
                             int32_t *oc) {
  auto port = get_opaque<ScopeWrapper>(params, param_count, "this");
  pack_results(FunctionResult{port->value == Scope::Local}, out, 16, oc);
}
void STRUCTScopeIsGlobalScope(const FalconParamEntry *params,
                              int32_t param_count, FalconResultSlot *out,
                              int32_t *oc) {
  auto port = get_opaque<ScopeWrapper>(params, param_count, "this");
  pack_results(FunctionResult{port->value == Scope::Global}, out, 16, oc);
}

void STRUCTInstrumentIsDcVoltageSource(const FalconParamEntry *params,
                                       int32_t param_count,
                                       FalconResultSlot *out, int32_t *oc) {
  auto port = get_opaque<InstrumentWrapper>(params, param_count, "this");
  pack_results(FunctionResult{port->value == Instrument::DC_Voltage_Source},
               out, 16, oc);
}
void STRUCTInstrumentIsAmnmeter(const FalconParamEntry *params,
                                int32_t param_count, FalconResultSlot *out,
                                int32_t *oc) {
  auto port = get_opaque<InstrumentWrapper>(params, param_count, "this");
  pack_results(FunctionResult{port->value == Instrument::Amnmeter}, out, 16,
               oc);
}
void STRUCTInstrumentIsClock(const FalconParamEntry *params,
                             int32_t param_count, FalconResultSlot *out,
                             int32_t *oc) {
  auto port = get_opaque<InstrumentWrapper>(params, param_count, "this");
  pack_results(FunctionResult{port->value == Instrument::Clock}, out, 16, oc);
}
void STRUCTInstrumentIsDiscrete(const FalconParamEntry *params,
                                int32_t param_count, FalconResultSlot *out,
                                int32_t *oc) {
  auto port = get_opaque<InstrumentWrapper>(params, param_count, "this");
  pack_results(FunctionResult{port->value == Instrument::Discrete}, out, 16,
               oc);
}

void STRUCTCharacteristicIsNone(const FalconParamEntry *params,
                                int32_t param_count, FalconResultSlot *out,
                                int32_t *oc) {
  auto port = get_opaque<CharacteristicWrapper>(params, param_count, "this");
  pack_results(FunctionResult{port->value == Characteristic::None}, out, 16,
               oc);
}
void STRUCTCharacteristicIsSampleRate(const FalconParamEntry *params,
                                      int32_t param_count,
                                      FalconResultSlot *out, int32_t *oc) {
  auto port = get_opaque<CharacteristicWrapper>(params, param_count, "this");
  pack_results(FunctionResult{port->value == Characteristic::Sample_Rate}, out,
               16, oc);
}
void STRUCTCharacteristicIsMaxSampleRate(const FalconParamEntry *params,
                                         int32_t param_count,
                                         FalconResultSlot *out, int32_t *oc) {
  auto port = get_opaque<CharacteristicWrapper>(params, param_count, "this");
  pack_results(FunctionResult{port->value == Characteristic::Max_Sample_Rate},
               out, 16, oc);
}
void STRUCTCharacteristicIsMinSampleRate(const FalconParamEntry *params,
                                         int32_t param_count,
                                         FalconResultSlot *out, int32_t *oc) {
  auto port = get_opaque<CharacteristicWrapper>(params, param_count, "this");
  pack_results(FunctionResult{port->value == Characteristic::Min_Sample_Rate},
               out, 16, oc);
}
void STRUCTCharacteristicIsNumberOfSamples(const FalconParamEntry *params,
                                           int32_t param_count,
                                           FalconResultSlot *out, int32_t *oc) {
  auto port = get_opaque<CharacteristicWrapper>(params, param_count, "this");
  pack_results(FunctionResult{port->value == Characteristic::Number_Of_Samples},
               out, 16, oc);
}
void STRUCTCharacteristicIsMaxNumberOfSamples(const FalconParamEntry *params,
                                              int32_t param_count,
                                              FalconResultSlot *out,
                                              int32_t *oc) {
  auto port = get_opaque<CharacteristicWrapper>(params, param_count, "this");
  pack_results(
      FunctionResult{port->value == Characteristic::Max_Number_Of_Samples}, out,
      16, oc);
}
void STRUCTCharacteristicIsMinNumberOfSamples(const FalconParamEntry *params,
                                              int32_t param_count,
                                              FalconResultSlot *out,
                                              int32_t *oc) {
  auto port = get_opaque<CharacteristicWrapper>(params, param_count, "this");
  pack_results(
      FunctionResult{port->value == Characteristic::Min_Number_Of_Samples}, out,
      16, oc);
}
void STRUCTCharacteristicIsSourceVoltage(const FalconParamEntry *params,
                                         int32_t param_count,
                                         FalconResultSlot *out, int32_t *oc) {
  auto port = get_opaque<CharacteristicWrapper>(params, param_count, "this");
  pack_results(FunctionResult{port->value == Characteristic::Source_Voltage},
               out, 16, oc);
}
void STRUCTCharacteristicIsMaxSourceVoltage(const FalconParamEntry *params,
                                            int32_t param_count,
                                            FalconResultSlot *out,
                                            int32_t *oc) {
  auto port = get_opaque<CharacteristicWrapper>(params, param_count, "this");
  pack_results(
      FunctionResult{port->value == Characteristic::Max_Source_Voltage}, out,
      16, oc);
}
void STRUCTCharacteristicIsMinSourceVoltage(const FalconParamEntry *params,
                                            int32_t param_count,
                                            FalconResultSlot *out,
                                            int32_t *oc) {
  auto port = get_opaque<CharacteristicWrapper>(params, param_count, "this");
  pack_results(
      FunctionResult{port->value == Characteristic::Min_Source_Voltage}, out,
      16, oc);
}
void STRUCTCharacteristicIsVoltageRampSlope(const FalconParamEntry *params,
                                            int32_t param_count,
                                            FalconResultSlot *out,
                                            int32_t *oc) {
  auto port = get_opaque<CharacteristicWrapper>(params, param_count, "this");
  pack_results(
      FunctionResult{port->value == Characteristic::Voltage_Ramp_Slope}, out,
      16, oc);
}
void STRUCTCharacteristicIsMaxVoltageRampSlope(const FalconParamEntry *params,
                                               int32_t param_count,
                                               FalconResultSlot *out,
                                               int32_t *oc) {
  auto port = get_opaque<CharacteristicWrapper>(params, param_count, "this");
  pack_results(
      FunctionResult{port->value == Characteristic::Max_Voltage_Ramp_Slope},
      out, 16, oc);
}
void STRUCTCharacteristicIsMinVoltageRampSlope(const FalconParamEntry *params,
                                               int32_t param_count,
                                               FalconResultSlot *out,
                                               int32_t *oc) {
  auto port = get_opaque<CharacteristicWrapper>(params, param_count, "this");
  pack_results(
      FunctionResult{port->value == Characteristic::Min_Voltage_Ramp_Slope},
      out, 16, oc);
}

// DefaultName(this: InstrumentPort) -> (string default_name)
void STRUCTInstrumentPortDefaultName(const FalconParamEntry *params,
                                     int32_t param_count, FalconResultSlot *out,
                                     int32_t *oc) {
  auto port = get_opaque<InstrumentPort>(params, param_count, "this");
  pack_results(FunctionResult{port->default_name()}, out, 16, oc);
}

// InstrumentName(this: InstrumentPort) -> (string instrument_name)
void STRUCTInstrumentPortInstrumentName(const FalconParamEntry *params,
                                        int32_t param_count,
                                        FalconResultSlot *out, int32_t *oc) {
  auto port = get_opaque<InstrumentPort>(params, param_count, "this");
  pack_results(FunctionResult{port->instrument_name()}, out, 16, oc);
}

// PsuedoName(this: InstrumentPort) -> (Connection conn)
void STRUCTInstrumentPortPsuedoName(const FalconParamEntry *params,
                                    int32_t param_count, FalconResultSlot *out,
                                    int32_t *oc) {
  auto port = get_opaque<InstrumentPort>(params, param_count, "this");
  auto conn = port->pseudo_name();
  out[0] = {};
  out[0].tag = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Connection";
  out[0].value.opaque.ptr = new ConnectionSP(conn);
  out[0].value.opaque.deleter = [](void *p) {
    delete static_cast<ConnectionSP *>(p);
  };
  *oc = 1;
}

// InstrumentType(this: InstrumentPort) -> (Instrument inst)
void STRUCTInstrumentPortInstrumentType(const FalconParamEntry *params,
                                        int32_t param_count,
                                        FalconResultSlot *out, int32_t *oc) {

  auto port = get_opaque<InstrumentPort>(params, param_count, "this");
  pack_instrument(port->instrument_type(), out, oc);
}

// Scope(this: InstrumentPort) -> (Scope scope)
void STRUCTInstrumentPortScope(const FalconParamEntry *params,
                               int32_t param_count, FalconResultSlot *out,
                               int32_t *oc) {

  auto port = get_opaque<InstrumentPort>(params, param_count, "this");

  pack_scope(port->scope(), out, oc);
}

// Access(this: InstrumentPort) -> (Access access)
void STRUCTInstrumentPortAccess(const FalconParamEntry *params,
                                int32_t param_count, FalconResultSlot *out,
                                int32_t *oc) {

  auto port = get_opaque<InstrumentPort>(params, param_count, "this");

  pack_access(port->access(), out, oc);
}

// Characteristic(this: InstrumentPort) -> (Characteristic char)
void STRUCTInstrumentPortCharacteristic(const FalconParamEntry *params,
                                        int32_t param_count,
                                        FalconResultSlot *out, int32_t *oc) {

  auto port = get_opaque<InstrumentPort>(params, param_count, "this");

  pack_characteristic(port->characteristic(), out, oc);
}

// Units(this: InstrumentPort) -> (SymbolUnit units)
void STRUCTInstrumentPortUnits(const FalconParamEntry *params,
                               int32_t param_count, FalconResultSlot *out,
                               int32_t *oc) {
  auto port = get_opaque<InstrumentPort>(params, param_count, "this");
  auto unit = port->units();
  out[0] = {};
  out[0].tag = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "SymbolUnit";
  out[0].value.opaque.ptr = new SymbolUnitSP(unit);
  out[0].value.opaque.deleter = [](void *p) {
    delete static_cast<SymbolUnitSP *>(p);
  };
  *oc = 1;
}

// Description(this: InstrumentPort) -> (string description)
void STRUCTInstrumentPortDescription(const FalconParamEntry *params,
                                     int32_t param_count, FalconResultSlot *out,
                                     int32_t *oc) {
  auto port = get_opaque<InstrumentPort>(params, param_count, "this");
  pack_results(FunctionResult{port->description()}, out, 16, oc);
}

// InstrumentFacingName(this: InstrumentPort) -> (string facing_name)
void STRUCTInstrumentPortInstrumentFacingName(const FalconParamEntry *params,
                                              int32_t param_count,
                                              FalconResultSlot *out,
                                              int32_t *oc) {
  auto port = get_opaque<InstrumentPort>(params, param_count, "this");
  pack_results(FunctionResult{port->instrument_facing_name()}, out, 16, oc);
}

// IsKnob(this: InstrumentPort) -> (bool is_knob)
void STRUCTInstrumentPortIsKnob(const FalconParamEntry *params,
                                int32_t param_count, FalconResultSlot *out,
                                int32_t *oc) {
  auto port = get_opaque<InstrumentPort>(params, param_count, "this");
  pack_results(FunctionResult{port->is_knob()}, out, 16, oc);
}

// IsMeter(this: InstrumentPort) -> (bool is_meter)
void STRUCTInstrumentPortIsMeter(const FalconParamEntry *params,
                                 int32_t param_count, FalconResultSlot *out,
                                 int32_t *oc) {
  auto port = get_opaque<InstrumentPort>(params, param_count, "this");
  pack_results(FunctionResult{port->is_meter()}, out, 16, oc);
}

// IsSetting(this: InstrumentPort) -> (bool is_setting)
void STRUCTInstrumentPortIsSetting(const FalconParamEntry *params,
                                   int32_t param_count, FalconResultSlot *out,
                                   int32_t *oc) {
  auto port = get_opaque<InstrumentPort>(params, param_count, "this");
  pack_results(FunctionResult{port->is_setting()}, out, 16, oc);
}

// ── Equality
// ──────────────────────────────────────────────────────────────────

// Equal(this: InstrumentPort, other: InstrumentPort) -> (bool equal)
void STRUCTInstrumentPortEqual(const FalconParamEntry *params,
                               int32_t param_count, FalconResultSlot *out,
                               int32_t *oc) {
  auto self = get_opaque<InstrumentPort>(params, param_count, "this");
  auto other = get_opaque<InstrumentPort>(params, param_count, "other");
  pack_results(FunctionResult{*self == *other}, out, 16, oc);
}

// NotEqual(this: InstrumentPort, other: InstrumentPort) -> (bool notequal)
void STRUCTInstrumentPortNotEqual(const FalconParamEntry *params,
                                  int32_t param_count, FalconResultSlot *out,
                                  int32_t *oc) {
  auto self = get_opaque<InstrumentPort>(params, param_count, "this");
  auto other = get_opaque<InstrumentPort>(params, param_count, "other");
  pack_results(FunctionResult{*self != *other}, out, 16, oc);
}

// ── JSON
// ──────────────────────────────────────────────────────────────────────

// ToJSON(this: InstrumentPort) -> (string json)
void STRUCTInstrumentPortToJSON(const FalconParamEntry *params,
                                int32_t param_count, FalconResultSlot *out,
                                int32_t *oc) {
  auto port = get_opaque<InstrumentPort>(params, param_count, "this");
  pack_results(FunctionResult{port->to_json_string()}, out, 16, oc);
}

// FromJSON(json: string) -> (InstrumentPort port)
void STRUCTInstrumentPortFromJSON(const FalconParamEntry *params,
                                  int32_t param_count, FalconResultSlot *out,
                                  int32_t *oc) {
  auto pm = unpack_params(params, param_count);
  auto json = std::get<std::string>(pm.at("json"));
  auto port = InstrumentPort::from_json_string<InstrumentPort>(json);
  pack_opaque_port(std::make_shared<InstrumentPort>(*port), out, oc);
}

} // extern "C"
