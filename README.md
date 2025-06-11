# User Authentication Kernel Module with JSON Export

cd user-auth-kernal && chmod +x run.sh test_json.sh auth.sh

A Linux kernel module that provides user authentication functionality and exports user data in JSON format compatible with analytics dashboards.

## Features

- **User Registration & Authentication**: Register users with password hashing
- **JSON Export**: Automatically generates structured JSON data with user statistics
- **CPU/Memory Usage Simulation**: Generates realistic usage patterns for each user
- **Security Events**: Logs authentication events in JSON format
- **Device Management**: Tracks user devices (Desktop/Mobile/Tablet)

## Files

- `passwd_module.c` - Main kernel module with JSON generation
- `auth.sh` - User authentication interface (Zenity GUI)
- `run.sh` - Build and run script with options
- `test_json.sh` - Test script for JSON generation
- `Makefile` - Build configuration

## JSON Output Format

The module generates `/etc/users.json` with the following structure:

```json
{
  "users": [
    {
      "id": 1,
      "username": "nikhil",
      "email": "nikhil@gmail.com",
      "loginTime": "2024-06-11T08:30:00Z",
      "logoutTime": "2024-06-11T17:45:00Z",
      "sessionDuration": 525,
      "device": "Desktop",
      "cpuUsage": {
        "average": 45,
        "peak": 70,
        "hourly": [42, 45, 48, ...]
      },
      "memoryUsage": {
        "average": 62,
        "peak": 82,
        "hourly": [58, 60, 62, ...]
      },
      "status": "active"
    }
  ],
  "systemMetrics": {
    "totalUsers": 192,
    "activeUsers": 4,
    "failedLogins": 14,
    "successRate": 96.2,
    ...
  },
  "securityEvents": [...]
}
```

## Usage

1. **Build and run**:
   ```bash
   chmod +x run.sh test_json.sh
   sudo ./run.sh
   ```

2. **Test JSON generation**:
   ```bash
   ./test_json.sh
   ```

3. **Manual operations**:
   ```bash
   # Build
   make
   
   # Load module
   sudo insmod passwd_module.ko
   
   # Generate JSON (reads from device)
   cat /dev/passwd_dev > output.json
   ```

## Integration

The generated JSON file can be directly used by analytics dashboards. The format is compatible with React applications using libraries like Recharts for data visualization.

## Requirements

- Linux kernel headers
- Make
- Zenity (for GUI interface)
- sudo privileges
