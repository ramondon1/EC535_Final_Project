#!/bin/bash

CONTROL_FILE="/proc/av_control"
STATUS_FILE="/proc/av_sched_status"

# Function to send commands to the kernel module
send_cmd() {
    echo "[CMD] $1"
    echo "$1" > "$CONTROL_FILE"
    sleep 1
}

# Background monitoring function
monitor_status() {
    while true; do
        clear
        echo "===== [AV Scheduler Status Monitor] ====="
        if [ -f "$STATUS_FILE" ]; then
            cat "$STATUS_FILE"
        else
            echo "Status file not available..."
        fi
        sleep 1
    done
}

# Start the status monitor in the background
monitor_status &
MONITOR_PID=$!

# Trap to kill background monitor when script exits
trap "kill $MONITOR_PID; echo 'Monitor stopped.'; exit" INT TERM

# Run simulation steps
sleep 2
send_cmd "sensor"
send_cmd "path"
send_cmd "control"
send_cmd "comm"
send_cmd "log"

send_cmd "comm"
send_cmd "log"

send_cmd "object 1"  # Trigger emergency brake

send_cmd "sensor"
send_cmd "path"
send_cmd "control"
send_cmd "comm"
send_cmd "log"

send_cmd "brake 0"  # Deactivate braking

send_cmd "sensor"
send_cmd "comm"
send_cmd "log"

# Allow final status viewing before stopping
sleep 5

# Cleanup and finish
kill $MONITOR_PID
echo "===== Simulation Finished ====="
