#!/usr/bin/env bash
# =============================================================================
# Run Falcon In-Container Test with External C++ Mock-Hub Daemon
# =============================================================================
# The Mock-Hub server runs outside of the test container as an external daemon
# (built from the official falcon-mock-hub vcpkg package), communicating over
# NATS on port 4222.
# =============================================================================
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PORT=4222

echo "=========================================================="
echo " 1. Ensuring External Mock-Hub Daemon Image is Built"
echo "=========================================================="
docker build -t falcon-mock-hub-daemon:latest -f "$SCRIPT_DIR/Dockerfile.daemon" "$SCRIPT_DIR"

echo "=========================================================="
echo " 2. Starting External Mock-Hub Daemon (Port $PORT)"
echo "=========================================================="
CONTAINER_NAME="falcon-mock-hub-external-$$"

cleanup() {
  echo "Stopping external mock_hub_daemon container ($CONTAINER_NAME)..."
  docker rm -f "$CONTAINER_NAME" >/dev/null 2>&1 || true
}
trap cleanup EXIT

docker run -d --rm \
  --name "$CONTAINER_NAME" \
  --network host \
  falcon-mock-hub-daemon:latest "$PORT"

# Wait for server to report ready
echo "Waiting for external mock-hub daemon to be ready..."
READY=0
for i in {1..30}; do
  if docker logs "$CONTAINER_NAME" 2>&1 | grep -q "MOCK_HUB_READY"; then
    echo "MockHub server is ready!"
    READY=1
    break
  fi
  sleep 0.2
done

if [ "$READY" -ne 1 ]; then
  echo "Error: mock_hub_daemon failed to start. Logs:"
  docker logs "$CONTAINER_NAME" || true
  exit 1
fi

echo "=========================================================="
echo " 3. Running Falcon Integration Test Inside Docker Container"
echo "=========================================================="
# Mount the test file read-only, copy it into an isolated temporary WORKDIR
# inside the container, and run the test there. This prevents falcon-test from
# leaking root-owned .falcon cache artifacts into the host repository.
docker run --rm \
  --network host \
  -v "$SCRIPT_DIR/test_external_hub.fal:/test_src/test_external_hub.fal:ro" \
  falcon:latest bash -c "
    set -e
    export NATS_URL=nats://localhost:$PORT

    WORKDIR=\$(mktemp -d /tmp/falcon-external-test-XXXXXX)
    cd \"\$WORKDIR\"
    cp /test_src/test_external_hub.fal .

    falcon-pm --init . ext-hub-test >/dev/null 2>&1
    falcon-pm --install github.com/falcon-autotuning/std-lib@v1.2.3/hub >/dev/null 2>&1
    falcon-pm --install github.com/falcon-autotuning/std-lib@v1.2.3/testing >/dev/null 2>&1
    falcon-pm --install github.com/falcon-autotuning/std-lib@v1.2.3/falconCore/communications/messages/measurementRequest >/dev/null 2>&1
    falcon-pm --install github.com/falcon-autotuning/std-lib@v1.2.3/falconCore/communications/messages/measurementResponse >/dev/null 2>&1

    echo 'Running falcon-test...'
    falcon-test ./test_external_hub.fal --log-level info
    rm -rf \"\$WORKDIR\"
"

echo "=========================================================="
echo "✅ SUCCESS: Containerized Falcon successfully communicated"
echo "   with external C++ MockHub daemon over live NATS!"
echo "=========================================================="
