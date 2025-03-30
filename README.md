# Smart City Traffic Light Management System

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![C Language](https://img.shields.io/badge/Language-C-blue.svg)](https://www.gnu.org/software/gnu-c-manual/)

A dynamic memory-managed traffic control system implementing:
- Real-time sensor data processing
- Memory-safe operations
- Garbage collection simulation

## Features
✅ **Dynamic Memory Management**  
- `malloc/calloc/realloc` for sensor data  
- Automatic expansion of sensor reading buffers  

✅ **Error Handling**  
- NULL pointer checks  
- Buffer overflow protection  

✅ **Task Requirements Met**  
- [x] Dynamic allocation/deallocation  
- [x] Memory leak prevention  
- [x] Orphaned data collection  

## Setup (Kali Linux)
```bash
git clone https://github.com/DenzelRensons/smart-city-traffic-system.git
cd smart-city-traffic-system
make && ./traffic_system
