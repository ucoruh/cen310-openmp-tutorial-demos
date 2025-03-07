#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <chrono>
#include <omp.h>
#include <numeric>
#include <algorithm>
#include <cmath>

// Timer utility class for measuring execution time
class Timer {
private:
    std::chrono::high_resolution_clock::time_point start_time;
    std::chrono::high_resolution_clock::time_point end_time;
    bool running;

public:
    Timer() : running(false) {}

    void start() {
        start_time = std::chrono::high_resolution_clock::now();
        running = true;
    }

    void stop() {
        end_time = std::chrono::high_resolution_clock::now();
        running = false;
    }

    double elapsedMilliseconds() {
        auto end = running ? std::chrono::high_resolution_clock::now() : end_time;
        return std::chrono::duration<double, std::milli>(end - start_time).count();
    }
};

// Function to check if a number is prime
bool isPrime(int n) {
    if (n <= 1) return false;
    if (n <= 3) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;
    
    for (int i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0) return false;
    }
    return true;
}

// Function to count primes up to a given number
int countPrimes(int limit) {
    int count = 0;
    for (int i = 2; i <= limit; i++) {
        if (isPrime(i)) count++;
        
        // Add some variability to the workload (simulate uneven computation)
        if (i % 10 == 0) {
            for (int j = 0; j < i % 100; j++) {
                // Extra work for numbers divisible by 10
                double dummy = std::sin(j) * std::cos(j);
                dummy += 1.0; // Prevent optimization
            }
        }
    }
    return count;
}

// Structure to hold scheduling results
struct SchedulingResult {
    std::string name;
    double time;
    double speedup;
    std::vector<int> thread_work_counts;
    
    SchedulingResult(const std::string& name_) : name(name_), time(0), speedup(0) {}
};

// Function to visualize thread workload distribution
void visualizeThreadWorkload(const std::vector<int>& work_counts) {
    int max_work = *std::max_element(work_counts.begin(), work_counts.end());
    int min_work = *std::min_element(work_counts.begin(), work_counts.end());
    int work_range = max_work - min_work;
    
    std::cout << "  Load balance: ";
    if (work_range == 0) {
        std::cout << "Perfect (all threads did equal work)\n";
    } else {
        double imbalance = static_cast<double>(work_range) / max_work * 100.0;
        std::cout << std::fixed << std::setprecision(2) << imbalance << "% imbalance\n";
    }
    
    std::cout << "  Thread work distribution:\n";
    
    for (size_t i = 0; i < work_counts.size(); i++) {
        std::cout << "  Thread " << i << ": [";
        
        // Normalize the work to a scale of 50 characters
        int chars = static_cast<int>((static_cast<double>(work_counts[i]) / max_work) * 50.0);
        
        for (int j = 0; j < chars; j++) {
            std::cout << "#";
        }
        
        std::cout << "] " << work_counts[i] << " iterations\n";
    }
    std::cout << "\n";
}

// Function to visualize iteration assignment patterns
void visualizeIterationPattern(const std::string& schedule_type, int num_threads, int iterations, int chunk_size = 0) {
    std::cout << "  Iteration pattern with " << schedule_type;
    if (chunk_size > 0) {
        std::cout << " (chunk size = " << chunk_size << ")";
    }
    std::cout << ":\n";
    
    // Create a simplified model of how iterations would be assigned
    std::vector<std::vector<bool>> thread_iterations(num_threads, std::vector<bool>(iterations, false));
    
    if (schedule_type == "static") {
        if (chunk_size <= 0) {
            // Default static scheduling
            int iters_per_thread = iterations / num_threads;
            int extra = iterations % num_threads;
            
            int pos = 0;
            for (int t = 0; t < num_threads; t++) {
                int count = iters_per_thread + (t < extra ? 1 : 0);
                for (int i = 0; i < count; i++) {
                    if (pos < iterations) {
                        thread_iterations[t][pos++] = true;
                    }
                }
            }
        } else {
            // Static with chunk size
            int chunk = 0;
            int thread = 0;
            
            for (int i = 0; i < iterations; i++) {
                if (i % chunk_size == 0 && i > 0) {
                    chunk++;
                    thread = chunk % num_threads;
                }
                thread_iterations[thread][i] = true;
            }
        }
    } else if (schedule_type == "dynamic") {
        // Simplified dynamic scheduling model
        int i = 0;
        int thread = 0;
        
        while (i < iterations) {
            for (int c = 0; c < chunk_size && i < iterations; c++) {
                thread_iterations[thread][i++] = true;
            }
            thread = (thread + 1) % num_threads;
        }
    } else if (schedule_type == "guided") {
        // Simplified guided scheduling model
        int i = 0;
        int thread = 0;
        int remaining = iterations;
        
        while (i < iterations) {
            int current_chunk = std::max(chunk_size, remaining / num_threads);
            current_chunk = std::min(current_chunk, remaining);
            
            for (int c = 0; c < current_chunk && i < iterations; c++) {
                thread_iterations[thread][i++] = true;
            }
            
            remaining -= current_chunk;
            thread = (thread + 1) % num_threads;
        }
    }
    
    // Visualize the iteration patterns (simplified)
    int display_iters = std::min(iterations, 40); // Limit display width
    double scale = static_cast<double>(display_iters) / iterations;
    
    for (int t = 0; t < num_threads; t++) {
        std::cout << "  Thread " << t << ": [";
        
        for (int i = 0; i < display_iters; i++) {
            int actual_iter = static_cast<int>(i / scale);
            std::cout << (thread_iterations[t][actual_iter] ? "#" : ".");
        }
        
        std::cout << "]\n";
    }
    std::cout << "\n";
}

int main() {
    std::cout << "=========================================\n";
    std::cout << "   OpenMP Scheduling Strategies Demo\n";
    std::cout << "=========================================\n\n";
    
    // Get number of available threads
    int max_threads = omp_get_max_threads();
    std::cout << "Number of available threads: " << max_threads << "\n\n";
    
    // Set up workload parameters
    const int NUM_WORKLOAD_ITEMS = 100;   // Number of tasks
    const int MAX_NUMBER = 50000;         // Upper limit for prime calculation
    
    Timer timer;
    
    // Create workload with varying computational intensity
    std::cout << "Generating workload...\n";
    std::vector<int> workload(NUM_WORKLOAD_ITEMS);
    
    for (int i = 0; i < NUM_WORKLOAD_ITEMS; i++) {
        // Create a varying workload
        // Items in the middle of the array require more computation
        double factor = 1.0;
        if (i > NUM_WORKLOAD_ITEMS / 4 && i < NUM_WORKLOAD_ITEMS * 3 / 4) {
            factor = 3.0 * (1.0 - std::abs(2.0 * i / NUM_WORKLOAD_ITEMS - 1.0));
        }
        
        workload[i] = static_cast<int>(MAX_NUMBER * factor);
    }
    
    std::cout << "Workload generated. " << NUM_WORKLOAD_ITEMS << " tasks with varying intensity.\n\n";
    
    // Measure sequential execution time as baseline
    std::cout << "Running sequential execution...\n";
    std::vector<int> results_sequential(NUM_WORKLOAD_ITEMS);
    
    timer.start();
    for (int i = 0; i < NUM_WORKLOAD_ITEMS; i++) {
        results_sequential[i] = countPrimes(workload[i]);
    }
    timer.stop();
    
    double sequential_time = timer.elapsedMilliseconds();
    std::cout << "Sequential execution time: " << sequential_time << " ms\n\n";
    
    // Prepare result structures for different scheduling strategies
    std::vector<SchedulingResult> results;
    results.push_back(SchedulingResult("Static (default)"));
    results.push_back(SchedulingResult("Static (chunk=1)"));
    results.push_back(SchedulingResult("Static (chunk=10)"));
    results.push_back(SchedulingResult("Dynamic (default)"));
    results.push_back(SchedulingResult("Dynamic (chunk=1)"));
    results.push_back(SchedulingResult("Dynamic (chunk=10)"));
    results.push_back(SchedulingResult("Guided (default)"));
    results.push_back(SchedulingResult("Guided (chunk=1)"));
    results.push_back(SchedulingResult("Runtime"));
    
    // Initialize thread work counters for each scheduling strategy
    for (auto& result : results) {
        result.thread_work_counts.resize(max_threads, 0);
    }
    
    std::cout << "Starting tests with different scheduling strategies...\n\n";
    
    // Run tests with different scheduling strategies
    
    // Static scheduling (default)
    {
        std::vector<int> results_parallel(NUM_WORKLOAD_ITEMS);
        timer.start();
        #pragma omp parallel for schedule(static)
        for (int i = 0; i < NUM_WORKLOAD_ITEMS; i++) {
            results_parallel[i] = countPrimes(workload[i]);
            results[0].thread_work_counts[omp_get_thread_num()]++;
        }
        timer.stop();
        results[0].time = timer.elapsedMilliseconds();
        results[0].speedup = sequential_time / results[0].time;
    }
    
    // Static scheduling (chunk=1)
    {
        std::vector<int> results_parallel(NUM_WORKLOAD_ITEMS);
        timer.start();
        #pragma omp parallel for schedule(static, 1)
        for (int i = 0; i < NUM_WORKLOAD_ITEMS; i++) {
            results_parallel[i] = countPrimes(workload[i]);
            results[1].thread_work_counts[omp_get_thread_num()]++;
        }
        timer.stop();
        results[1].time = timer.elapsedMilliseconds();
        results[1].speedup = sequential_time / results[1].time;
    }
    
    // Static scheduling (chunk=10)
    {
        std::vector<int> results_parallel(NUM_WORKLOAD_ITEMS);
        timer.start();
        #pragma omp parallel for schedule(static, 10)
        for (int i = 0; i < NUM_WORKLOAD_ITEMS; i++) {
            results_parallel[i] = countPrimes(workload[i]);
            results[2].thread_work_counts[omp_get_thread_num()]++;
        }
        timer.stop();
        results[2].time = timer.elapsedMilliseconds();
        results[2].speedup = sequential_time / results[2].time;
    }
    
    // Dynamic scheduling (default)
    {
        std::vector<int> results_parallel(NUM_WORKLOAD_ITEMS);
        timer.start();
        #pragma omp parallel for schedule(dynamic)
        for (int i = 0; i < NUM_WORKLOAD_ITEMS; i++) {
            results_parallel[i] = countPrimes(workload[i]);
            results[3].thread_work_counts[omp_get_thread_num()]++;
        }
        timer.stop();
        results[3].time = timer.elapsedMilliseconds();
        results[3].speedup = sequential_time / results[3].time;
    }
    
    // Dynamic scheduling (chunk=1)
    {
        std::vector<int> results_parallel(NUM_WORKLOAD_ITEMS);
        timer.start();
        #pragma omp parallel for schedule(dynamic, 1)
        for (int i = 0; i < NUM_WORKLOAD_ITEMS; i++) {
            results_parallel[i] = countPrimes(workload[i]);
            results[4].thread_work_counts[omp_get_thread_num()]++;
        }
        timer.stop();
        results[4].time = timer.elapsedMilliseconds();
        results[4].speedup = sequential_time / results[4].time;
    }
    
    // Dynamic scheduling (chunk=10)
    {
        std::vector<int> results_parallel(NUM_WORKLOAD_ITEMS);
        timer.start();
        #pragma omp parallel for schedule(dynamic, 10)
        for (int i = 0; i < NUM_WORKLOAD_ITEMS; i++) {
            results_parallel[i] = countPrimes(workload[i]);
            results[5].thread_work_counts[omp_get_thread_num()]++;
        }
        timer.stop();
        results[5].time = timer.elapsedMilliseconds();
        results[5].speedup = sequential_time / results[5].time;
    }
    
    // Guided scheduling (default)
    {
        std::vector<int> results_parallel(NUM_WORKLOAD_ITEMS);
        timer.start();
        #pragma omp parallel for schedule(guided)
        for (int i = 0; i < NUM_WORKLOAD_ITEMS; i++) {
            results_parallel[i] = countPrimes(workload[i]);
            results[6].thread_work_counts[omp_get_thread_num()]++;
        }
        timer.stop();
        results[6].time = timer.elapsedMilliseconds();
        results[6].speedup = sequential_time / results[6].time;
    }
    
    // Guided scheduling (chunk=1)
    {
        std::vector<int> results_parallel(NUM_WORKLOAD_ITEMS);
        timer.start();
        #pragma omp parallel for schedule(guided, 1)
        for (int i = 0; i < NUM_WORKLOAD_ITEMS; i++) {
            results_parallel[i] = countPrimes(workload[i]);
            results[7].thread_work_counts[omp_get_thread_num()]++;
        }
        timer.stop();
        results[7].time = timer.elapsedMilliseconds();
        results[7].speedup = sequential_time / results[7].time;
    }
    
    // Auto scheduling
    {
        std::vector<int> results_parallel(NUM_WORKLOAD_ITEMS);
        timer.start();
        #pragma omp parallel for schedule(runtime)
        for (int i = 0; i < NUM_WORKLOAD_ITEMS; i++) {
            results_parallel[i] = countPrimes(workload[i]);
            results[8].thread_work_counts[omp_get_thread_num()]++;
        }
        timer.stop();
        results[8].time = timer.elapsedMilliseconds();
        results[8].speedup = sequential_time / results[8].time;
    }
    
    // Display summary of results
    std::cout << "=========================================\n";
    std::cout << "Performance Summary (sorted by execution time)\n";
    std::cout << "=========================================\n\n";
    
    // Sort results by execution time
    std::sort(results.begin(), results.end(), [](const SchedulingResult& a, const SchedulingResult& b) {
        return a.time < b.time;
    });
    
    // Print results in table format
    std::cout << std::setw(20) << "Scheduling Type" << " | " 
              << std::setw(12) << "Time (ms)" << " | " 
              << std::setw(10) << "Speedup" << " | " 
              << std::setw(15) << "Load Balance" << "\n";
    std::cout << std::string(65, '-') << "\n";
    
    for (const auto& result : results) {
        // Calculate load balance metrics
        int max_work = *std::max_element(result.thread_work_counts.begin(), result.thread_work_counts.end());
        int min_work = *std::min_element(result.thread_work_counts.begin(), result.thread_work_counts.end());
        
        double imbalance = 0.0;
        if (max_work > 0) {
            imbalance = static_cast<double>(max_work - min_work) / max_work * 100.0;
        }
        
        std::cout << std::setw(20) << result.name << " | " 
                  << std::setw(12) << std::fixed << std::setprecision(2) << result.time << " | " 
                  << std::setw(10) << std::fixed << std::setprecision(2) << result.speedup << " | "
                  << std::setw(13) << std::fixed << std::setprecision(2) << imbalance << "%" << "\n";
    }
    
    // Show detailed visualization for the fastest scheduling strategy
    std::cout << "\n=========================================\n";
    std::cout << "Detailed Analysis of Best Strategy: " << results[0].name << "\n";
    std::cout << "=========================================\n\n";
    
    std::cout << "Execution time: " << results[0].time << " ms\n";
    std::cout << "Speedup: " << results[0].speedup << "x\n\n";
    
    visualizeThreadWorkload(results[0].thread_work_counts);
    
    // Show iteration pattern visualizations for different scheduling types
    std::cout << "=========================================\n";
    std::cout << "Iteration Assignment Patterns (Simplified)\n";
    std::cout << "=========================================\n\n";
    
    visualizeIterationPattern("static", max_threads, NUM_WORKLOAD_ITEMS);
    visualizeIterationPattern("static", max_threads, NUM_WORKLOAD_ITEMS, 10);
    visualizeIterationPattern("dynamic", max_threads, NUM_WORKLOAD_ITEMS, 1);
    visualizeIterationPattern("guided", max_threads, NUM_WORKLOAD_ITEMS, 1);
    
    std::cout << "=========================================\n";
    std::cout << "Scheduling Recommendations\n";
    std::cout << "=========================================\n\n";
    
    std::cout << "For this particular workload:\n";
    std::cout << "- Best strategy: " << results[0].name << "\n";
    std::cout << "- Worst strategy: " << results.back().name << "\n\n";
    
    std::cout << "General Guidelines:\n";
    std::cout << "- Static scheduling: Best for uniform workloads\n";
    std::cout << "- Dynamic scheduling: Best for non-uniform workloads\n";
    std::cout << "- Guided scheduling: Best for workloads with decreasing complexity\n";
    std::cout << "- Runtime scheduling: Let the runtime decide (using OMP_SCHEDULE environment variable)\n\n";
    
    std::cout << "Chunk size considerations:\n";
    std::cout << "- Smaller chunks: Better load balancing but higher overhead\n";
    std::cout << "- Larger chunks: Lower overhead but potentially worse load balancing\n\n";
    
    return 0;
} 