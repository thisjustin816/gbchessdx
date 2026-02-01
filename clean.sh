#!/bin/bash

# Check if file exists
if [ ! -f "$1" ]; then
    echo "Usage: $0 <filename.c>"
    exit 1
fi

FILE=$1

echo "Processing $FILE..."

# Remove numbers from numbered comments
# This regex looks for // followed by any non-digit chars, 
# then a number and a dot, and removes the number and dot.
sed -E -i 's/(\/\/.*)[0-9]+\.\s*(.*)/\1\2/' "$FILE"

# Run indent with the specified requirements
# -kr: K&R style
# -brf: Put braces on function definition line
# -nut: Use spaces instead of tabs (no tabs)
# -l0: Set max line length to 0 (ignore wrapping limits)
# -i4: Standard 4-space indent (optional, adjust as needed)
indent -kr -brf -nut -l0 "$FILE"

echo "Done! Cleanup and formatting complete."
