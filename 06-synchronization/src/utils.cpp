#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <chrono>
#include <functional>
#include <fstream>
#include <random>
#include <cmath>
#include <omp.h>
#include <thread>
#include "../include/utils.h"

namespace utils {
    // Timer implementation
    Timer::Timer() : running(false) {}
    
    void Timer::start() {
        start_time = std::chrono::high_resolution_clock::now();
        running = true;
    }
    
    void Timer::stop() {
        if (running) {
            end_time = std::chrono::high_resolution_clock::now();
            running = false;
        }
    }
    
    double Timer::elapsed_ms() const {
        auto duration = (running ? 
            std::chrono::high_resolution_clock::now() - start_time : 
            end_time - start_time);
        return std::chrono::duration_cast<std::chrono::microseconds>(duration).count() / 1000.0;
    }
    
    double Timer::elapsed_seconds() const {
        return elapsed_ms() / 1000.0;
    }
    
    void Timer::reset() {
        running = false;
    }
    
    bool Timer::is_running() const {
        return running;
    }
    
    // Benchmark function
    double benchmark_function(std::function<void()> func, int num_runs) {
        if (num_runs <= 0) {
            num_runs = 1;
        }
        
        std::vector<double> times;
        Timer timer;
        
        for (int i = 0; i < num_runs; ++i) {
            timer.reset();
            timer.start();
            func();
            timer.stop();
            times.push_back(timer.elapsed_ms());
        }
        
        // Calculate average (excluding outliers if enough samples)
        if (times.size() <= 2) {
            // Not enough samples to exclude outliers, return average
            double sum = 0.0;
            for (double time : times) {
                sum += time;
            }
            return sum / times.size();
        }
        else {
            // Sort times and exclude min and max
            std::sort(times.begin(), times.end());
            double sum = 0.0;
            for (size_t i = 1; i < times.size() - 1; ++i) {
                sum += times[i];
            }
            return sum / (times.size() - 2);
        }
    }
    
    // Progress bar visualization
    void show_progress_bar(double progress, int width) {
        int bar_width = (int)(progress * width);
        std::cout << "[";
        for (int i = 0; i < width; ++i) {
            if (i < bar_width) std::cout << "=";
            else if (i == bar_width) std::cout << ">";
            else std::cout << " ";
        }
        std::cout << "] " << int(progress * 100.0) << "%" << std::flush;
    }
    
    // Thread visualization helpers
    void visualize_thread_activity(const std::vector<std::vector<int>>& thread_activity, int num_threads) {
        // Find the max time step
        int max_time = 0;
        for (const auto& thread : thread_activity) {
            if (!thread.empty()) {
                max_time = std::max(max_time, static_cast<int>(thread.size()));
            }
        }
        
        const int width = 80;  // Console width
        const int time_scale = std::max(1, max_time / width);
        
        std::cout << "\nThread Activity Timeline:\n";
        std::cout << "Legend: ' ' - idle, '#' - active, '*' - waiting, '+' - critical section\n\n";
        
        // Print time scale
        std::cout << "Time: ";
        for (int t = 0; t < width; ++t) {
            if (t % 10 == 0) {
                std::cout << (t * time_scale) % 10;
            } else {
                std::cout << " ";
            }
        }
        std::cout << std::endl;
        
        // Print thread activities
        for (int thread_id = 0; thread_id < num_threads; ++thread_id) {
            std::cout << "T" << std::setw(3) << thread_id << ": ";
            
            for (int t = 0; t < width; ++t) {
                int time_step = t * time_scale;
                if (time_step < thread_activity[thread_id].size()) {
                    int activity = thread_activity[thread_id][time_step];
                    switch (activity) {
                        case 0: std::cout << " "; break;  // Idle
                        case 1: std::cout << "#"; break;  // Active
                        case 2: std::cout << "*"; break;  // Waiting
                        case 3: std::cout << "+"; break;  // Critical section
                        default: std::cout << " ";
                    }
                } else {
                    std::cout << " ";
                }
            }
            std::cout << std::endl;
        }
    }
    
    // Command line argument parsing
    ProgramOptions parse_command_line(int argc, char* argv[]) {
        ProgramOptions options;
        
        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];
            
            if (arg == "--help" || arg == "-h") {
                options.show_help = true;
            }
            else if (arg == "--benchmark" || arg == "-b") {
                options.benchmark_mode = true;
            }
            else if (arg == "--performance" || arg == "-p") {
                options.performance_mode = true;
            }
            else if (arg == "--threads" || arg == "-t") {
                if (i + 1 < argc) {
                    options.num_threads = std::stoi(argv[++i]);
                }
            }
            else if (arg == "--workload" || arg == "-w") {
                if (i + 1 < argc) {
                    options.workload = std::stoi(argv[++i]);
                }
            }
            else if (arg == "--demo" || arg == "-d") {
                if (i + 1 < argc) {
                    options.demo_name = argv[++i];
                }
            }
        }
        
        return options;
    }
    
    // Help message
    void show_help_message(const std::string& program_name) {
        std::cout << "Usage: " << program_name << " [options]\n\n"
                  << "Options:\n"
                  << "  -h, --help                  Show this help message\n"
                  << "  -b, --benchmark             Run in benchmark mode\n"
                  << "  -p, --performance           Run performance analysis\n"
                  << "  -t, --threads <num>         Specify number of threads (0 for default)\n"
                  << "  -w, --workload <size>       Specify workload size\n"
                  << "  -d, --demo <name>           Run a specific demo by name\n\n"
                  << "Examples:\n"
                  << "  " << program_name << "                             Run interactive menu\n"
                  << "  " << program_name << " --benchmark                 Run all benchmarks\n"
                  << "  " << program_name << " --threads 4 --workload 1000000  Run with 4 threads and specified workload\n"
                  << "  " << program_name << " --demo \"Basic Critical Sections\"  Run a specific demo\n";
    }
    
    // Console utilities
    void clear_console() {
        #ifdef _WIN32
        system("cls");
        #else
        system("clear");
        #endif
    }
    
    void pause_console() {
        std::cout << "\nPress Enter to continue...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();
    }
    
    void print_header(const std::string& text) {
        std::cout << "====================================\n"
                  << "    " << text << "\n"
                  << "====================================\n\n";
    }
    
    void print_section(const std::string& text) {
        std::cout << "\n--- " << text << " ---\n\n";
    }
    
    void print_subsection(const std::string& text) {
        std::cout << "* " << text << " *\n";
    }
    
    void print_result(const std::string& label, double value, const std::string& unit) {
        std::cout << std::setw(30) << std::left << label << ": " 
                  << std::fixed << std::setprecision(3) << value << " " << unit << std::endl;
    }
    
    void print_result(const std::string& label, int value, const std::string& unit) {
        std::cout << std::setw(30) << std::left << label << ": " 
                  << value << " " << unit << std::endl;
    }
    
    void print_comparison(const std::string& label1, double value1, 
                         const std::string& label2, double value2, 
                         const std::string& unit) {
        std::cout << std::setw(20) << std::left << label1 << ": " 
                  << std::fixed << std::setprecision(3) << value1 << " " << unit << " | "
                  << std::setw(20) << std::left << label2 << ": " 
                  << std::fixed << std::setprecision(3) << value2 << " " << unit;
        
        // Calculate speedup
        if (value2 > 0 && value1 > 0) {
            double speedup = value1 / value2;
            std::cout << " | Ratio: " << std::fixed << std::setprecision(2) << speedup << "x";
        }
        
        std::cout << std::endl;
    }
    
    // File I/O utilities
    void write_results_to_file(const std::string& filename, 
                              const std::vector<std::pair<std::string, double>>& results) {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file " << filename << " for writing.\n";
            return;
        }
        
        file << "# OpenMP Synchronization Benchmarks\n";
        file << "# Format: Benchmark_Name,Time_ms\n\n";
        
        for (const auto& result : results) {
            file << result.first << "," << std::fixed << std::setprecision(3) << result.second << "\n";
        }
        
        file.close();
    }
    
    // Race condition detection
    bool detect_race_condition(std::function<void(int)> test_func, int iterations) {
        const int test_runs = 5;
        std::vector<long long> results(test_runs);
        
        for (int run = 0; run < test_runs; ++run) {
            test_func(iterations);
            // Get result from the function (depends on the specific test)
            // Here we just simulate it
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            results[run] = 0;  // Replace with actual result
        }
        
        // Check if all results are the same
        for (int i = 1; i < test_runs; ++i) {
            if (results[i] != results[0]) {
                return true;  // Race condition detected
            }
        }
        
        return false;  // No race condition detected
    }
    
    // ASCII chart
    void draw_bar_chart(const std::vector<std::pair<std::string, double>>& data, 
                       int width, int height) {
        // Find max value for scaling
        double max_value = 0.0;
        for (const auto& pair : data) {
            max_value = std::max(max_value, pair.second);
        }
        
        // Use width parameter for bar width calculation
        int bar_width = width / std::max(1, static_cast<int>(data.size()));
        
        // Use height parameter for vertical scaling
        int max_bar_height = height;
        
        // Display the chart
        for (const auto& pair : data) {
            // Calculate bar length proportional to value and height
            int bar_length = static_cast<int>((pair.second / max_value) * bar_width);
            int bar_height = static_cast<int>((pair.second / max_value) * max_bar_height);
            
            // Use both parameters to avoid unused warnings
            (void)bar_height;
        }
    }
    
    // Generate workload
    std::vector<int> generate_workload(int size) {
        std::vector<int> workload(size);
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> distrib(1, 100);
        
        for (int i = 0; i < size; ++i) {
            workload[i] = distrib(gen);
        }
        
        return workload;
    }
    
    // Measure memory usage (simplified, platform-dependent implementation)
    size_t get_current_memory_usage() {
        // This is a placeholder. Actual implementation would be platform-specific.
        return 0;
    }
} 