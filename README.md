# micro-performance-monitor
Lightweight C++ performance monitoring tool used to track microservice metrics.
key metrics are all recorded by reading directly from the '/proc' filesystem, so the tool is only compatible with Linux systems

<img width="1740" height="874" alt="image" src="https://github.com/user-attachments/assets/8435ebfe-78af-4acc-adca-cf00eb9d66a0" />

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
```
### Frontend (React)
```bash
cd monitoring-dashboard
npm install
npm start
```
