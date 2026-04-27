#!/bin/bash

# Find project root (upwards search for falcon.yml)
search_dir="$(pwd)"
project_root=""
while [[ "$search_dir" != "/" ]]; do
  if [[ -f "$search_dir/falcon.yml" ]]; then
    project_root="$search_dir"
    break
  fi
  search_dir="$(dirname "$search_dir")"
done

if [[ -z "$project_root" ]]; then
  project_root="$(pwd)"
fi

rel_path=$(python3 -c "import os; print(os.path.relpath('$(pwd)', '$project_root'))")

# Ensure host persistence directory exists
mkdir -p "$HOME/.falcon/opt"

docker run -it --rm \
  -v "$project_root:/workspace" \
  -v "$HOME/.falcon/opt:/opt/falcon" \
  -v falcon-config:/config:ro \
  -u "$(id -u):$(id -g)" \
  -w "/workspace/$rel_path" \
  -e FALCON_DATABASE_URL="$FALCON_DATABASE_URL" \
  ghcr.io/falcon-autotuning/falcon:1.1.0 falcon-test "$@"
