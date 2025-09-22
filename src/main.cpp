#include "monitor.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <signal.h>

PerformanceMonitor* global_monitor = nullptr;

void signalHandler(int signum) {
    std::cout << "\nShutting down server..." << std::endl;
    if (global_monitor) {
        global_monitor->stopHTTPServer();
    }
    exit(0);
}

int main() {
    PerformanceMonitor monitor;
    global_monitor = &monitor;
    
    // signal handler for shutdown
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    std::cout << "=== Microservice Performance Monitor ===" << std::endl;
    
    monitor.startHTTPServer(8080);
    
    // keep main thread alive to monitor metrics
    while (monitor.isServerRunning()) {
        // collect metrics every 5 seconds for interal usage
        monitor.collectAllMetrics();
        
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
    
    return 0;
}