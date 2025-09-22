#pragma once
#include <string>
#include <thread>
#include <atomic>
#include <vector>
#include <random>

enum class ServiceType {
    WEB_SERVER,     // High CPU, moderate memory
    DATABASE,       // High memory, moderate CPU, high disk I/O
    API_GATEWAY,    // High network I/O, moderate CPU
    CACHE_SERVICE,  // High memory, low CPU
    WORKER_SERVICE  // Variable CPU based on "jobs"
};

class MockService {
public:
    MockService(const std::string& name, ServiceType type, int port);
    ~MockService();
    
    void start();
    void stop();
    bool isRunning() const { return running; }
    
    // Service info
    std::string getName() const { return service_name; }
    ServiceType getType() const { return service_type; }
    int getPort() const { return service_port; }
    int getPID() const { return getPID(); }
    
private:
    std::string service_name;
    ServiceType service_type;
    int service_port;
    std::atomic<bool> running{false};
    std::thread service_thread;
    std::mt19937 rng;
    
    // Service simulation methods
    void serviceLoop();
    void simulateWebServer();
    void simulateDatabase();
    void simulateAPIGateway();
    void simulateCacheService();
    void simulateWorkerService();
    
    // Utility methods
    void consumeCPU(int milliseconds);
    void consumeMemory(size_t bytes, int seconds);
    void simulateNetworkActivity();
    void simulateDiskActivity();
};

class ServiceManager {
public:
    ServiceManager();
    ~ServiceManager();
    
    void startAllServices();
    void stopAllServices();
    void printServiceStatus() const;
    
    std::vector<MockService*> getRunningServices() const;
    
private:
    std::vector<std::unique_ptr<MockService>> services;
};