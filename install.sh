# !/bin/sh

ARDUINO_LIBRARIES="$HOME/Arduino/libraries"

if [ ! -d $ARDUINO_LIBRARIES ]; then
    echo "Failed to find Arduino libraries"
    exit 1
fi

ln -s $1 $ARDUINO_LIBRARIES