#!/bin/bash

# Set the current directory to the script's location
CURRENT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Create an output directory for the executables
OUTPUT_DIR="$CURRENT_DIR/bin"
mkdir -p "$OUTPUT_DIR"

# Compile the client
gcc -I"$CURRENT_DIR" "$CURRENT_DIR/client.c" -o "$OUTPUT_DIR/client" -lm

# Compile the server with the digSessControl
gcc -I"$CURRENT_DIR" "$CURRENT_DIR/server.c" "$CURRENT_DIR/services/udsDigCon/digSessControl.c" "$CURRENT_DIR/services/udsEcuReset/ecuReset.c" -o "$OUTPUT_DIR/server" -lm



echo "Compilation process completed!"
