#include "falcon-core/communications/HDF5Data.hpp"
#include <falcon-core/CerealRegistry.hpp>
#include <falcon-typing/FFIHelpers.hpp>

using namespace falcon::typing;
using namespace falcon::typing::ffi::wrapper;

using HDF5Data   = falcon_core::communications::HDF5Data;
using HDF5DataSP = std::shared_ptr<HDF5Data>;

// ── pack helper ───────────────────────────────────────────────────────────────

static void pack_hdf5(HDF5DataSP obj, FalconResultSlot *out, int32_t *oc) {
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "HDF5Data";
  out[0].value.opaque.ptr       = new HDF5DataSP(std::move(obj));
  out[0].value.opaque.deleter   = [](void *p) {
    delete static_cast<HDF5DataSP *>(p);
  };
  *oc = 1;
}

extern "C" {

void SampleJSON(const FalconParamEntry * /*params*/, int32_t /*param_count*/,
                FalconResultSlot *out, int32_t *oc) {
  auto shape = std::make_shared<falcon_core::math::Axes<int>>();
  auto unit_domain =
      std::make_shared<falcon_core::math::Axes<falcon_core::math::arrays::ControlArray>>();
  auto domain_labels =
      std::make_shared<falcon_core::math::Axes<falcon_core::math::domains::CoupledLabelledDomain>>();
  auto ranges =
      std::make_shared<falcon_core::math::arrays::LabelledArrays<falcon_core::math::arrays::LabelledMeasuredArray>>();
  auto metadata =
      std::make_shared<falcon_core::generic::Map<std::string, std::string>>();

  HDF5Data obj(shape, unit_domain, domain_labels, ranges, metadata,
               "test_title", 123, 456);
  pack_results(FunctionResult{obj.to_json_string()}, out, 16, oc);
}

// New(...) -> (HDF5Data data)
// The full constructor requires complex Axes types from the FAL layer; this
// stub is provided for completeness — tests use FromJSON instead.
void STRUCTHDF5DataNew(const FalconParamEntry * /*params*/,
                       int32_t /*param_count*/, FalconResultSlot *out,
                       int32_t *oc) {
  throw std::runtime_error(
      "HDF5Data.New: use FromJSON or FromCommunications to construct");
  (void)out; (void)oc;
}

// FromCommunications(request, response, states, uuid, title, uniqueID, ts)
// -> (HDF5Data data)
void STRUCTHDF5DataFromCommunications(const FalconParamEntry * /*params*/,
                                       int32_t /*param_count*/,
                                       FalconResultSlot *out, int32_t *oc) {
  throw std::runtime_error("HDF5Data.FromCommunications: not yet implemented");
  (void)out; (void)oc;
}

// ── Accessors ─────────────────────────────────────────────────────────────────

// MeasurementTitle(this: HDF5Data) -> (string title)
void STRUCTHDF5DataMeasurementTitle(const FalconParamEntry *params,
                                     int32_t param_count, FalconResultSlot *out,
                                     int32_t *oc) {
  auto self = get_opaque<HDF5Data>(params, param_count, "this");
  pack_results(FunctionResult{self->measurement_title()}, out, 16, oc);
}

// UniqueID(this: HDF5Data) -> (int id)
void STRUCTHDF5DataUniqueID(const FalconParamEntry *params, int32_t param_count,
                              FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<HDF5Data>(params, param_count, "this");
  pack_results(FunctionResult{static_cast<int64_t>(self->unique_id())}, out, 16,
               oc);
}

// Timestamp(this: HDF5Data) -> (int time)
void STRUCTHDF5DataTimestamp(const FalconParamEntry *params,
                              int32_t param_count, FalconResultSlot *out,
                              int32_t *oc) {
  auto self = get_opaque<HDF5Data>(params, param_count, "this");
  pack_results(FunctionResult{static_cast<int64_t>(self->timestamp())}, out, 16,
               oc);
}

// Shape(this: HDF5Data) -> (Axes<int> shape)
void STRUCTHDF5DataShape(const FalconParamEntry *params, int32_t param_count,
                          FalconResultSlot *out, int32_t *oc) {
  // Returns as NIL — Axes<T> is a FAL struct that cannot be trivially repacked
  (void)params; (void)param_count;
  out[0] = {}; out[0].tag = FALCON_TYPE_NIL; *oc = 1;
}

// UnitDomain(this: HDF5Data) -> (Axes<ControlArray> domain)
void STRUCTHDF5DataUnitDomain(const FalconParamEntry *params,
                               int32_t param_count, FalconResultSlot *out,
                               int32_t *oc) {
  (void)params; (void)param_count;
  out[0] = {}; out[0].tag = FALCON_TYPE_NIL; *oc = 1;
}

// DomainLabels(this: HDF5Data) -> (Axes<CoupledLabelledDomain> labels)
void STRUCTHDF5DataDomainLabels(const FalconParamEntry *params,
                                 int32_t param_count, FalconResultSlot *out,
                                 int32_t *oc) {
  (void)params; (void)param_count;
  out[0] = {}; out[0].tag = FALCON_TYPE_NIL; *oc = 1;
}

// Ranges(this: HDF5Data) -> (LabelledArrays<LabelledMeasuredArray> ranges)
void STRUCTHDF5DataRanges(const FalconParamEntry *params, int32_t param_count,
                           FalconResultSlot *out, int32_t *oc) {
  (void)params; (void)param_count;
  out[0] = {}; out[0].tag = FALCON_TYPE_NIL; *oc = 1;
}

// Metadata(this: HDF5Data) -> (Map<string,string> metadata)
void STRUCTHDF5DataMetadata(const FalconParamEntry *params, int32_t param_count,
                             FalconResultSlot *out, int32_t *oc) {
  (void)params; (void)param_count;
  out[0] = {}; out[0].tag = FALCON_TYPE_NIL; *oc = 1;
}

// ── Equality ──────────────────────────────────────────────────────────────────

// Equal(this: HDF5Data, other: HDF5Data) -> (bool equal)
void STRUCTHDF5DataEqual(const FalconParamEntry *params, int32_t param_count,
                          FalconResultSlot *out, int32_t *oc) {
  auto self  = get_opaque<HDF5Data>(params, param_count, "this");
  auto other = get_opaque<HDF5Data>(params, param_count, "other");
  pack_results(FunctionResult{*self == *other}, out, 16, oc);
}

// NotEqual(this: HDF5Data, other: HDF5Data) -> (bool notequal)
void STRUCTHDF5DataNotEqual(const FalconParamEntry *params, int32_t param_count,
                             FalconResultSlot *out, int32_t *oc) {
  auto self  = get_opaque<HDF5Data>(params, param_count, "this");
  auto other = get_opaque<HDF5Data>(params, param_count, "other");
  pack_results(FunctionResult{!(*self == *other)}, out, 16, oc);
}

// ── JSON ──────────────────────────────────────────────────────────────────────

// ToJSON(this: HDF5Data) -> (string json)
void STRUCTHDF5DataToJSON(const FalconParamEntry *params, int32_t param_count,
                           FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<HDF5Data>(params, param_count, "this");
  pack_results(FunctionResult{self->to_json_string()}, out, 16, oc);
}

// FromJSON(json: string) -> (HDF5Data data)
void STRUCTHDF5DataFromJSON(const FalconParamEntry *params, int32_t param_count,
                             FalconResultSlot *out, int32_t *oc) {
  auto pm   = unpack_params(params, param_count);
  auto json = std::get<std::string>(pm.at("json"));
  auto obj  = HDF5Data::from_json_string<HDF5Data>(json);
  pack_hdf5(obj, out, oc);
}

} // extern "C"
