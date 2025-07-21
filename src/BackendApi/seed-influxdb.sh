#!/bin/sh
set -e
apk add --no-cache curl coreutils

INFLUX_URL="http://influxdb:8086"
INFLUX_TOKEN="${INFLUXDB_ADMIN_TOKEN:-adminadmin}"
INFLUX_ORG="${INFLUXDB_ORG:-weatherstation}"
INFLUX_BUCKET="${INFLUXDB_BUCKET:-weather}"

# EUIs
declare_bash() {
  export BALKON="0004A30B001FB02B"
  export WOHNZIMMER="0004A30B001FDC0B"
  export SIMON="A84041000181854C"
  export SCHLAFZIMMER="0025CA0A00000476"
  export BADEZIMMER="0004A30B002240C2"
}
declare_bash

EUIS="$BALKON $WOHNZIMMER $SIMON $SCHLAFZIMMER $BADEZIMMER"

# 1. Lösche alle Daten im Bucket
curl -X POST "$INFLUX_URL/api/v2/delete?org=$INFLUX_ORG&bucket=$INFLUX_BUCKET" \
  -H "Authorization: Token $INFLUX_TOKEN" \
  -H "Content-Type: application/json" \
  --data '{"start":"1970-01-01T00:00:00Z","stop":"'$(date -u +"%Y-%m-%dT%H:%M:%SZ")'"}'

# 2. Schreibe Seed-Daten für die letzten 48h
for h in $(seq 48 -1 0); do
  timestamp=$(date -u -d "$h hours ago" +"%Y-%m-%dT%H:00:00Z")
  ts_ns=$(( $(date -d "$timestamp" +%s) * 1000000000 ))
  
  # Unterschiedlicher Seed für jeden Zeitpunkt
  seed=$(( $(date +%s) + h ))
  
  for eui in $EUIS; do
    # Unterschiedlicher Seed pro EUI
    eui_seed=$(( seed + $(echo "$eui" | od -An -N4 -tu4) ))
    
    temp=$(awk -v min=16 -v max=25 -v seed="$eui_seed" 'BEGIN{srand(seed); print min+rand()*(max-min)}')
    hum=$(awk -v min=30 -v max=60 -v seed="$eui_seed" 'BEGIN{srand(seed+1); print int(min+rand()*(max-min+1))}')
    batt=$(awk -v min=3.2 -v max=4.2 -v seed="$eui_seed" 'BEGIN{srand(seed+2); printf "%.2f", min+rand()*(max-min)}')
    
    line="weather,dev_eui=$eui Temperature=$temp,Humidity=$hum,Battery=$batt $ts_ns"
    curl -s -X POST "$INFLUX_URL/api/v2/write?org=$INFLUX_ORG&bucket=$INFLUX_BUCKET&precision=ns" \
      -H "Authorization: Token $INFLUX_TOKEN" \
      --data-raw "$line"
  done
done

echo "InfluxDB Seed erfolgreich!"