#include "monitor.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>


void PerformanceMonitor::collectCPUUsage(){
    std::ifstream statFile("/proc/stat");
    if(!statFile.is_open()){
        // add throw later, return for now
        return;
    }
    std::string line;
    while(std::getline(statFile, line)){
        std::stringstream ss(line);
        std::string key;
        ss >> key; // reading first word from stream into key
        if(key == "cpu"){
            // read all the CPU values from stream and store
            long user, nice, system, idle, iowait, irq, softirq, steal;
            ss >> user >> nice >> system >> idle >> iowait>> irq >> softirq >> steal;

            // calc totals
            long total_idle = idle + iowait;
            long total_active = user + nice + system + irq + softirq + steal;
            long total_time = total_idle + total_active;
            // first read check
            if(first_cpu_read){
                prev_total_time = total_time;
                prev_active_time = total_active;
                first_cpu_read = false;
                cpu_usage = 0.0;
            }
            else{
                // calc based on difference from last read
                long diff_total = total_time - prev_total_time;
                long diff_active = total_active - prev_active_time;

                if(diff_total > 0){
                    cpu_usage = (double)diff_active / diff_total * 100.0;
                }

                prev_total_time = total_time;
                prev_active_time = total_active;
            }

        } 
        break;
    }
}

void PerformanceMonitor::collectMemoryUsage(){
    std::ifstream memFile("/proc/meminfo");
    if(!memFile.is_open()){
        // add throw later
        return;
    }
    long total_mem, available_mem;
    std::string line;
    while(std::getline(memFile, line)){
        std::stringstream ss(line);
        std::string key;
        ss >> key;
        if(key == "MemTotal:"){
            ss >> total_mem;
        }
        else if(key == "MemAvailable:"){
            ss >> available_mem;
            break;
        }
    }
    memory_usage = total_mem - available_mem;
}

void PerformanceMonitor::collectLoadAverage(){
    std::ifstream loadFile("/proc/loadavg");
    if(!loadFile.is_open()){
        return;
    }
    loadFile >> load_average_1min >> load_average_5min >> load_average_15min;
}


void PerformanceMonitor::collectProcessCount(){
    std::ifstream statFile("/proc/stat");
    if(!statFile.is_open()){
        return;
    }
    std::string line;
    while(std::getline(statFile, line)){
        std::stringstream ss(line);
        std::string key;
        ss >> key;

        if(key == "processes"){
            ss >> process_count;
            break;
        }
    }
}

void PerformanceMonitor::collectNetworkStats(){
    std::ifstream netFile("/proc/net/dev");
    if(!netFile.is_open()){
        return;
    }

    std::string line;
    std::getline(netFile, line); //header1
    std::getline(netFile, line); // header2
    size_t total_recv = 0, total_sent = 0;
    
    while(std::getline(netFile, line)){
        std::stringstream ss(line);
        std::string interface;
        ss >> interface;

        if(interface.find("lo:") == 0){
            continue;
        }

        size_t recv_bytes;
        ss >> recv_bytes;

        for(int i = 0; i < 7; i++){
            size_t dummy;
            ss >> dummy;
        }
        size_t sent_bytes;
        ss >> sent_bytes;

        total_recv += recv_bytes;
        total_sent += sent_bytes;
    }
    network_stats.bytes_received = total_recv;
    network_stats.bytes_sent = total_sent;
}

void PerformanceMonitor::collectDiskStats(){
    std::ifstream diskFile("/proc/diskstats");
    if(!diskFile.is_open()){
        return;
    }
    
    std::string line;
    size_t current_sectors_read = 0, current_sectors_written = 0;
    
    while(std::getline(diskFile, line)){
        std::stringstream ss(line);
        int major, minor;
        std::string device;
        ss >> major >> minor >> device;
        
        // Check if this is a main disk device (not a partition)
        bool is_main_device = false;
        
        // NVMe devices: nvme0n1, nvme1n1 (not nvme0n1p1, nvme1n1p2)
        if(device.find("nvme") == 0 && device.find("p") == std::string::npos) {
            is_main_device = true;
        }
        // SATA/SSD devices: sda, sdb, sdc (not sda1, sdb2)
        else if(device.find("sd") == 0 && device.length() == 3) {
            is_main_device = true;
        }
        // Virtual/IDE devices: hda, hdb (not hda1, hdb2)
        else if(device.find("hd") == 0 && device.length() == 3) {
            is_main_device = true;
        }
        // MMC/eMMC storage: mmcblk0, mmcblk1 (not mmcblk0p1)
        else if(device.find("mmcblk") == 0 && device.find("p") == std::string::npos) {
            is_main_device = true;
        }
        // VM devices: vda, vdb, xvda, xvdb (not vda1, xvda2)
        else if((device.find("vd") == 0 || device.find("xvd") == 0) && 
                device.length() >= 3 && std::isalpha(device.back())) {
            is_main_device = true;
        }
        
        // Skip virtual/loop devices
        if(device.find("loop") == 0 || device.find("ram") == 0 || 
           device.find("dm-") == 0 || device.find("zram") == 0) {
            is_main_device = false;
        }
        
        if(is_main_device) {
            size_t reads, reads_merged, sectors_read, time_reading;
            size_t writes, writes_merged, sectors_written, time_writing;
            
            ss >> reads >> reads_merged >> sectors_read >> time_reading
               >> writes >> writes_merged >> sectors_written >> time_writing;
            
            // Sum up all real disk devices
            current_sectors_read += sectors_read;
            current_sectors_written += sectors_written;
        }
    }
    
    if(first_disk_read) {
        // First reading - just store baseline
        prev_sectors_read = current_sectors_read;
        prev_sectors_written = current_sectors_written;
        first_disk_read = false;
        disk_stats.bytes_read = 0;
        disk_stats.bytes_written = 0;
    } else {
        // Calculate difference since last reading
        size_t diff_read = current_sectors_read - prev_sectors_read;
        size_t diff_written = current_sectors_written - prev_sectors_written;
        
        // Convert sectors to bytes (512 bytes per sector)
        disk_stats.bytes_read = diff_read * 512;
        disk_stats.bytes_written = diff_written * 512;
        
        // Store current values for next time
        prev_sectors_read = current_sectors_read;
        prev_sectors_written = current_sectors_written;
    }
}

void PerformanceMonitor::printStats() const {
    std::cout << "=== Performance Stats ===" << std::endl;
    std::cout << "CPU: " << cpu_usage << "%" << std::endl;
    std::cout << "Memory: " << memory_usage << " KB" << std::endl;
    std::cout << "Processes: " << process_count << std::endl;
    std::cout << "Load: " << load_average_1min << " " << load_average_5min << " " << load_average_15min << std::endl;
    std::cout << "Network - Sent: " << network_stats.bytes_sent << " bytes, Received: " << network_stats.bytes_received << " bytes" << std::endl;
    std::cout << "Disk - Read: " << disk_stats.bytes_read << " bytes, Written: " << disk_stats.bytes_written << " bytes" << std::endl;
    std::cout << std::endl;
}


std::string PerformanceMonitor::toJSON() const {
    std::stringstream json;
    json << std::fixed << std::setprecision(2);
    
    json << "{\n";
    json << "  \"cpu_usage\": " << cpu_usage << ",\n";
    json << "  \"memory_usage_kb\": " << memory_usage << ",\n";
    json << "  \"network\": {\n";
    json << "    \"bytes_sent\": " << network_stats.bytes_sent << ",\n";
    json << "    \"bytes_received\": " << network_stats.bytes_received << "\n";
    json << "  },\n";
    json << "  \"disk\": {\n";
    json << "    \"bytes_read\": " << disk_stats.bytes_read << ",\n";
    json << "    \"bytes_written\": " << disk_stats.bytes_written << "\n";
    json << "  },\n";
    json << "  \"processes\": " << process_count << ",\n";
    json << "  \"load_average\": {\n";
    json << "    \"1min\": " << load_average_1min << ",\n";
    json << "    \"5min\": " << load_average_5min << ",\n";
    json << "    \"15min\": " << load_average_15min << "\n";
    json << "  }\n";
    json << "}";
    
    return json.str();
}

void PerformanceMonitor::saveToFile(const std::string& filename) const {
    std::ofstream file(filename);
    if(file.is_open()) {
        file << toJSON();
    }
}

void PerformanceMonitor::collectAllMetrics() {
    collectCPUUsage();
    collectMemoryUsage();
    collectNetworkStats();
    collectDiskStats();
    collectProcessCount();
    collectLoadAverage();
}


// http server implementation funcs 
void PerformanceMonitor::startHTTPServer(int port) {
    if (server_running) {
        std::cout << "Server already running!" << std::endl;
        return;
    }
    
    server_running = true;
    server_thread = std::thread(&PerformanceMonitor::serverLoop, this, port);
    
    std::cout << "HTTP Server started on port " << port << std::endl;
    std::cout << "Try: curl http://localhost:" << port << "/metrics" << std::endl;
}

void PerformanceMonitor::stopHTTPServer() {
    if (!server_running) {
        return;
    }
    
    server_running = false;
    
    if (server_socket != -1) {
        close(server_socket);
        server_socket = -1;
    }
    
    if (server_thread.joinable()) {
        server_thread.join();
    }
    
    std::cout << "HTTP Server stopped" << std::endl;
}

bool PerformanceMonitor::isServerRunning() const {
    return server_running;
}

void PerformanceMonitor::serverLoop(int port) {
    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        std::cerr << "Failed to create socket" << std::endl;
        server_running = false;
        return;
    }
    
    // Set socket options
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    // Bind socket
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    
    if (bind(server_socket, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed" << std::endl;
        close(server_socket);
        server_running = false;
        return;
    }
    
    // Listen for connections
    if (listen(server_socket, 3) < 0) {
        std::cerr << "Listen failed" << std::endl;
        close(server_socket);
        server_running = false;
        return;
    }
    
    // Accept connections
    while (server_running) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        if (client_socket < 0) {
            if (server_running) {  // Only log error if we're still supposed to be running
                std::cerr << "Accept failed" << std::endl;
            }
            continue;
        }
        
        // Handle client in same thread (simple approach)
        handleClient(client_socket);
        close(client_socket);
    }
    
    close(server_socket);
    server_socket = -1;
}

void PerformanceMonitor::handleClient(int client_socket) {
    char buffer[1024] = {0};
    ssize_t bytes_read = read(client_socket, buffer, 1024);
    
    if (bytes_read <= 0) {
        return;
    }
    
    std::string request(buffer);
    std::cout << "Request: " << request.substr(0, request.find('\n')) << std::endl;
    
    // Parse the request line
    std::stringstream ss(request);
    std::string method, path, version;
    ss >> method >> path >> version;
    
    std::string response;
    
    if (method == "GET") {
        if (path == "/metrics" || path == "/") {
            // Collect fresh metrics
            const_cast<PerformanceMonitor*>(this)->collectAllMetrics();
            
            // Return JSON metrics
            std::string json = toJSON();
            response = buildHTTPResponse(json, "application/json");
        } else if (path == "/health") {
            // Simple health check
            response = buildHTTPResponse("{\"status\":\"ok\"}", "application/json");
        } else {
            // 404 Not Found
            std::string error = "{\"error\":\"Not Found\"}";
            response = "HTTP/1.1 404 Not Found\r\n";
            response += "Content-Type: application/json\r\n";
            response += "Content-Length: " + std::to_string(error.length()) + "\r\n";
            response += "Connection: close\r\n\r\n";
            response += error;
        }
    } else {
        // 405 Method Not Allowed
        std::string error = "{\"error\":\"Method Not Allowed\"}";
        response = "HTTP/1.1 405 Method Not Allowed\r\n";
        response += "Content-Type: application/json\r\n";
        response += "Content-Length: " + std::to_string(error.length()) + "\r\n";
        response += "Connection: close\r\n\r\n";
        response += error;
    }
    
    send(client_socket, response.c_str(), response.length(), 0);
}

std::string PerformanceMonitor::buildHTTPResponse(const std::string& body, const std::string& content_type) const {
    std::string response;
    response += "HTTP/1.1 200 OK\r\n";
    response += "Content-Type: " + content_type + "\r\n";
    response += "Content-Length: " + std::to_string(body.length()) + "\r\n";
    response += "Access-Control-Allow-Origin: *\r\n";  // Enable CORS for web dashboards
    response += "Connection: close\r\n";
    response += "\r\n";
    response += body;
    return response;
}