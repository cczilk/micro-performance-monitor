# micro-performance-monitor
Lightweight C++ performance monitoring tool used to track microservice metrics.
key metrics are all recorded by reading directly from the '/proc' filesystem, so the tool is only compatible with Linux systems

---

## Features

- Run multiple mock microservices (web, API, database, cache, worker)  
- Monitor CPU, memory, and load patterns  
- HTTP API for metrics (`/metrics`) and health (`/health`)  
- React frontend for real-time visualization  

---

## Quick Start

### Backend (C++)
```bash
make
./microservice_demo
