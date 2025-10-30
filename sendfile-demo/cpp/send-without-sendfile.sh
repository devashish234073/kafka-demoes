#!/usr/bin/env bash
# Simple aggregate timing for netcat-based send
# Usage: ./send-without-sendfile.sh [count]

COUNT=${1:-40}
FILE="testfile.bin"
HOST="127.0.0.1"
PORT="8000"

if [ ! -f "$FILE" ]; then
    echo "File $FILE not found" >&2
    exit 1
fi

# Query receiver's monotonic time endpoint
get_time() {
    # returns floating seconds or empty on failure
    curl -s --max-time 5 http://127.0.0.1:8001/time | sed -E 's/.*"time"[[:space:]]*:[[:space:]]*([0-9.]+).*/\1/'
}

start=$(get_time)
if [ -z "$start" ]; then
    echo "Warning: failed to get start time from receiver; aborting" >&2
    exit 1
fi

for i in $(seq 1 $COUNT); do
    cat "$FILE" | nc -q 0 "$HOST" "$PORT"
done

end=$(get_time)
if [ -z "$end" ]; then
    echo "Warning: failed to get end time from receiver; aborting" >&2
    exit 1
fi

elapsed=$(echo "$end - $start" | bc -l)
average=$(echo "scale=9; $elapsed / $COUNT" | bc -l)

echo "Total elapsed: $elapsed seconds for $COUNT runs"
echo "Average time: $average seconds"