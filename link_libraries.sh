#!/bin/bash

# Create a link to libraries in the Arduino path.

ARDUINO_LIBRARIES="$HOME/Arduino/libraries"
LIBRARIES=("PandionTC" "AtaLogger")

if [ ! -d $ARDUINO_LIBRARIES ]; then
    echo "Failed to find Arduino libraries"
    exit 1
fi

PATHCMD=$(realpath ./libraries/$PANDION)

for library in ${LIBRARIES[@]}; do
    if [ -e "$ARDUINO_LIBRARIES/$library" ]; then
        echo "$library already linked in arduino path."
    else
        echo "Linking $(realpath ./libraries/$library) to $ARDUINO_LIBRARIES/$library"
        ln -s $(realpath ./libraries/$library) $ARDUINO_LIBRARIES/$library
    fi

done