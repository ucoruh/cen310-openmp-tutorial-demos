#include <iostream>
#include <vector>
#include <iomanip>
#include <thread>
#include <omp.h>
#include "../include/synchronization_demos.h"
#include "../include/utils.h"

// Simple counter to demonstrate race conditions
void race_condition_counter(int num_threads, int iterations) {
    utils::print_subsection("Simple Counter Race Condition");
    std::cout << "Incrementing a shared counter with " << num_threads << " threads\n";
    std::cout << "Each thread will increment " << iterations << " times\n\n";
    
    // Expected final count
    long long expected_count = static_cast<long long>(num_threads) * iterations;
    
    // Race condition version
    long long counter_race = 0;
    
    // Set the number of threads
    if (num_threads > 0) {
        omp_set_num_threads(num_threads);
    } else {
        num_threads = omp_get_max_threads();
    }
    
    utils::print_result("Number of threads", num_threads);
    utils::print_result("Iterations per thread", iterations);
    utils::print_result("Expected final count", static_cast<double>(expected_count));
    std::cout << "\nRunning with race condition...\n";
    
    utils::Timer timer;
    timer.start();
    
    #pragma omp parallel 
    {
        for (int i = 0; i < iterations; i++) {
            counter_race++; // Race condition here!
        }
    }
    
    timer.stop();
    
    // Calculate and print results
    utils::print_result("Final count (with race)", static_cast<double>(counter_race));
    utils::print_result("Expected count", static_cast<double>(expected_count));
    utils::print_result("Error", static_cast<double>(expected_count - counter_race));
    utils::print_result("Error percentage", 
                       (expected_count > 0) ? 
                       (100.0 * std::abs(expected_count - counter_race) / expected_count) : 0.0, "%");
    utils::print_result("Execution time", timer.elapsed_ms(), "ms");
    
    std::cout << "\nThe race condition causes the counter to be much lower than expected\n";
    std::cout << "because threads overwrite each other's increments.\n";
}

// Array update race condition
void race_condition_array(int num_threads, int array_size) {
    utils::print_subsection("Array Update Race Condition");
    std::cout << "Updating array elements with " << num_threads << " threads\n";
    std::cout << "Array size: " << array_size << " elements\n\n";
    
    // Create and initialize the array
    std::vector<int> array(array_size, 0);
    
    // Set the number of threads
    if (num_threads > 0) {
        omp_set_num_threads(num_threads);
    } else {
        num_threads = omp_get_max_threads();
    }
    
    utils::print_result("Number of threads", num_threads);
    utils::print_result("Array size", array_size);
    
    // Without race condition for comparison
    std::vector<int> array_safe(array_size, 0);
    
    std::cout << "\nUpdating array without race conditions...\n";
    utils::Timer timer;
    timer.start();
    
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        #pragma omp for
        for (int i = 0; i < array_size; i++) {
            array_safe[i] = thread_id + 1;
        }
    }
    
    timer.stop();
    utils::print_result("Execution time (safe)", timer.elapsed_ms(), "ms");
    
    // Now with race condition
    std::cout << "\nUpdating array with race conditions...\n";
    timer.reset();
    timer.start();
    
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        // No omp for here - threads will race
        for (int i = 0; i < array_size; i++) {
            array[i] = thread_id + 1;
        }
    }
    
    timer.stop();
    utils::print_result("Execution time (race)", timer.elapsed_ms(), "ms");
    
    // Count how many elements were updated by each thread
    std::vector<int> thread_count(num_threads, 0);
    for (int i = 0; i < array_size; i++) {
        if (array[i] > 0 && array[i] <= num_threads) {
            thread_count[array[i] - 1]++;
        }
    }
    
    // Print thread statistics
    std::cout << "\nArray elements updated by each thread:\n";
    for (int i = 0; i < num_threads; i++) {
        double percentage = 100.0 * thread_count[i] / array_size;
        std::cout << "Thread " << std::setw(2) << i << ": " << std::setw(8) 
                  << thread_count[i] << " elements (" 
                  << std::fixed << std::setprecision(2) << percentage << "%)\n";
    }
    
    std::cout << "\nIn the race condition version, threads may overwrite each other's updates,\n";
    std::cout << "resulting in an unpredictable distribution of values in the array.\n";
}

// Sum reduction race condition
void race_condition_sum(int num_threads, int array_size) {
    utils::print_subsection("Sum Reduction Race Condition");
    std::cout << "Summing array elements with " << num_threads << " threads\n";
    std::cout << "Array size: " << array_size << " elements\n\n";
    
    // Create and initialize the array with random values
    std::vector<int> array = utils::generate_workload(array_size);
    
    // Set the number of threads
    if (num_threads > 0) {
        omp_set_num_threads(num_threads);
    } else {
        num_threads = omp_get_max_threads();
    }
    
    utils::print_result("Number of threads", num_threads);
    utils::print_result("Array size", array_size);
    
    // Compute the correct sum sequentially
    int correct_sum = 0;
    for (int i = 0; i < array_size; i++) {
        correct_sum += array[i];
    }
    
    utils::print_result("Correct sum", correct_sum);
    
    // With race condition
    int sum_with_race = 0;
    utils::Timer timer;
    timer.start();
    
    #pragma omp parallel
    {
        #pragma omp for
        for (int i = 0; i < array_size; i++) {
            sum_with_race += array[i]; // Race condition here!
        }
    }
    
    timer.stop();
    
    // Print results
    utils::print_result("Sum with race condition", sum_with_race);
    utils::print_result("Error", std::abs(correct_sum - sum_with_race));
    utils::print_result("Execution time (race)", timer.elapsed_ms(), "ms");
    
    std::cout << "\nThe race condition causes the sum to be incorrect because\n";
    std::cout << "multiple threads may update the sum variable simultaneously,\n";
    std::cout << "leading to lost updates.\n";
}

// Main race conditions demo 
void demo_race_conditions(int num_threads, int workload) {
    utils::print_header("Race Conditions Demonstration");
    std::cout << "This demo shows different types of race conditions that can occur\n";
    std::cout << "when multiple threads access shared data without proper synchronization.\n\n";
    
    // If threads not specified, use all available
    if (num_threads <= 0) {
        num_threads = omp_get_max_threads();
    }
    
    // Scale workload based on demo
    int counter_iterations = std::min(workload, 10000000);
    int array_size = std::min(workload, 1000000);
    
    // Run the three race condition demos
    race_condition_counter(num_threads, counter_iterations);
    std::cout << "\n" << std::string(50, '-') << "\n\n";
    
    race_condition_array(num_threads, array_size);
    std::cout << "\n" << std::string(50, '-') << "\n\n";
    
    race_condition_sum(num_threads, array_size);
    
    std::cout << "\nRace conditions are one of the most common and difficult bugs in parallel programming.\n";
    std::cout << "They occur when multiple threads access shared data simultaneously without proper synchronization.\n";
    std::cout << "OpenMP provides various synchronization mechanisms to prevent race conditions, which we'll explore in the next demos.\n";
} 