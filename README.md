# Autonomous Vehicle Real-Time Scheduler (Kernel Module + Simulator)

A real-time task scheduling simulation for an autonomous vehicle system using a Linux kernel module and user space simulator. Prioritizes emergency braking via real-time scheduling while simulating soft-deadline tasks like sensor scanning and path planning.

---

## Project Structure

```
.
├── vehicle_scheduling.c       # Kernel module
├── av_simulator.c             # User space simulation tool
├── Makefile                   # Kernel module Makefile
├── user/Makefile              # User space Makefile
├── README.md
```

---

## Features

- Emergency Braking with hard real-time deadlines
- Multi-threaded task simulation for AV subsystems
- Deadline tracking and statistics via `/proc`
- Event-driven scheduling using wait queues
- User space control tool to simulate inputs dynamically

---

## Requirements

- Linux system with kernel headers installed  
- ARM cross-compilation tools (`arm-linux-gnueabihf-gcc`)
- Make
- Root access to load/unload modules

---

## Build Instructions

### 1. Build the Kernel Module

```bash
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf-
```

### 2. Build the User Space Program

```bash
cd user
make CC=arm-linux-gnueabihf-gcc
```

---

## Usage

### 1. Insert the Kernel Module

```bash
sudo insmod av_sched.ko
```

### 2. Start the Simulator

```bash
./av_simulator
```

You will be prompted with a menu like:

```
Select an input:
1 - Obstacle Detected (Trigger Braking)
2 - Clear Obstacle (Disable Braking)
3 - Start Sensor Task
4 - Stop Sensor Task
5 - Get Status
6 - Exit
```

### 3. Check Kernel Logs

```bash
dmesg | tail -n 50
```

### 4. Check Task Status

```bash
cat /proc/av_sched_status
```

### 5. Remove the Kernel Module

```bash
sudo rmmod av_sched
```

---

## System Overview

**Tasks Simulated:**

- Emergency Braking (hard deadline)
- Sensor Data Collection
- Path Planning
- Vehicle Control
- Communication
- Data Logging

**Communication Interface:**

- `/proc/av_sched_control`: Accepts user space commands  
- `/proc/av_sched_status`: Displays task activity and timing stats
