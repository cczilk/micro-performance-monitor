#pragma once
#include <string>
#include <chrono>
#include <atomic>
#include <thread>

struct NetworkStats {
    size_t bytes_sent = 0;
    size_t bytes_received = 0;
};

struct DiskStats {
    size_t bytes_read = 0;
    size_t bytes_written = 0;
};

class PerformanceMonitor {
public:
    // Existing methods
    void collectCPUUsage();
    void collectMemoryUsage();
    
    // Phase 1 expansions
    void collectNetworkStats();
    void collectDiskStats();
    void collectProcessCount();
    void collectLoadAverage();
    
    // Phase 2: Data export
    void printStats() const;
    std::string toJSON() const;
    void saveToFile(const std::string& filename) const;
    void appendToCSV(const std::string& filename) const;
    
    // Collect all metrics at once
    void collectAllMetrics();

    // Phase 3
    void startHTTPServer(int port = 8080);
    void stopHTTPServer();
    bool isServerRunning() const;
    
private:
    // Existing CPU data
    double cpu_usage = 0.0;
    long prev_total_time = 0;
    long prev_active_time = 0;
    bool first_cpu_read = true;
    
    // Existing memory data
    size_t memory_usage = 0;  // in KB
    size_t total_memory = 0;  // in KB
    
    // New metrics
    NetworkStats network_stats;
    DiskStats disk_stats;
    int process_count = 0;
    double load_average_1min = 0.0;
    double load_average_5min = 0.0;
    double load_average_15min = 0.0;

    // disk sector stats
    size_t prev_sectors_read = 0;
    size_t prev_sectors_written = 0;
    bool first_disk_read = true;
    // HTTP server
    std::atomic<bool> server_running{false};
    std::thread server_thread;
    int server_socket = -1;
    
    // Timestamp for this collection cycle
    std::chrono::system_clock::time_point timestamp;
    
    // Helper functions
    std::string getCurrentTimestamp() const;
    void serverLoop(int port);
    void handleClient(int client_socket);
    std::string buildHTTPResponse(const std::string& body, const std::string& content_type = "application/json") const;
};