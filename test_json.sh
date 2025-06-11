#!/bin/bash

DEVICE="/dev/passwd_dev"
JSON_FILE="/etc/users.json"

echo "Testing User Auth Kernel Module with JSON Output"
echo "================================================"

# Check if module is loaded
if ! lsmod | grep -q passwd_module; then
    echo "Module not loaded. Loading..."
    sudo insmod passwd_module.ko
    if [ $? -ne 0 ]; then
        echo "Failed to load module. Make sure it's compiled."
        exit 1
    fi
fi

# Ensure device file exists
if [ ! -e "$DEVICE" ]; then
    echo "Creating device file..."
    sudo mknod $DEVICE c 91 0
    sudo chmod 666 $DEVICE
fi

echo "Adding some test users to generate JSON data..."

# Add some test users (this will also trigger JSON generation)
echo -n "nikhil:$(echo -n 'nikhil:password123' | sha256sum | awk '{print $1}')" > "$DEVICE"
echo -n "saurabh:$(echo -n 'saurabh:mypass456' | sha256sum | awk '{print $1}')" > "$DEVICE"
echo -n "sachin:$(echo -n 'sachin:secure789' | sha256sum | awk '{print $1}')" > "$DEVICE"
echo -n "virat:$(echo -n 'virat:virat2024' | sha256sum | awk '{print $1}')" > "$DEVICE"

echo "Users added. Triggering JSON generation..."

# Read from device to generate JSON
cat "$DEVICE" > /tmp/kernel_json_output.json

echo "JSON generated! Here's the output:"
echo "=================================="

if [ -f "$JSON_FILE" ]; then
    echo "JSON file created at: $JSON_FILE"
    echo "Content preview:"
    head -20 "$JSON_FILE"
    echo "..."
    echo "(Full JSON saved to $JSON_FILE)"
    
    # Copy to current directory for easier access
    sudo cp "$JSON_FILE" ./users_from_kernel.json
    sudo chown $USER:$USER ./users_from_kernel.json
    echo "Copy saved as: ./users_from_kernel.json"
else
    echo "JSON file not found at $JSON_FILE"
    echo "Checking kernel output..."
    cat /tmp/kernel_json_output.json
fi

echo ""
echo "Kernel log messages:"
dmesg | tail -10

echo ""
echo "User database content:"
if [ -f /etc/users.db ]; then
    sudo cat /etc/users.db
else
    echo "No user database found."
fi

echo ""
echo "Test complete!" 