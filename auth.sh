#!/bin/bash

DEVICE="/dev/passwd_dev"

function hash_password() {
    echo -n "$1" | sha256sum | awk '{print $1}'
}

function register_user() {
    user=$(zenity --entry --title="Register" --text="Enter Username:")
    
    # Early checks for username
    if [ -z "$user" ]; then
        zenity --error --text="Username cannot be empty!"
        return
    fi

    if [ ${#user} -lt 4 ]; then
        zenity --error --text="Username must be at least 4 characters long!"
        return
    fi

    if grep -q "^$user:" /etc/users.db 2>/dev/null; then
        zenity --error --text="Username already exists!"
        return
    fi

    # If username is OK, continue to password
    pass=$(zenity --password --title="Register")

    if [ -z "$pass" ]; then
        zenity --error --text="Password cannot be empty!"
        return
    fi

    if [ ${#pass} -lt 7 ]; then
        zenity --error --text="Password must be at least 7 characters long!"
        return
    fi

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

function list_users() {
    if [ ! -f /etc/users.db ]; then
        zenity --error --text="No users registered yet!"
        return
    fi

    users=$(cut -d':' -f1 /etc/users.db)
    if [ -z "$users" ]; then
        zenity --info --title="Registered Users" --text="No users found."
    else
        zenity --info --title="Registered Users" --text="$users"
    fi
}

function delete_user() {
    user=$(zenity --entry --title="Delete User" --text="Enter Username:")

    # Early check for empty username
    if [ -z "$user" ]; then
        zenity --error --text="Username cannot be empty!"
        return
    fi

    pass=$(zenity --password --title="Delete User")

    if [ -z "$pass" ]; then
        zenity --error --text="Password cannot be empty!"
        return
    fi

    entered_hash=$(hash_password "$user:$pass")

    user_info=$(grep "^$user:" /etc/users.db 2>/dev/null)
    if [ -z "$user_info" ]; then
        zenity --error --text="User not found!"
        return
    fi

    stored_hash=$(echo "$user_info" | cut -d':' -f2)

    if [ "$entered_hash" != "$stored_hash" ]; then
        zenity --error --text="Authentication Failed!"
        return
    fi

    # Authentication successful — delete the user line
    sudo sed -i "/^$user:/d" /etc/users.db

    zenity --info --text="User '$user' deleted successfully!"
}

function change_password() {
    user=$(zenity --entry --title="Change Password" --text="Enter Username:")

    if [ -z "$user" ]; then
        zenity --error --text="Username cannot be empty!"
        return
    fi

    pass=$(zenity --password --title="Change Password" --text="Enter Current Password:")
    if [ -z "$pass" ]; then
        zenity --error --text="Password cannot be empty!"
        return
    fi

    entered_hash=$(hash_password "$user:$pass")
    user_info=$(grep "^$user:" /etc/users.db 2>/dev/null)
    if [ -z "$user_info" ]; then
        zenity --error --text="User not found!"
        return
    fi

    stored_hash=$(echo "$user_info" | cut -d':' -f2)
    if [ "$entered_hash" != "$stored_hash" ]; then
        zenity --error --text="Authentication failed!"
        return
    fi

    # Prompt new password
    new_pass=$(zenity --password --title="Change Password" --text="Enter New Password:")
    if [ -z "$new_pass" ]; then
        zenity --error --text="New password cannot be empty!"
        return
    fi

    if [ ${#new_pass} -lt 7 ]; then
        zenity --error --text="New password must be at least 7 characters long!"
        return
    fi

    new_hash=$(hash_password "$user:$new_pass")

    # Replace password hash in file
    sudo sed -i "s/^$user:.*/$user:$new_hash/" /etc/users.db
    zenity --info --text="Password changed successfully!"
}



function main_menu() {
    choice=$(zenity --list --radiolist \
        --title="User Auth System" \
        --text="Choose an option:" \
        --column="Pick" --column="Option" \
        TRUE "Register" FALSE "Login" FALSE "Delete User" FALSE "Change Password" FALSE "List Users" FALSE "Exit")

    case "$choice" in
        Register)
            register_user
            ;;
        Login)
            authenticate_user
            ;;
        "Delete User")
            delete_user
            ;;
        "Change Password")
            change_password
            ;;
        "List Users")
            list_users
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
