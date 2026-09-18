#!/bin/bash
set -e

# Dedicated Mock Integration Test for Falcon Container
# Tests standalone mock-hub and std-lib hub communication inside the container

DOCKER_IMAGE="${1:-falcon:latest}"
WORKSPACE_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)"

echo "===================================================="
echo "Running Falcon Docker Mock Integration Test"
echo "Image: $DOCKER_IMAGE"
echo "Workspace: $WORKSPACE_ROOT"
echo "===================================================="

# 1. Start a clean test container mounting the playground workspace
echo "[1/3] Starting test container with mounted workspace..."
CONTAINER_ID=$(docker run -d -it -v "$WORKSPACE_ROOT:/playground" "$DOCKER_IMAGE" bash)

# Cleanup on exit
trap "echo 'Cleaning up test container...'; docker rm -f $CONTAINER_ID > /dev/null 2>&1 || true" EXIT

# 2. Ensure prerequisites (nats-server and runtime libraries if missing from base image)
echo "[2/3] Setting up container environment..."
docker exec "$CONTAINER_ID" bash -c "
  pkgs=()
  command -v nats-server &> /dev/null || pkgs+=(nats-server)
  ldconfig -p | grep -q 'libgfortran.so.5' || pkgs+=(libgfortran5)
  if [ \${#pkgs[@]} -gt 0 ]; then
    apt-get update >/dev/null 2>&1 && apt-get install -y \"\${pkgs[@]}\" >/dev/null 2>&1 || true
  fi
"

# 3. Run test suites inside container using container toolchain (/opt/falcon)
echo "[3/3] Running test suites inside container..."
docker exec "$CONTAINER_ID" bash -c "
  set -e
  export LD_LIBRARY_PATH=/opt/falcon/lib:\$LD_LIBRARY_PATH
  export PATH=/opt/falcon/bin:\$PATH

  echo '--- Step A: Running mock-hub self-tests ---'
  cd /playground/mock-hub/tests
  falcon-test ./run_tests.fal --log-level info

  echo '--- Step B: Running std-lib hub tests with mock-hub ---'
  cd /playground/std-lib/hub/tests
  falcon-test ./run_tests.fal --log-level info
"

echo ""
echo "===================================================="
echo "✅ ALL CONTAINER MOCK INTEGRATION TESTS PASSED!"
echo "===================================================="
