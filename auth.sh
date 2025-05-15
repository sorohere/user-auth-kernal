#!/bin/bash

DEVICE="/dev/passwd_dev"

function hash_password() {
    echo -n "$1" | sha256sum | awk '{print $1}'
}

function register_user() {
    user=$(zenity --entry --title="Register" --text="Enter Username:")
    pass=$(zenity --password --title="Register")
    hash=$(hash_password "$user:$pass")

    echo -n "$user:$hash" > "$DEVICE"
    zenity --info --text="User Registered!"
}

function authenticate_user() {
    user=$(zenity --entry --title="Login" --text="Enter Username:")
    pass=$(zenity --password --title="Login")
    entered_hash=$(hash_password "$user:$pass")

    # Read the user DB file to validate
    user_info=$(cat /etc/users.db | grep "^$user:")
    
    if [[ $user_info =~ $user:([a-zA-Z0-9]+) ]]; then
        stored_hash="${BASH_REMATCH[1]}"
        if [ "$entered_hash" == "$stored_hash" ]; then
            zenity --info --text="Authentication Successful"
        else
            zenity --error --text="Authentication Failed"
        fi
    else
        zenity --error --text="User not found"
    fi
}

function main_menu() {
    choice=$(zenity --list --radiolist \
        --title="User Auth System" \
        --text="Choose an option:" \
        --column="Pick" --column="Option" \
        TRUE "Register" FALSE "Login" FALSE "Exit")

    case "$choice" in
        Register)
            register_user
            ;;
        Login)
            authenticate_user
            ;;
        Exit)
            exit 0
            ;;
    esac
}

# Ensure device file exists
if [ ! -e "$DEVICE" ]; then
    sudo mknod $DEVICE c 91 0
    sudo chmod 666 $DEVICE
fi

while true; do
    main_menu
done
