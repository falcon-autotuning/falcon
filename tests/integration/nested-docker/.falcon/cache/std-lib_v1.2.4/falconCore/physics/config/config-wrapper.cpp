#include "falcon-core/physics/config/Loader.hpp"
#include "falcon-core/physics/config/core/Config.hpp"
#include "falcon-core/physics/config/core/Group.hpp"
#include "falcon-core/physics/config/core/VoltageConstraints.hpp"
#include "falcon-core/physics/config/core/Adjacency.hpp"
#include "falcon-core/physics/device_structures/Connection.hpp"
#include "falcon-core/physics/device_structures/Connections.hpp"
#include "falcon-core/physics/device_structures/Impedance.hpp"
#include "falcon-core/physics/device_structures/Impedances.hpp"
#include "falcon-core/autotuner_interfaces/names/Channel.hpp"
#include "falcon-core/autotuner_interfaces/names/Channels.hpp"
#include "falcon-core/autotuner_interfaces/names/Gname.hpp"
#include "falcon-core/generic/FArray.hpp"
#include <falcon-core/CerealRegistry.hpp>
#include <falcon-typing/FFIHelpers.hpp>
#include <stdexcept>
#include <vector>

using namespace falcon::typing;
using namespace falcon::typing::ffi::wrapper;

using Config = falcon_core::physics::config::core::Config;
using ConfigSP = std::shared_ptr<Config>;
using Loader = falcon_core::physics::config::Loader;
using LoaderSP = std::shared_ptr<Loader>;
using Group = falcon_core::physics::config::core::Group;
using GroupSP = std::shared_ptr<Group>;
using VoltageConstraints = falcon_core::physics::config::core::VoltageConstraints;
using VoltageConstraintsSP = std::shared_ptr<VoltageConstraints>;
using Adjacency = falcon_core::physics::config::core::Adjacency;
using AdjacencySP = std::shared_ptr<Adjacency>;
using Connection = falcon_core::physics::device_structures::Connection;
using ConnectionSP = std::shared_ptr<Connection>;
using Connections = falcon_core::physics::device_structures::Connections;
using ConnectionsSP = std::shared_ptr<Connections>;
using Impedance = falcon_core::physics::device_structures::Impedance;
using ImpedanceSP = std::shared_ptr<Impedance>;
using Impedances = falcon_core::physics::device_structures::Impedances;
using ImpedancesSP = std::shared_ptr<Impedances>;
using Channel = falcon_core::autotuner_interfaces::names::Channel;
using ChannelSP = std::shared_ptr<Channel>;
using Channels = falcon_core::autotuner_interfaces::names::Channels;
using ChannelsSP = std::shared_ptr<Channels>;
using Gname = falcon_core::autotuner_interfaces::names::Gname;
using GnameSP = std::shared_ptr<Gname>;
using FArray = falcon_core::generic::FArray<double>;
using FArraySP = std::shared_ptr<FArray>;

// ── pack helpers ─────────────────────────────────────────────────────────────

static void pack_opaque_config(ConfigSP cfg, FalconResultSlot *out, int32_t *oc) {
  out[0] = {};
  out[0].tag = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Config";
  out[0].value.opaque.ptr = new ConfigSP(std::move(cfg));
  out[0].value.opaque.deleter = [](void *p) { delete static_cast<ConfigSP *>(p); };
  *oc = 1;
}

static void pack_opaque_loader(LoaderSP ldr, FalconResultSlot *out, int32_t *oc) {
  out[0] = {};
  out[0].tag = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Loader";
  out[0].value.opaque.ptr = new LoaderSP(std::move(ldr));
  out[0].value.opaque.deleter = [](void *p) { delete static_cast<LoaderSP *>(p); };
  *oc = 1;
}

static void pack_opaque_group(GroupSP grp, FalconResultSlot *out, int32_t *oc) {
  out[0] = {};
  out[0].tag = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Group";
  out[0].value.opaque.ptr = new GroupSP(std::move(grp));
  out[0].value.opaque.deleter = [](void *p) { delete static_cast<GroupSP *>(p); };
  *oc = 1;
}

static void pack_opaque_vc(VoltageConstraintsSP vc, FalconResultSlot *out, int32_t *oc) {
  out[0] = {};
  out[0].tag = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "VoltageConstraints";
  out[0].value.opaque.ptr = new VoltageConstraintsSP(std::move(vc));
  out[0].value.opaque.deleter = [](void *p) { delete static_cast<VoltageConstraintsSP *>(p); };
  *oc = 1;
}

static void pack_opaque_adj(AdjacencySP adj, FalconResultSlot *out, int32_t *oc) {
  out[0] = {};
  out[0].tag = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Adjacency";
  out[0].value.opaque.ptr = new AdjacencySP(std::move(adj));
  out[0].value.opaque.deleter = [](void *p) { delete static_cast<AdjacencySP *>(p); };
  *oc = 1;
}

static void pack_opaque_channel(ChannelSP ch, FalconResultSlot *out, int32_t *oc) {
  out[0] = {};
  out[0].tag = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Channel";
  out[0].value.opaque.ptr = new ChannelSP(std::move(ch));
  out[0].value.opaque.deleter = [](void *p) { delete static_cast<ChannelSP *>(p); };
  *oc = 1;
}

static void pack_opaque_channels(ChannelsSP chs, FalconResultSlot *out, int32_t *oc) {
  out[0] = {};
  out[0].tag = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Channels";
  out[0].value.opaque.ptr = new ChannelsSP(std::move(chs));
  out[0].value.opaque.deleter = [](void *p) { delete static_cast<ChannelsSP *>(p); };
  *oc = 1;
}

static void pack_opaque_gname(GnameSP gn, FalconResultSlot *out, int32_t *oc) {
  out[0] = {};
  out[0].tag = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Gname";
  out[0].value.opaque.ptr = new GnameSP(std::move(gn));
  out[0].value.opaque.deleter = [](void *p) { delete static_cast<GnameSP *>(p); };
  *oc = 1;
}

static void pack_opaque_connection(ConnectionSP conn, FalconResultSlot *out, int32_t *oc) {
  out[0] = {};
  out[0].tag = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Connection";
  out[0].value.opaque.ptr = new ConnectionSP(std::move(conn));
  out[0].value.opaque.deleter = [](void *p) { delete static_cast<ConnectionSP *>(p); };
  *oc = 1;
}

static void pack_opaque_connections(ConnectionsSP conns, FalconResultSlot *out, int32_t *oc) {
  out[0] = {};
  out[0].tag = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Connections";
  out[0].value.opaque.ptr = new ConnectionsSP(std::move(conns));
  out[0].value.opaque.deleter = [](void *p) { delete static_cast<ConnectionsSP *>(p); };
  *oc = 1;
}

static void pack_opaque_imp(ImpedanceSP imp, FalconResultSlot *out, int32_t *oc) {
  out[0] = {};
  out[0].tag = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Impedance";
  out[0].value.opaque.ptr = new ImpedanceSP(std::move(imp));
  out[0].value.opaque.deleter = [](void *p) { delete static_cast<ImpedanceSP *>(p); };
  *oc = 1;
}

static void pack_opaque_imps(ImpedancesSP imps, FalconResultSlot *out, int32_t *oc) {
  out[0] = {};
  out[0].tag = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Impedances";
  out[0].value.opaque.ptr = new ImpedancesSP(std::move(imps));
  out[0].value.opaque.deleter = [](void *p) { delete static_cast<ImpedancesSP *>(p); };
  *oc = 1;
}

static void pack_opaque_farray(FArraySP arr, FalconResultSlot *out, int32_t *oc) {
  out[0] = {};
  out[0].tag = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "FArray";
  out[0].value.opaque.ptr = new FArraySP(std::move(arr));
  out[0].value.opaque.deleter = [](void *p) { delete static_cast<FArraySP *>(p); };
  *oc = 1;
}

static RuntimeValue wrap_conn_as_struct(ConnectionSP conn) {
  auto inst = std::make_shared<StructInstance>("Connection");
  inst->native_handle = std::static_pointer_cast<void>(conn);
  return inst;
}

static RuntimeValue wrap_channel_as_struct(ChannelSP ch) {
  auto inst = std::make_shared<StructInstance>("Channel");
  inst->native_handle = std::static_pointer_cast<void>(ch);
  return inst;
}

static RuntimeValue wrap_gname_as_struct(GnameSP gn) {
  auto inst = std::make_shared<StructInstance>("Gname");
  inst->native_handle = std::static_pointer_cast<void>(gn);
  return inst;
}

static void pack_array_result(std::shared_ptr<ArrayValue> arr, FalconResultSlot *out, int32_t *oc) {
  out[0] = {};
  out[0].tag = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Array";
  out[0].value.opaque.ptr = new std::shared_ptr<void>(std::static_pointer_cast<void>(arr));
  out[0].value.opaque.deleter = [](void *p) { delete static_cast<std::shared_ptr<void> *>(p); };
  *oc = 1;
}

static void pack_conns_as_array(ConnectionsSP conns, FalconResultSlot *out, int32_t *oc) {
  auto arr_val = std::make_shared<ArrayValue>();
  if (conns) {
    arr_val->elements.reserve(conns->size());
    for (const auto &item : *conns) {
      arr_val->elements.push_back(wrap_conn_as_struct(item));
    }
  }
  pack_array_result(arr_val, out, oc);
}

extern "C" {

// ── Adjacency ───────────────────────────────────────────────────────────────

void STRUCTAdjacencyEqual(const FalconParamEntry *params, int32_t param_count,
                          FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Adjacency>(params, param_count, "this");
  auto other = get_opaque<Adjacency>(params, param_count, "other");
  pack_results(FunctionResult{*self == *other}, out, 16, oc);
}

void STRUCTAdjacencyNotEqual(const FalconParamEntry *params, int32_t param_count,
                             FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Adjacency>(params, param_count, "this");
  auto other = get_opaque<Adjacency>(params, param_count, "other");
  pack_results(FunctionResult{*self != *other}, out, 16, oc);
}

void STRUCTAdjacencyToJSON(const FalconParamEntry *params, int32_t param_count,
                           FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Adjacency>(params, param_count, "this");
  pack_results(FunctionResult{self->to_json_string()}, out, 16, oc);
}

void STRUCTAdjacencyFromJSON(const FalconParamEntry *params, int32_t param_count,
                             FalconResultSlot *out, int32_t *oc) {
  auto pm = unpack_params(params, param_count);
  auto json = std::get<std::string>(pm.at("json"));
  auto adj = Adjacency::from_json_string<Adjacency>(json);
  pack_opaque_adj(std::make_shared<Adjacency>(*adj), out, oc);
}

// ── VoltageConstraints ─────────────────────────────────────────────────────

void STRUCTVoltageConstraintsMatrix(const FalconParamEntry *, int32_t,
                                    FalconResultSlot *, int32_t *) {
  throw std::runtime_error("VoltageConstraints::matrix is not implemented");
}

void STRUCTVoltageConstraintsLimits(const FalconParamEntry *, int32_t,
                                    FalconResultSlot *, int32_t *) {
  throw std::runtime_error("VoltageConstraints::limits is not implemented");
}

void STRUCTVoltageConstraintsAdjacency(const FalconParamEntry *params, int32_t param_count,
                                       FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<VoltageConstraints>(params, param_count, "this");
  pack_opaque_adj(self->adjacency(), out, oc);
}

void STRUCTVoltageConstraintsEqual(const FalconParamEntry *params, int32_t param_count,
                                   FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<VoltageConstraints>(params, param_count, "this");
  auto other = get_opaque<VoltageConstraints>(params, param_count, "other");
  pack_results(FunctionResult{*self == *other}, out, 16, oc);
}

void STRUCTVoltageConstraintsNotEqual(const FalconParamEntry *params, int32_t param_count,
                                      FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<VoltageConstraints>(params, param_count, "this");
  auto other = get_opaque<VoltageConstraints>(params, param_count, "other");
  pack_results(FunctionResult{*self != *other}, out, 16, oc);
}

void STRUCTVoltageConstraintsToJSON(const FalconParamEntry *params, int32_t param_count,
                                    FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<VoltageConstraints>(params, param_count, "this");
  pack_results(FunctionResult{self->to_json_string()}, out, 16, oc);
}

void STRUCTVoltageConstraintsFromJSON(const FalconParamEntry *params, int32_t param_count,
                                      FalconResultSlot *out, int32_t *oc) {
  auto pm = unpack_params(params, param_count);
  auto json = std::get<std::string>(pm.at("json"));
  auto vc = VoltageConstraints::from_json_string<VoltageConstraints>(json);
  pack_opaque_vc(std::make_shared<VoltageConstraints>(*vc), out, oc);
}

// ── Group ──────────────────────────────────────────────────────────────────

void STRUCTGroupName(const FalconParamEntry *params, int32_t param_count,
                     FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Group>(params, param_count, "this");
  pack_opaque_channel(self->name(), out, oc);
}

void STRUCTGroupNumDots(const FalconParamEntry *params, int32_t param_count,
                        FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Group>(params, param_count, "this");
  pack_results(FunctionResult{static_cast<int64_t>(self->num_dots())}, out, 16, oc);
}

void STRUCTGroupScreeningGates(const FalconParamEntry *params, int32_t param_count,
                              FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Group>(params, param_count, "this");
  pack_opaque_connections(self->screening_gates(), out, oc);
}

void STRUCTGroupReservoirGates(const FalconParamEntry *params, int32_t param_count,
                              FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Group>(params, param_count, "this");
  pack_opaque_connections(self->reservoir_gates(), out, oc);
}

void STRUCTGroupPlungerGates(const FalconParamEntry *params, int32_t param_count,
                             FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Group>(params, param_count, "this");
  pack_opaque_connections(self->plunger_gates(), out, oc);
}

void STRUCTGroupBarrierGates(const FalconParamEntry *params, int32_t param_count,
                             FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Group>(params, param_count, "this");
  pack_opaque_connections(self->barrier_gates(), out, oc);
}

void STRUCTGroupOhmics(const FalconParamEntry *params, int32_t param_count,
                       FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Group>(params, param_count, "this");
  pack_opaque_connections(self->ohmics(), out, oc);
}

void STRUCTGroupOrder(const FalconParamEntry *params, int32_t param_count,
                      FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Group>(params, param_count, "this");
  auto ord = self->order();
  std::vector<ConnectionSP> vec(ord->begin(), ord->end());
  pack_opaque_connections(std::make_shared<Connections>(vec), out, oc);
}

void STRUCTGroupEqual(const FalconParamEntry *params, int32_t param_count,
                      FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Group>(params, param_count, "this");
  auto other = get_opaque<Group>(params, param_count, "other");
  pack_results(FunctionResult{*self == *other}, out, 16, oc);
}

void STRUCTGroupNotEqual(const FalconParamEntry *params, int32_t param_count,
                         FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Group>(params, param_count, "this");
  auto other = get_opaque<Group>(params, param_count, "other");
  pack_results(FunctionResult{*self != *other}, out, 16, oc);
}

void STRUCTGroupToJSON(const FalconParamEntry *params, int32_t param_count,
                       FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Group>(params, param_count, "this");
  pack_results(FunctionResult{self->to_json_string()}, out, 16, oc);
}

void STRUCTGroupFromJSON(const FalconParamEntry *params, int32_t param_count,
                         FalconResultSlot *out, int32_t *oc) {
  auto pm = unpack_params(params, param_count);
  auto json = std::get<std::string>(pm.at("json"));
  auto grp = Group::from_json_string<Group>(json);
  pack_opaque_group(std::make_shared<Group>(*grp), out, oc);
}

// ── Loader ─────────────────────────────────────────────────────────────────

void STRUCTLoaderNew(const FalconParamEntry *params, int32_t param_count,
                     FalconResultSlot *out, int32_t *oc) {
  auto pm = unpack_params(params, param_count);
  auto path = std::get<std::string>(pm.at("config_path"));
  pack_opaque_loader(std::make_shared<Loader>(path), out, oc);
}

void STRUCTLoaderConfig(const FalconParamEntry *params, int32_t param_count,
                        FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Loader>(params, param_count, "this");
  pack_opaque_config(self->config(), out, oc);
}

// ── Config ─────────────────────────────────────────────────────────────────

void STRUCTConfigLoad(const FalconParamEntry *params, int32_t param_count,
                      FalconResultSlot *out, int32_t *oc) {
  auto pm = unpack_params(params, param_count);
  auto path = std::get<std::string>(pm.at("config_path"));
  Loader ldr(path);
  pack_opaque_config(ldr.config(), out, oc);
}

void STRUCTConfigNumUniqueChannels(const FalconParamEntry *params, int32_t param_count,
                                   FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Config>(params, param_count, "this");
  pack_results(FunctionResult{static_cast<int64_t>(self->num_unique_channels())}, out, 16, oc);
}

void STRUCTConfigChannels(const FalconParamEntry *params, int32_t param_count,
                          FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Config>(params, param_count, "this");
  pack_opaque_channels(self->channels(), out, oc);
}

void STRUCTConfigGetCurrentChannels(const FalconParamEntry *params, int32_t param_count,
                                    FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Config>(params, param_count, "this");
  auto chs = self->get_current_channels();
  auto arr_val = std::make_shared<ArrayValue>();
  if (chs) {
    arr_val->elements.reserve(chs->size());
    for (const auto &ch : *chs) {
      arr_val->elements.push_back(wrap_channel_as_struct(ch));
    }
  }
  pack_array_result(arr_val, out, oc);
}

void STRUCTConfigGetChannelPlungerGates(const FalconParamEntry *params, int32_t param_count,
                                        FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Config>(params, param_count, "this");
  auto ch = get_opaque<Channel>(params, param_count, "ch");
  pack_conns_as_array(self->get_channel_plunger_gates(ch), out, oc);
}

void STRUCTConfigGetChannelBarrierGates(const FalconParamEntry *params, int32_t param_count,
                                        FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Config>(params, param_count, "this");
  auto ch = get_opaque<Channel>(params, param_count, "ch");
  pack_conns_as_array(self->get_channel_barrier_gates(ch), out, oc);
}

void STRUCTConfigGetChannelReservoirGates(const FalconParamEntry *params, int32_t param_count,
                                          FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Config>(params, param_count, "this");
  auto ch = get_opaque<Channel>(params, param_count, "ch");
  pack_conns_as_array(self->get_channel_reservoir_gates(ch), out, oc);
}

void STRUCTConfigGetChannelScreeningGates(const FalconParamEntry *params, int32_t param_count,
                                          FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Config>(params, param_count, "this");
  auto ch = get_opaque<Channel>(params, param_count, "ch");
  pack_conns_as_array(self->get_channel_screening_gates(ch), out, oc);
}

void STRUCTConfigGetChannelDotGates(const FalconParamEntry *params, int32_t param_count,
                                    FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Config>(params, param_count, "this");
  auto ch = get_opaque<Channel>(params, param_count, "ch");
  pack_conns_as_array(self->get_channel_dot_gates(ch), out, oc);
}

void STRUCTConfigGetChannelGates(const FalconParamEntry *params, int32_t param_count,
                                 FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Config>(params, param_count, "this");
  auto ch = get_opaque<Channel>(params, param_count, "ch");
  pack_conns_as_array(self->get_channel_gates(ch), out, oc);
}

void STRUCTConfigGetChannelOhmics(const FalconParamEntry *params, int32_t param_count,
                                  FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Config>(params, param_count, "this");
  auto ch = get_opaque<Channel>(params, param_count, "ch");
  pack_conns_as_array(self->get_channel_ohmics(ch), out, oc);
}

void STRUCTConfigGetAssociatedOhmic(const FalconParamEntry *params, int32_t param_count,
                                    FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Config>(params, param_count, "this");
  auto res = get_opaque<Connection>(params, param_count, "reservoir_gate");
  pack_opaque_connection(self->get_associated_ohmic(res), out, oc);
}

void STRUCTConfigGetGname(const FalconParamEntry *params, int32_t param_count,
                          FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Config>(params, param_count, "this");
  auto ch = get_opaque<Channel>(params, param_count, "ch");
  pack_opaque_gname(self->get_gname(ch), out, oc);
}

void STRUCTConfigGetGroupPlungerGates(const FalconParamEntry *params, int32_t param_count,
                                      FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Config>(params, param_count, "this");
  auto gn = get_opaque<Gname>(params, param_count, "gn");
  pack_opaque_connections(self->get_group_plunger_gates(gn), out, oc);
}

void STRUCTConfigGetGroupBarrierGates(const FalconParamEntry *params, int32_t param_count,
                                      FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Config>(params, param_count, "this");
  auto gn = get_opaque<Gname>(params, param_count, "gn");
  pack_opaque_connections(self->get_group_barrier_gates(gn), out, oc);
}

void STRUCTConfigGetGroupReservoirGates(const FalconParamEntry *params, int32_t param_count,
                                        FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Config>(params, param_count, "this");
  auto gn = get_opaque<Gname>(params, param_count, "gn");
  pack_opaque_connections(self->get_group_reservoir_gates(gn), out, oc);
}

void STRUCTConfigGetGroupScreeningGates(const FalconParamEntry *params, int32_t param_count,
                                        FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Config>(params, param_count, "this");
  auto gn = get_opaque<Gname>(params, param_count, "gn");
  pack_opaque_connections(self->get_group_screening_gates(gn), out, oc);
}

void STRUCTConfigGetGroupDotGates(const FalconParamEntry *params, int32_t param_count,
                                  FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Config>(params, param_count, "this");
  auto gn = get_opaque<Gname>(params, param_count, "gn");
  pack_opaque_connections(self->get_group_dot_gates(gn), out, oc);
}

void STRUCTConfigGetGroupGates(const FalconParamEntry *params, int32_t param_count,
                               FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Config>(params, param_count, "this");
  auto gn = get_opaque<Gname>(params, param_count, "gn");
  pack_opaque_connections(self->get_group_gates(gn), out, oc);
}

void STRUCTConfigHasChannel(const FalconParamEntry *params, int32_t param_count,
                            FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Config>(params, param_count, "this");
  auto ch = get_opaque<Channel>(params, param_count, "ch");
  pack_results(FunctionResult{self->has_channel(ch)}, out, 16, oc);
}

void STRUCTConfigHasGname(const FalconParamEntry *params, int32_t param_count,
                          FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Config>(params, param_count, "this");
  auto gn = get_opaque<Gname>(params, param_count, "gn");
  pack_results(FunctionResult{self->has_gname(gn)}, out, 16, oc);
}

void STRUCTConfigGetDotNumber(const FalconParamEntry *params, int32_t param_count,
                              FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Config>(params, param_count, "this");
  auto ch = get_opaque<Channel>(params, param_count, "ch");
  pack_results(FunctionResult{static_cast<int64_t>(self->get_dot_number(ch))}, out, 16, oc);
}

void STRUCTConfigSelectGroup(const FalconParamEntry *params, int32_t param_count,
                             FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Config>(params, param_count, "this");
  auto gn = get_opaque<Gname>(params, param_count, "gn");
  pack_opaque_group(self->select_group(gn), out, oc);
}

void STRUCTConfigGetAllGnames(const FalconParamEntry *params, int32_t param_count,
                              FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Config>(params, param_count, "this");
  auto gnames = self->get_all_gnames();
  auto arr_val = std::make_shared<ArrayValue>();
  if (gnames) {
    arr_val->elements.reserve(gnames->size());
    for (const auto &gn : *gnames) {
      arr_val->elements.push_back(wrap_gname_as_struct(gn));
    }
  }
  pack_array_result(arr_val, out, oc);
}

void STRUCTConfigOhmicInChargeSensor(const FalconParamEntry *params, int32_t param_count,
                                     FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Config>(params, param_count, "this");
  auto ohmic = get_opaque<Connection>(params, param_count, "ohmic");
  pack_results(FunctionResult{self->ohmic_in_charge_sensor(ohmic)}, out, 16, oc);
}

void STRUCTConfigGetImpedance(const FalconParamEntry *params, int32_t param_count,
                              FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Config>(params, param_count, "this");
  auto conn = get_opaque<Connection>(params, param_count, "conn");
  pack_opaque_imp(self->get_impedance(conn), out, oc);
}

void STRUCTConfigVoltageConstraints(const FalconParamEntry *params, int32_t param_count,
                                    FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Config>(params, param_count, "this");
  auto vc = std::make_shared<VoltageConstraints>(
      self->adjacency(),
      self->max_safe_diff(),
      std::make_pair(self->min_bound(), self->max_bound()));
  pack_opaque_vc(vc, out, oc);
}

void STRUCTConfigWiringDC(const FalconParamEntry *params, int32_t param_count,
                          FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Config>(params, param_count, "this");
  pack_opaque_imps(self->wiring_DC(), out, oc);
}

void STRUCTConfigEqual(const FalconParamEntry *params, int32_t param_count,
                       FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Config>(params, param_count, "this");
  auto other = get_opaque<Config>(params, param_count, "other");
  pack_results(FunctionResult{*self == *other}, out, 16, oc);
}

void STRUCTConfigNotEqual(const FalconParamEntry *params, int32_t param_count,
                          FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Config>(params, param_count, "this");
  auto other = get_opaque<Config>(params, param_count, "other");
  pack_results(FunctionResult{*self != *other}, out, 16, oc);
}

void STRUCTConfigToJSON(const FalconParamEntry *params, int32_t param_count,
                        FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Config>(params, param_count, "this");
  pack_results(FunctionResult{self->to_json_string()}, out, 16, oc);
}

void STRUCTConfigFromJSON(const FalconParamEntry *params, int32_t param_count,
                          FalconResultSlot *out, int32_t *oc) {
  auto pm = unpack_params(params, param_count);
  auto json = std::get<std::string>(pm.at("json"));
  auto cfg = Config::from_json_string<Config>(json);
  pack_opaque_config(std::make_shared<Config>(*cfg), out, oc);
}

} // extern "C"
