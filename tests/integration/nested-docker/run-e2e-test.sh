#!/usr/bin/env bash
# =============================================================================
# Run Large-scale End-to-End Integration Test in Ubuntu 26 Docker Container
# Acts as a target machine proxy and wellness check for instrument-controller & falcon
# =============================================================================
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
WORKSPACE_ROOT="$(cd "$SCRIPT_DIR/../../.." && pwd)"

echo "=========================================================="
echo " Building Ubuntu 26 Outer Target Container"
echo "=========================================================="
IMAGE_TAG="falcon-wellness-check:ubuntu26"

docker build \
  -t "$IMAGE_TAG" \
  -f "$SCRIPT_DIR/Dockerfile.target" \
  "$SCRIPT_DIR"

echo "=========================================================="
echo " Executing End-to-End Test & Wellness Check in Outer Container"
echo "=========================================================="
docker run --rm \
  --network host \
  -v /var/run/docker.sock:/var/run/docker.sock \
  -v "$WORKSPACE_ROOT:/workspace" \
  "$IMAGE_TAG"
# docker run --rm -it \
#   --network host \
#   -v /var/run/docker.sock:/var/run/docker.sock \
#   -v "$WORKSPACE_ROOT:/workspace" \
#   --entrypoint bash \
#   "$IMAGE_TAG"
