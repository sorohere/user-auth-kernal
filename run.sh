#!/bin/bash

echo "User Auth Kernel Module - Build and Run"
echo "========================================"

# Clean previous builds
echo "Cleaning previous builds..."
make clean
sudo rm -rf /etc/users.db /etc/users.json

# Remove module if loaded
echo "Removing existing module (if loaded)..."
sudo rmmod passwd_module 2>/dev/null || true

# Build module
echo "Building kernel module..."
make
if [ $? -ne 0 ]; then
    echo "Build failed!"
    exit 1
fi

# Load module
echo "Loading kernel module..."
sudo insmod passwd_module.ko
if [ $? -ne 0 ]; then
    echo "Failed to load module!"
    exit 1
fi

echo "Module loaded successfully!"
echo "Choose an option:"
echo "1. Run authentication interface (auth.sh)"
echo "2. Test JSON generation (test_json.sh)"
echo "3. Both"

read -p "Enter choice (1/2/3): " choice

case $choice in
    1)
        echo "Starting authentication interface..."
        sudo bash ./auth.sh
        ;;
    2)
        echo "Testing JSON generation..."
        bash ./test_json.sh
        ;;
    3)
        echo "Testing JSON generation first..."
        bash ./test_json.sh
        echo ""
        echo "Now starting authentication interface..."
        sudo bash ./auth.sh
        ;;
    *)
        echo "Invalid choice. Running authentication interface by default..."
        sudo bash ./auth.sh
        ;;
esac

echo ""
echo "Final status:"
echo "============="
if [ -f /etc/users.json ]; then
    echo "✓ JSON file generated at /etc/users.json"
    ls -la /etc/users.json
else
    echo "✗ No JSON file found"
fi

if [ -f /etc/users.db ]; then
    echo "✓ User database exists at /etc/users.db"
    echo "Users in database:"
    sudo cat /etc/users.db | cut -d':' -f1
else
    echo "✗ No user database found"
fi
