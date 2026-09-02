#!/bin/bash
set -e

# Bootstrap /opt/falcon if it's empty (e.g. first run with a host mount)
if [ -z "$(ls -A /opt/falcon 2>/dev/null)" ]; then
  echo "Bootstrapping /opt/falcon from seed..."
  cp -r /opt/falcon-seed/* /opt/falcon/
fi

# If the secure config volume is mounted, load the database URL
if [ -f /config/db.env ]; then
  source /config/db.env
  export FALCON_DATABASE_URL="$FALCON_DB_URL"
fi

# Execute the user's command
exec "$@"
