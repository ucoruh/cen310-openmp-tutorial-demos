#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <algorithm>
#include <omp.h>
#include "../include/synchronization_demos.h"
#include "../include/utils.h"

// Display the main menu and get user's choice
int show_main_menu(const std::vector<std::pair<std::string, std::vector<DemoMenuItem>>>& demo_groups) {
    utils::clear_console();
    utils::print_header("OpenMP Synchronization Mechanisms Demo");
    std::cout << "Available demo groups:\n\n";

    int option_num = 1;
    std::map<int, std::pair<std::string, DemoFunction>> option_map;

    for (const auto& group : demo_groups) {
        utils::print_section(group.first);
        
        for (const auto& demo : group.second) {
            std::cout << std::setw(3) << option_num << ". " << demo.name << "\n";
            std::cout << "    " << demo.description << "\n\n";
            
            option_map[option_num] = {demo.name, demo.function};
            option_num++;
        }
    }

    std::cout << std::setw(3) << 0 << ". Exit\n\n";
    
    int choice;
    std::cout << "Enter your choice (0-" << option_num - 1 << "): ";
    std::cin >> choice;
    
    // İlk adımda, sıfır seçilirse hemen çıkış yap
    if (choice == 0) {
        std::cout << "You selected Exit. Goodbye!\n";
        return 999; // Özel çıkış kodu - doğrudan ana fonksiyona taşınacak
    }
    
    if (choice < 0 || choice >= option_num) {
        std::cout << "Invalid choice. Please try again.\n";
        utils::pause_console();
        return -1;
    }
    
    utils::clear_console();
    utils::print_header("Running: " + option_map[choice].first);
    
    // Configure parameters
    int num_threads = 0;
    int workload = 1000000;
    
    std::cout << "Enter number of threads (0 for default/max): ";
    std::cin >> num_threads;
    
    std::cout << "Enter workload size (default: 1000000): ";
    std::cin >> workload;
    
    if (workload <= 0) {
        workload = 1000000;
    }
    
    try {
        // Execute the selected demo function
        option_map[choice].second(num_threads, workload);
    }
    catch (const std::exception& e) {
        std::cerr << "Error executing demo: " << e.what() << std::endl;
    }
    
    utils::pause_console();
    return 1; // Continue
}

// Implementation of the get_all_demos function declared in the header
std::vector<std::pair<std::string, std::vector<DemoMenuItem>>> get_all_demos() {
    std::vector<std::pair<std::string, std::vector<DemoMenuItem>>> demo_groups;
    
    // Race Conditions group
    demo_groups.push_back({
        "Race Conditions",
        {
            {
                "Basic Race Condition Demo",
                "Demonstrates problems with unsynchronized parallel access to shared data",
                demo_race_conditions
            }
        }
    });
    
    // Critical Sections group
    demo_groups.push_back({
        "Critical Sections",
        {
            {
                "Basic Critical Sections",
                "Shows how to use #pragma omp critical to protect shared data",
                demo_critical_sections
            },
            {
                "Named Critical Sections",
                "Demonstrates using named critical sections for fine-grained control",
                demo_named_critical_sections
            },
            {
                "Nested Critical Sections",
                "Shows how nested critical sections work and potential issues",
                demo_nested_critical_sections
            },
            {
                "Critical Sections Performance",
                "Benchmark the performance overhead of critical sections",
                benchmark_critical_sections
            }
        }
    });
    
    // Atomic Operations group
    demo_groups.push_back({
        "Atomic Operations",
        {
            {
                "Atomic Operations Overview",
                "Overview of different atomic operations in OpenMP",
                demo_atomic_operations
            },
            {
                "Atomic Update",
                "Using #pragma omp atomic update for atomic updates",
                demo_atomic_update
            },
            {
                "Atomic Read",
                "Using #pragma omp atomic read for atomic reads",
                demo_atomic_read
            },
            {
                "Atomic Write",
                "Using #pragma omp atomic write for atomic writes",
                demo_atomic_write
            },
            {
                "Atomic Capture",
                "Using #pragma omp atomic capture for read-modify-write operations",
                demo_atomic_capture
            },
            {
                "Atomic vs Critical Performance",
                "Compare performance of atomic operations vs critical sections",
                benchmark_atomic_vs_critical
            }
        }
    });
    
    // Locks group
    demo_groups.push_back({
        "Lock Mechanisms",
        {
            {
                "Lock Mechanisms Overview",
                "Overview of OpenMP lock mechanisms",
                demo_locks
            },
            {
                "Simple Locks",
                "Using omp_set_lock and omp_unset_lock",
                demo_simple_locks
            },
            {
                "Nested Locks",
                "Using nested locks (omp_set_nest_lock and omp_unset_nest_lock)",
                demo_nested_locks
            },
            {
                "Lock Hints",
                "Using lock hints for performance optimization",
                demo_lock_hints
            },
            {
                "Reader-Writer Locks",
                "Implementing reader-writer locks for parallel access patterns",
                demo_reader_writer_locks
            }
        }
    });
    
    // Barriers group
    demo_groups.push_back({
        "Barriers",
        {
            {
                "Barriers Overview",
                "Overview of barrier synchronization in OpenMP",
                demo_barriers
            },
            {
                "Implicit Barriers",
                "Understanding implicit barriers in OpenMP constructs",
                demo_implicit_barriers
            },
            {
                "Explicit Barriers",
                "Using #pragma omp barrier for explicit synchronization",
                demo_explicit_barriers
            },
            {
                "Barrier Performance",
                "Measure the performance impact of barriers",
                benchmark_barrier_performance
            }
        }
    });
    
    // Ordered construct group
    demo_groups.push_back({
        "Ordered Execution",
        {
            {
                "Ordered Construct",
                "Using #pragma omp ordered for sequentially ordered execution",
                demo_ordered
            },
            {
                "Ordered vs Unordered Execution",
                "Performance comparison of ordered and unordered execution",
                ordered_vs_unordered
            }
        }
    });
    
    // Master and Single constructs group
    demo_groups.push_back({
        "Master & Single Constructs",
        {
            {
                "Master & Single Overview",
                "Comparison of master and single constructs",
                demo_master_single
            },
            {
                "Master vs Single Performance",
                "Performance comparison of master and single constructs",
                compare_master_single
            }
        }
    });
    
    // Flush directive group
    demo_groups.push_back({
        "Memory Consistency",
        {
            {
                "Flush Directive",
                "Using #pragma omp flush for memory consistency",
                demo_flush
            },
            {
                "Memory Consistency Visualization",
                "Visual demonstration of memory consistency issues",
                demo_memory_consistency
            }
        }
    });
    
    // Visualization utilities group
    demo_groups.push_back({
        "Visualization Tools",
        {
            {
                "Thread Timeline",
                "Visualize thread execution timeline",
                visualize_thread_timeline
            },
            {
                "Lock Contention",
                "Visualize lock contention between threads",
                visualize_lock_contention
            },
            {
                "Memory Consistency Visualization",
                "Visualize memory state during parallel execution",
                visualize_memory_consistency
            }
        }
    });
    
    // Performance Analysis group
    demo_groups.push_back({
        "Performance Analysis",
        {
            {
                "Synchronization Overhead Analysis",
                "Comprehensive analysis of synchronization mechanisms overhead",
                run_performance_analysis
            },
            {
                "Run All Benchmarks",
                "Run all benchmarks and generate a comprehensive report",
                run_benchmarks
            }
        }
    });
    
    return demo_groups;
}

int main(int argc, char* argv[]) {
    // Parse command line arguments
    utils::ProgramOptions options = utils::parse_command_line(argc, argv);
    
    // Show help if requested
    if (options.show_help) {
        utils::show_help_message(argv[0]);
        return 0;
    }
    
    // Run in benchmark mode if requested
    if (options.benchmark_mode) {
        utils::print_header("OpenMP Synchronization Benchmarks");
        // Kilitlenmeyi önlemek için workload ve thread sayısını sınırlayalım
        int safe_threads = options.num_threads > 0 ? std::min(options.num_threads, 4) : 4;
        int safe_workload = std::min(options.workload, 1000);
        std::cout << "\nRunning all benchmarks with " << safe_threads << " threads and workload size " << safe_workload << "\n";
        run_benchmarks(safe_threads, safe_workload);
        return 0;
    }
    
    // Run in performance analysis mode if requested
    if (options.performance_mode) {
        utils::print_header("OpenMP Synchronization Performance Analysis");
        run_performance_analysis(options.num_threads, options.workload);
        return 0;
    }
    
    // If a specific demo was requested by name
    if (!options.demo_name.empty()) {
        auto all_demos = get_all_demos();
        for (const auto& group : all_demos) {
            for (const auto& demo : group.second) {
                if (demo.name == options.demo_name) {
                    utils::print_header("Running: " + demo.name);
                    demo.function(options.num_threads, options.workload);
                    return 0;
                }
            }
        }
        std::cerr << "Error: Demo '" << options.demo_name << "' not found.\n";
        return 1;
    }
    
    // Interactive menu mode
    auto all_demos = get_all_demos();
    
    // Menüyü göster ve sonucu al
    int menu_result = show_main_menu(all_demos);
    
    // Eğer çıkış seçildiyse (0), özel çıkış kodu döndür
    if (menu_result == 999) {
        return 999;
    }
    
    // Geçersiz seçim veya başarılı demo çalıştırma durumunda normal çıkış yap
    return 0;
} 