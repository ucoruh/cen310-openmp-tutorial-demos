#pragma once

#include <string>
#include <vector>
#include <functional>

// Demo function signature
using DemoFunction = std::function<void(int, int)>;

// Race conditions demo
void demo_race_conditions(int num_threads, int workload);

// Critical sections demos
void demo_critical_sections(int num_threads, int workload);
void demo_named_critical_sections(int num_threads, int workload);
void demo_nested_critical_sections(int num_threads, int workload);
void benchmark_critical_sections(int num_threads, int workload);

// Atomic operations demos
void demo_atomic_operations(int num_threads, int workload);
void demo_atomic_update(int num_threads, int workload);
void demo_atomic_read(int num_threads, int workload);
void demo_atomic_write(int num_threads, int workload);
void demo_atomic_capture(int num_threads, int workload);
void benchmark_atomic_vs_critical(int num_threads, int workload);

// Locks demos
void demo_locks(int num_threads, int workload);
void demo_simple_locks(int num_threads, int workload);
void demo_nested_locks(int num_threads, int workload);
void demo_lock_hints(int num_threads, int workload);
void demo_reader_writer_locks(int num_threads, int workload);

// Barriers demos
void demo_barriers(int num_threads, int workload);
void demo_implicit_barriers(int num_threads, int workload);
void demo_explicit_barriers(int num_threads, int workload);
void benchmark_barrier_performance(int num_threads, int workload);

// Ordered construct demos
void demo_ordered(int num_threads, int workload);
void ordered_vs_unordered(int num_threads, int workload);

// Master and single constructs demos
void demo_master_single(int num_threads, int workload);
void compare_master_single(int num_threads, int workload);

// Flush directive demos
void demo_flush(int num_threads, int workload);
void demo_memory_consistency(int num_threads, int workload);

// Visualization utilities
void visualize_thread_timeline(int num_threads, int workload);
void visualize_lock_contention(int num_threads, int workload);
void visualize_memory_consistency(int num_threads, int workload);

// Performance analysis
void run_performance_analysis(int num_threads, int workload);
void generate_performance_report(const std::string& filename);

// Benchmark mode
void run_benchmarks(int num_threads, int workload);

// Structure representing a demo menu item
struct DemoMenuItem {
    std::string name;
    std::string description;
    DemoFunction function;
};

// Get all demos organized in groups
std::vector<std::pair<std::string, std::vector<DemoMenuItem>>> get_all_demos(); 