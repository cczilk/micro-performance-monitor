#include "mock_service.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <fstream>
#include <unistd.h>
#include <cmath>

MockService::MockService(const std::string& name, ServiceType type, int port) 
    : service_name(name), service_type(type), service_port(port), rng(std::random_device{}()) {
}

MockService::~MockService() {
    stop();
}

void MockService::start() {
    if (running) return;
    
    running = true;
    service_thread = std::thread(&MockService::serviceLoop, this);
    std::cout << "Started " << service_name << " on port " << service_port << std::endl;
}

void MockService::stop() {
    if (!running) return;
    
    running = false;
    if (service_thread.joinable()) {
        service_thread.join();
    }
    std::cout << "Stopped " << service_name << std::endl;
}

void MockService::serviceLoop() {
    while (running) {
        switch (service_type) {
            case ServiceType::WEB_SERVER:
                simulateWebServer();
                break;
            case ServiceType::DATABASE:
                simulateDatabase();
                break;
            case ServiceType::API_GATEWAY:
                simulateAPIGateway();
                break;
            case ServiceType::CACHE_SERVICE:
                simulateCacheService();
                break;
            case ServiceType::WORKER_SERVICE:
                simulateWorkerService();
                break;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void MockService::simulateWebServer() {
    // Simulate handling HTTP requests - bursts of CPU activity
    std::uniform_int_distribution<> request_dist(1, 10);
    int requests = request_dist(rng);
    
    for (int i = 0; i < requests && running; i++) {
        // Each request uses some CPU
        consumeCPU(50); // 50ms of CPU work
        
        // Random sleep between requests
        std::uniform_int_distribution<> sleep_dist(10, 100);
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep_dist(rng)));
    }
}

void MockService::simulateDatabase() {
    // Simulate database operations - memory intensive with disk I/O
    std::uniform_int_distribution<> operation_dist(1, 3);
    int operation = operation_dist(rng);
    
    switch (operation) {
        case 1: // Read operation
            consumeCPU(30);
            simulateDiskActivity();
            break;
        case 2: // Write operation  
            consumeCPU(50);
            simulateDiskActivity();
            consumeMemory(1024 * 1024, 2); // 1MB for 2 seconds
            break;
        case 3: // Query optimization
            consumeCPU(200); // Heavy CPU work
            consumeMemory(5 * 1024 * 1024, 3); // 5MB for 3 seconds
            break;
    }
}

void MockService::simulateAPIGateway() {
    // Simulate API gateway - network heavy, request routing
    std::uniform_int_distribution<> request_dist(5, 20);
    int requests = request_dist(rng);
    
    for (int i = 0; i < requests && running; i++) {
        // Route request - some CPU + network
        consumeCPU(20);
        simulateNetworkActivity();
        
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

void MockService::simulateCacheService() {
    // Simulate cache operations - memory heavy, low CPU
    std::uniform_int_distribution<> operation_dist(1, 4);
    int operation = operation_dist(rng);
    
    if (operation == 1) {
        // Cache miss - need to populate
        consumeCPU(100);
        consumeMemory(2 * 1024 * 1024, 5); // 2MB for 5 seconds
    } else {
        // Cache hit - just memory access
        consumeCPU(10);
    }
}

void MockService::simulateWorkerService() {
    // Simulate background worker - variable load based on "job queue"
    std::uniform_int_distribution<> job_dist(0, 5);
    int jobs = job_dist(rng);
    
    for (int i = 0; i < jobs && running; i++) {
        // Process a job
        std::uniform_int_distribution<> work_dist(100, 500);
        consumeCPU(work_dist(rng));
        
        // Some jobs need memory
        if (i % 2 == 0) {
            consumeMemory(512 * 1024, 1); // 512KB for 1 second
        }
    }
    
    // Longer sleep if no jobs
    if (jobs == 0) {
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}

void MockService::consumeCPU(int milliseconds) {
    auto start = std::chrono::high_resolution_clock::now();
    auto end = start + std::chrono::milliseconds(milliseconds);
    
    // Busy loop to consume CPU
    volatile double result = 0;
    while (std::chrono::high_resolution_clock::now() < end && running) {
        for (int i = 0; i < 10000; i++) {
            result += std::sin(i) * std::cos(i);
        }
    }
}

void MockService::consumeMemory(size_t bytes, int seconds) {
    // Allocate memory and hold it
    std::vector<char> memory_hog(bytes);
    
    // Fill with some data to ensure it's actually allocated
    for (size_t i = 0; i < bytes && running; i += 4096) {
        memory_hog[i] = static_cast<char>(i % 256);
    }
    
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
}

void MockService::simulateNetworkActivity() {
    // Simulate network I/O by sleeping (as if waiting for network)
    std::uniform_int_distribution<> latency_dist(10, 100);
    std::this_thread::sleep_for(std::chrono::milliseconds(latency_dist(rng)));
}

void MockService::simulateDiskActivity() {
    // Simulate disk I/O by creating temporary file operations
    std::string temp_file = "/tmp/mock_service_" + service_name + ".tmp";
    
    // Write some data
    std::ofstream file(temp_file);
    if (file.is_open()) {
        for (int i = 0; i < 1000 && running; i++) {
            file << "Mock data line " << i << std::endl;
        }
        file.close();
    }
    
    // Read it back
    std::ifstream read_file(temp_file);
    std::string line;
    while (std::getline(read_file, line) && running) {
        // Just read, don't do anything
    }
    read_file.close();
    
    // Clean up
    std::remove(temp_file.c_str());
}

// ServiceManager Implementation
ServiceManager::ServiceManager() {
    // Create a realistic microservice setup
    services.push_back(std::make_unique<MockService>("web-frontend", ServiceType::WEB_SERVER, 3000));
    services.push_back(std::make_unique<MockService>("api-gateway", ServiceType::API_GATEWAY, 8080));
    services.push_back(std::make_unique<MockService>("user-service", ServiceType::WEB_SERVER, 8081));
    services.push_back(std::make_unique<MockService>("product-service", ServiceType::WEB_SERVER, 8082));
    services.push_back(std::make_unique<MockService>("postgres-db", ServiceType::DATABASE, 5432));
    services.push_back(std::make_unique<MockService>("redis-cache", ServiceType::CACHE_SERVICE, 6379));
    services.push_back(std::make_unique<MockService>("background-worker", ServiceType::WORKER_SERVICE, 0));
}

ServiceManager::~ServiceManager() {
    stopAllServices();
}

void ServiceManager::startAllServices() {
    std::cout << "Starting microservice environment..." << std::endl;
    for (auto& service : services) {
        service->start();
        std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Stagger startup
    }
    std::cout << "All services started!" << std::endl;
}

void ServiceManager::stopAllServices() {
    std::cout << "Stopping all services..." << std::endl;
    for (auto& service : services) {
        service->stop();
    }
}

void ServiceManager::printServiceStatus() const {
    std::cout << "\n=== Service Status ===" << std::endl;
    for (const auto& service : services) {
        std::cout << service->getName() << " (port " << service->getPort() 
                  << ") - " << (service->isRunning() ? "RUNNING" : "STOPPED") << std::endl;
    }
    std::cout << std::endl;
}

std::vector<MockService*> ServiceManager::getRunningServices() const {
    std::vector<MockService*> running_services;
    for (const auto& service : services) {
        if (service->isRunning()) {
            running_services.push_back(service.get());
        }
    }
    return running_services;
}