#include "monitor.h"
#include "mock_service.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <signal.h>

ServiceManager* global_service_manager = nullptr;
PerformanceMonitor* global_monitor = nullptr;

void signalHandler(int signum) {
    std::cout << "\n\nShutting down microservice environment..." << std::endl;
    
    if (global_service_manager) {
        global_service_manager->stopAllServices();
    }
    
    if (global_monitor) {
        global_monitor->stopHTTPServer();
    }
    
    exit(0);
}

int main() {
    ServiceManager service_manager;
    PerformanceMonitor monitor;
    
    global_service_manager = &service_manager;
    global_monitor = &monitor;
    
    // Set up signal handler for graceful shutdown
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    std::cout << "=== Microservice Performance Monitoring Demo ===" << std::endl;
    std::cout << "This demo will:" << std::endl;
    std::cout << "1. Start multiple mock microservices" << std::endl;
    std::cout << "2. Start performance monitoring HTTP server" << std::endl;
    std::cout << "3. Generate realistic load patterns" << std::endl;
    std::cout << std::endl;
    
    // Start all mock services
    service_manager.startAllServices();
    
    // Give services time to start up
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // Start performance monitoring server
    monitor.startHTTPServer(9090); // Different port to avoid conflicts
    
    std::cout << "\n=== Demo Running ===" << std::endl;
    std::cout << "Performance Monitor API: http://localhost:9090/metrics" << std::endl;
    std::cout << "Health Check: http://localhost:9090/health" << std::endl;
    std::cout << "\nMock Services Running:" << std::endl;
    service_manager.printServiceStatus();
    
    std::cout << "Try these commands:" << std::endl;
    std::cout << "  curl http://localhost:9090/metrics" << std::endl;
    std::cout << "  curl http://localhost:9090/health" << std::endl;
    std::cout << "\nPress Ctrl+C to stop all services" << std::endl;
    
    // Main monitoring loop
    int cycle = 0;
    while (monitor.isServerRunning()) {
        // Collect and display metrics every 10 seconds
        monitor.collectAllMetrics();
        
        cycle++;
        if (cycle % 2 == 0) { // Every 20 seconds
            std::cout << "\n--- Cycle " << cycle << " ---" << std::endl;
            monitor.printStats();
            service_manager.printServiceStatus();
        }
        
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }
    
    return 0;
}