# Hub Module (`hub`)

The `hub` package provides standard library bindings for interacting with the Falcon Instrument Hub and caching device configurations, voltages, and relations in the local database.

---

## ⚠️ Prerequisites

The `hub` package connects to the Falcon Instrument Hub (via NATS) and the local device cache (via PostgreSQL).

- **`FALCON_DATABASE_URL`**: PostgreSQL connection string for device state caching.
- **`NATS_URL`**: NATS message broker URL for real-time instrument hub communication.

---

## Functions

- `RequestDeviceState(timeout_ms)`: Query current device state from the instrument hub.
- `RequestMeasurement(req, timeout_ms)`: Send a measurement request to the instrument hub.
- `RequestConfig(timeout_ms)`: Request the active device configuration.
- `RequestPortPayload(timeout_ms)`: Request current port assignments (knobs, meters).
- `CacheDeviceVoltages(voltages)`: Cache voltage states locally in the database.
- `ReadDeviceVoltages(timeout_ms)`: Read cached voltages (or query hub if not cached).
- `CacheConfig(config)`: Cache device configuration locally in the database.
- `ReadConfig(timeout_ms)`: Read cached configuration (or query hub if not cached).
- `GetOhmicsConnectedToVoltageSources(timeout_ms)`: Retrieve ohmics connected to active voltage sources.
- `GetVoltageBounds(search_domain, timeout_ms)`: Compute maximal safe voltage bounds.
- `SafeVoltageChange(proposed_voltages, timeout_ms)`: Verify if a voltage change satisfies safety constraints.
- `Ramp(end_point, max_ramp_rate, timeout_ms)`: Execute a ramp measurement to a target point.
