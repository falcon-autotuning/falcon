#!/usr/bin/env bash
# =============================================================================
# Falcon / Instrument Controller Wellness Check
#
# Validates a fresh Falcon and Instrument Controller installation by:
#   1. Verifying required binaries are available.
#   2. Building mock instrument plugins.
#   3. Launching a temporary instrument-hub instance.
#   4. Running an end-to-end Falcon measurement test.
#
# Usage:
#   ./wellness-check.sh <workspace>
#
# Examples:
#   ./wellness-check.sh ~/falcon
#
#   WORKSPACE=~/falcon ./wellness-check.sh
#
# When run inside Docker, /workspace is used by default.
# =============================================================================
set -euo pipefail

WORKSPACE="${1:-${WORKSPACE:-/workspace}}"
if [[ ! -d "$WORKSPACE" ]]; then
  echo "❌ Workspace does not exist: $WORKSPACE"
  exit 1
fi
echo "Using workspace: $WORKSPACE"

if [[ -f "$WORKSPACE/test_1d_trace.fal" ]]; then
  TEST_DIR="$WORKSPACE"
elif [[ -f "$WORKSPACE/tests/integration/nested-docker/test_1d_trace.fal" ]]; then
  TEST_DIR="$WORKSPACE/tests/integration/nested-docker"
else
  echo "❌ Could not locate wellness test assets"
  exit 1
fi
echo "Using test directory: $TEST_DIR"

BUILD_DIR="$TEST_DIR/build"
mkdir -p "$BUILD_DIR" /tmp/measurements /tmp/hub-workdir

export LD_LIBRARY_PATH="/opt/falcon/lib:/opt/instrument-controller/lib:$BUILD_DIR:${LD_LIBRARY_PATH:-}"

echo "=========================================================="
echo " 🩺 Running Wellness Check on Target Environment"
echo "=========================================================="
echo "Checking instrument-controller installation..."
which instrument-hub
which instrument-script-server
which instrument-worker

echo "Checking instrument-hub version / help..."
instrument-hub --help >/dev/null 2>&1 || {
  echo "❌ instrument-hub failed to run"
  exit 1
}
instrument-script-server --help >/dev/null 2>&1 || {
  echo "❌ instrument-script-server failed to run"
  exit 1
}
echo "✅ instrument-controller installation verified!"

echo "Checking falcon installation..."
which falcon-run
which falcon-test

falcon-run --help >/dev/null 2>&1 || {
  echo "❌ falcon-run failed to run"
  exit 1
}
falcon-test --help >/dev/null 2>&1 || {
  echo "❌ falcon-test failed to run"
  exit 1
}
echo "✅ falcon installation verified!"

echo "=========================================================="
echo " (1/3) Building Mock Instrument Plugins"
echo "=========================================================="

cmake \
  -DCMAKE_FIND_DEBUG_MODE=ON \
  -S "$TEST_DIR" \
  -B "$BUILD_DIR"

cmake --build "$BUILD_DIR" -j

echo "=========================================================="
echo " (2/3) Starting the instrument-hub "
echo "=========================================================="

HUB_WORKDIR="/tmp/hub-workdir"
mkdir -p "$HUB_WORKDIR"

HUB_CONFIG="$BUILD_DIR/hub_config.yaml"

export INSTRUMENT_SCRIPT_SERVER_RPC_PORT=5555
NATS_PORT=4222
NATS_HOST=127.0.0.1

cat >"$HUB_CONFIG" <<EOF
wiremap:
  - name: S1
    instrument:
      name: Source1
      channel_group: analog
      index: 1
  - name: S2
    instrument:
      name: Source1
      channel_group: analog
      index: 2
  - name: S3
    instrument:
      name: Source1
      channel_group: analog
      index: 3
  - name: P1
    instrument:
      name: Source1
      channel_group: analog
      index: 4
  - name: P2
    instrument:
      name: Source1
      channel_group: analog
      index: 5
  - name: P3
    instrument:
      name: Source1
      channel_group: analog
      index: 6
  - name: B1
    instrument:
      name: Source1
      channel_group: analog
      index: 7
  - name: B2
    instrument:
      name: Source1
      channel_group: analog
      index: 8
  - name: B3
    instrument:
      name: Source1
      channel_group: analog
      index: 9
  - name: B4
    instrument:
      name: Source1
      channel_group: analog
      index: 10
  - name: B5
    instrument:
      name: Source1
      channel_group: analog
      index: 11
  - name: R1
    instrument:
      name: Source1
      channel_group: analog
      index: 12
  - name: R2
    instrument:
      name: Source1
      channel_group: analog
      index: 13
  - name: R3
    instrument:
      name: Source1
      channel_group: analog
      index: 14
  - name: R4
    instrument:
      name: Source1
      channel_group: analog
      index: 15
  - name: O1
    instrument:
      name: Meter1
      channel_group: analog
      index: 1
  - name: O2
    instrument:
      name: Meter1
      channel_group: analog
      index: 2
  - name: O3
    instrument:
      name: Meter1
      channel_group: analog
      index: 3
  - name: O4
    instrument:
      name: Meter1
      channel_group: analog
      index: 4

quantum-dot-config: $TEST_DIR/2-dot-1-chargesensor.yml

initial-voltage-state: []

nats:
  host: $NATS_HOST 
  port: $NATS_PORT 

database:
  owner: wellness-test

working-directory: $HUB_WORKDIR

user-measurement-luas: $TEST_DIR

instrument-server:
  rpc-port: $INSTRUMENT_SCRIPT_SERVER_RPC_PORT 
  autostart: true

  instruments:
    - config: $TEST_DIR/mock_source_config.yml
      plugin: $BUILD_DIR/mock_voltage_source.so
      type: dc_voltage_source
      characteristics:
        - identifier: source_voltage
          readCommand:
            command: GET_VOLTAGE
          writeCommand:
            command: SET_VOLTAGE

    - config: $TEST_DIR/mock_meter_config.yml
      plugin: $BUILD_DIR/mock_multimeter.so
      type: amnmeter
      characteristics:
        - identifier: sample_rate
          writeCommand:
            command: SET_SAMPLE_RATE
        - identifier: number_of_samples
          readCommand:
            command: GET_BINS
          writeCommand:
            command: SET_BINS
EOF

HUB_LOG=/tmp/hub.log

export MOCK_MULTIMETER_DATA_FILE="$TEST_DIR/gaussian-1d.txt"
if [[ ! -f "$MOCK_MULTIMETER_DATA_FILE" ]]; then
  echo "❌ Missing mock data file: $MOCK_MULTIMETER_DATA_FILE"
  exit 1
fi
echo "Using mock multimeter data file: $MOCK_MULTIMETER_DATA_FILE"

instrument-hub --config "$HUB_CONFIG" >"$HUB_LOG" 2>&1 &
HUB_PID=$!
sleep 3

cleanup() {
  echo "Cleaning up background services..."
  kill $HUB_PID 2>/dev/null || true
}
trap cleanup EXIT

if ! kill -0 $HUB_PID 2>/dev/null; then
  echo "❌ Error: Falcon Instrument Hub failed to start. Logs:"
  cat /tmp/hub.log
  exit 1
fi
echo "✅ Falcon Instrument Hub is running (pid=$HUB_PID)."

echo "=========================================================="
echo " (3/3) Launching Wellness Check"
echo "=========================================================="

export NATS_URL=nats://$NATS_HOST:$NATS_PORT

TEST_LOG=/tmp/falcon-test.log

if falcon-test "$TEST_DIR/test_1d_trace.fal" 2>&1 | tee "$TEST_LOG"; then
  echo "✅ WELLNESS CHECK COMPLETED SUCCESSFULLY!"
else
  echo "❌ falcon-test failed"
  cat $TEST_LOG
  exit 1
fi
