#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <functional>
#include <iostream>

namespace utils {
    // Timer class for performance measurements
    class Timer {
    private:
        std::chrono::high_resolution_clock::time_point start_time;
        std::chrono::high_resolution_clock::time_point end_time;
        bool running;

    public:
        Timer();
        void start();
        void stop();
        double elapsed_ms() const;
        double elapsed_seconds() const;
        void reset();
        bool is_running() const;
    };

    // Benchmark a function with given parameters
    double benchmark_function(std::function<void()> func, int num_runs = 5);

    // Progress bar visualization
    void show_progress_bar(double progress, int width = 50);

    // Thread visualization helpers
    void visualize_thread_activity(const std::vector<std::vector<int>>& thread_activity, int num_threads);
    
    // Function to parse command line arguments
    struct ProgramOptions {
        bool benchmark_mode = false;
        bool performance_mode = false;
        int num_threads = 0;  // 0 means use default (max available)
        int workload = 1000000;
        std::string demo_name = "";
        bool show_help = false;
    };

    ProgramOptions parse_command_line(int argc, char* argv[]);

    // Print help message
    void show_help_message(const std::string& program_name);

    // Console utilities
    void clear_console();
    void pause_console();
    void print_header(const std::string& text);
    void print_section(const std::string& text);
    void print_subsection(const std::string& text);
    void print_result(const std::string& label, double value, const std::string& unit = "ms");
    void print_result(const std::string& label, int value, const std::string& unit = "");
    void print_comparison(const std::string& label1, double value1, 
                         const std::string& label2, double value2, 
                         const std::string& unit = "ms");

    // File I/O utilities
    void write_results_to_file(const std::string& filename, 
                              const std::vector<std::pair<std::string, double>>& results);
    
    // Race condition detection (simplified)
    bool detect_race_condition(std::function<void(int)> test_func, int iterations = 1000);
    
    // ASCII chart for visualizing performance results
    void draw_bar_chart(const std::vector<std::pair<std::string, double>>& data, 
                       int width = 60, int height = 10);

    // Generate a workload with the specified size
    std::vector<int> generate_workload(int size);
    
    // Function to measure memory usage
    size_t get_current_memory_usage();
} 