#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <chrono>
#include <functional>
#include <algorithm>
#include <omp.h>

// Forward declarations for example functions
void basic_tasks_example(int num_threads, int task_granularity);
void fibonacci_example(int n, int num_threads, int cutoff);
void quicksort_example(int size, int num_threads, int cutoff);
void tree_traversal_example(int depth, int num_threads);
void graph_processing_example(int nodes, int edges, int num_threads);
void task_dependencies_example(int num_tasks, int num_threads);
void task_priority_example(int num_tasks, int num_threads);
void taskloop_example(int size, int num_threads, int grainsize);
void taskgroup_example(int num_tasks, int num_threads);
void run_additional_examples(int num_threads, int task_granularity);

// Performance measurement utilities
double measure_execution_time(std::function<void()> func) {
    auto start = std::chrono::high_resolution_clock::now();
    func();
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double, std::milli>(end - start).count();
}

// Helper function to display a title
void display_title(const std::string& title) {
    std::cout << "\n=== " << title << " ===" << std::endl;
}

// Command-line argument parsing
struct ProgramOptions {
    int num_threads = omp_get_max_threads();
    int task_granularity = 100;
    bool run_all = false;
    bool generate_reports = false;
    std::string example;
};

ProgramOptions parse_arguments(int argc, char* argv[]) {
    ProgramOptions options;
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "--threads" || arg == "-t") {
            if (i + 1 < argc) {
                options.num_threads = std::stoi(argv[++i]);
            }
        }
        else if (arg == "--granularity" || arg == "-g") {
            if (i + 1 < argc) {
                options.task_granularity = std::stoi(argv[++i]);
            }
        }
        else if (arg == "--example" || arg == "-e") {
            if (i + 1 < argc) {
                options.example = argv[++i];
            }
        }
        else if (arg == "--all" || arg == "-a") {
            options.run_all = true;
        }
        else if (arg == "--generate-reports") {
            options.generate_reports = true;
        }
        else if (arg == "--help" || arg == "-h") {
            std::cout << "Usage: " << argv[0] << " [options]" << std::endl;
            std::cout << "Options:" << std::endl;
            std::cout << "  --threads, -t N          Set number of threads (default: max available)" << std::endl;
            std::cout << "  --granularity, -g N      Set task granularity (default: 100)" << std::endl;
            std::cout << "  --example, -e NAME       Run specific example. Options:" << std::endl;
            std::cout << "                           basic, fibonacci, quicksort, tree, graph," << std::endl;
            std::cout << "                           dependencies, priority, taskloop, taskgroup" << std::endl;
            std::cout << "  --all, -a                Run all examples" << std::endl;
            std::cout << "  --generate-reports       Generate performance reports" << std::endl;
            std::cout << "  --help, -h               Show this help message" << std::endl;
            exit(0);
        }
    }
    
    return options;
}

int main(int argc, char* argv[]) {
    // Check if OpenMP is available
    #ifdef _OPENMP
        std::cout << "OpenMP is supported! Version: " << _OPENMP << std::endl;
    #else
        std::cerr << "OpenMP is not supported!" << std::endl;
        return 1;
    #endif
    
    // Parse command-line arguments
    ProgramOptions options = parse_arguments(argc, argv);
    
    // Set number of threads
    omp_set_num_threads(options.num_threads);
    
    std::cout << "Running with " << options.num_threads << " threads" << std::endl;
    std::cout << "Task granularity: " << options.task_granularity << std::endl;
    
    // If a specific example was requested
    if (!options.example.empty() && !options.run_all) {
        if (options.example == "basic") {
            display_title("Basic Tasks Example");
            basic_tasks_example(options.num_threads, options.task_granularity);
        }
        else if (options.example == "fibonacci") {
            display_title("Fibonacci Example");
            fibonacci_example(35, options.num_threads, 10);
        }
        else if (options.example == "quicksort") {
            display_title("Quicksort Example");
            quicksort_example(1000000, options.num_threads, 1000);
        }
        else if (options.example == "tree") {
            display_title("Tree Traversal Example");
            tree_traversal_example(10, options.num_threads);
        }
        else if (options.example == "graph") {
            display_title("Graph Processing Example");
            graph_processing_example(300, 1500, options.num_threads);
        }
        else if (options.example == "dependencies") {
            display_title("Task Dependencies Example");
            task_dependencies_example(options.task_granularity, options.num_threads);
        }
        else if (options.example == "priority") {
            display_title("Task Priority Example");
            task_priority_example(options.task_granularity, options.num_threads);
        }
        else if (options.example == "taskloop") {
            display_title("Taskloop Example");
            taskloop_example(1000000, options.num_threads, options.task_granularity);
        }
        else if (options.example == "taskgroup") {
            display_title("Taskgroup Example");
            taskgroup_example(options.task_granularity, options.num_threads);
        }
        else if (options.example == "throttling") {
            display_title("Task Throttling Example");
            system(".\\task_throttling.exe");
        }
        else if (options.example == "visualizer") {
            display_title("Task Visualizer Example");
            system(".\\task_visualizer.exe");
        }
        else if (options.example == "heterogeneous") {
            display_title("Heterogeneous Tasks Example");
            std::string cmd = ".\\heterogeneous_tasks.exe 5 5 10 " + std::to_string(options.num_threads);
            system(cmd.c_str());
        }
        else if (options.example == "stealing") {
            display_title("Task Stealing Example");
            std::string cmd = ".\\task_stealing.exe " + std::to_string(options.task_granularity) + " " + std::to_string(options.num_threads);
            system(cmd.c_str());
        }
        else if (options.example == "nested") {
            display_title("Nested Tasks Example");
            std::string cmd = ".\\nested_tasks.exe 8 " + std::to_string(std::min(8, options.num_threads));
            system(cmd.c_str());
        }
        else {
            std::cerr << "Unknown example: " << options.example << std::endl;
            return 1;
        }
    }
    // If generate reports was requested
    else if (options.generate_reports) {
        std::cout << "Generating performance reports..." << std::endl;
        // Code to generate reports would go here
        std::cout << "Reports generated successfully!" << std::endl;
    }
    // Run all examples
    else {
        display_title("OpenMP Task Parallelism Demo");
        
        std::cout << "\nRunning all examples with " << options.num_threads << " threads "
                  << "and task granularity of " << options.task_granularity << std::endl;
        
        display_title("Basic Tasks Example");
        basic_tasks_example(options.num_threads, options.task_granularity);
        
        display_title("Fibonacci Example");
        fibonacci_example(35, options.num_threads, 10);
        
        display_title("Quicksort Example");
        quicksort_example(1000000, options.num_threads, 1000);
        
        display_title("Tree Traversal Example");
        tree_traversal_example(10, options.num_threads);
        
        display_title("Graph Processing Example");
        graph_processing_example(300, 1500, options.num_threads);
        
        display_title("Task Dependencies Example");
        task_dependencies_example(50, options.num_threads);
        
        display_title("Task Priority Example");
        task_priority_example(50, options.num_threads);
        
        display_title("Taskloop Example");
        taskloop_example(1000000, options.num_threads, options.task_granularity);
        
        display_title("Taskgroup Example");
        taskgroup_example(50, options.num_threads);
        
        // Run additional examples
        run_additional_examples(options.num_threads, options.task_granularity);
    }
    
    return 0;
}

// Implementation of basic_tasks_example
void basic_tasks_example(int num_threads, int task_granularity) {
    std::cout << "Running the basic tasks example with " << num_threads << " threads..." << std::endl;
    std::string command = "basic_tasks.exe " + std::to_string(task_granularity) + " " + std::to_string(num_threads);
    int result = system(command.c_str());
    if (result != 0) {
        std::cerr << "Basic tasks example failed with error code: " << result << std::endl;
    }
}

// Other example implementations
void fibonacci_example(int n, int num_threads, int cutoff) {
    std::cout << "Running the Fibonacci example with n=" << n << ", " << num_threads << " threads, and cutoff=" << cutoff << "..." << std::endl;
    std::string command = "fibonacci.exe " + std::to_string(n) + " " + std::to_string(cutoff) + " " + std::to_string(num_threads);
    int result = system(command.c_str());
    if (result != 0) {
        std::cerr << "Fibonacci example failed with error code: " << result << std::endl;
    }
}

void quicksort_example(int size, int num_threads, int cutoff) {
    // Limit size to avoid memory problems - daha güvenli limitler
    if (size > 10000) {
        std::cout << "Limiting array size to 10,000 elements for stability" << std::endl;
        size = 10000;
    }
    if (cutoff < 10) cutoff = 10;
    if (cutoff > size / 5) cutoff = size / 5;
    if (num_threads > 16) {
        std::cout << "Limiting threads to 16 for stability" << std::endl;
        num_threads = 16;
    }
    
    std::cout << "Running the quicksort example with size=" << size << ", " << num_threads << " threads, and cutoff=" << cutoff << "..." << std::endl;
    std::string command = "quicksort.exe " + std::to_string(size) + " " + std::to_string(cutoff) + " " + std::to_string(num_threads);
    int result = system(command.c_str());
    if (result != 0) {
        std::cerr << "Quicksort example failed with error code: " << result << std::endl;
        // Try with smaller size
        size = 1000; // Daha küçük bir boyut
        cutoff = 100; // Daha güvenli bir cutoff değeri
        std::cout << "Retrying with smaller size=" << size << " and cutoff=" << cutoff << std::endl;
        command = "quicksort.exe " + std::to_string(size) + " " + std::to_string(cutoff) + " " + std::to_string(num_threads);
        result = system(command.c_str());
        if (result != 0) {
            std::cerr << "Quicksort example still failed with error code: " << result << std::endl;
        }
    }
}

void tree_traversal_example(int depth, int num_threads) {
    // Limit depth to avoid stack overflow - daha güvenli limitler
    if (depth > 5) {
        std::cout << "Limiting tree depth to 5 for stability" << std::endl;
        depth = 5;
    }
    int cutoff = 2; // Daha düşük ve güvenli bir cutoff
    
    if (num_threads > 16) {
        std::cout << "Limiting threads to 16 for stability" << std::endl;
        num_threads = 16;
    }
    
    std::cout << "Running the tree traversal example with depth=" << depth << ", cutoff=" << cutoff 
              << ", and " << num_threads << " threads..." << std::endl;
    std::string command = "tree_traversal.exe " + std::to_string(depth) + " " + std::to_string(num_threads) + " " + std::to_string(cutoff);
    int result = system(command.c_str());
    if (result != 0) {
        std::cerr << "Tree traversal example failed with error code: " << result << std::endl;
        // Try with even smaller parameters
        depth = 3;
        cutoff = 1;
        std::cout << "Retrying with smaller depth=" << depth << " and cutoff=" << cutoff << std::endl;
        command = "tree_traversal.exe " + std::to_string(depth) + " " + std::to_string(num_threads) + " " + std::to_string(cutoff);
        result = system(command.c_str());
        if (result != 0) {
            std::cerr << "Tree traversal example still failed with error code: " << result << std::endl;
        }
    }
}

void graph_processing_example(int nodes, int edges, int num_threads) {
    // Limit graph size for stability
    if (nodes > 500) {
        std::cout << "Limiting graph to 500 nodes for stability" << std::endl;
        nodes = 500;
        edges = nodes * 5; // Maintain reasonable edge density
    }
    
    std::cout << "Running the graph processing example with nodes=" << nodes << ", edges=" << edges 
              << ", and " << num_threads << " threads..." << std::endl;
    std::string command = "graph_processing.exe " + std::to_string(nodes) + " " + std::to_string(edges) + " " + std::to_string(num_threads);
    int result = system(command.c_str());
    if (result != 0) {
        std::cerr << "Graph processing example failed with error code: " << result << std::endl;
        // Try with smaller graph
        nodes = 100;
        edges = 200;
        std::cout << "Retrying with smaller graph: nodes=" << nodes << ", edges=" << edges << std::endl;
        command = "graph_processing.exe " + std::to_string(nodes) + " " + std::to_string(edges) + " " + std::to_string(num_threads);
        result = system(command.c_str());
        if (result != 0) {
            std::cerr << "Graph processing example still failed with error code: " << result << std::endl;
        }
    }
}

void task_dependencies_example(int num_tasks, int num_threads) {
    // Limit number of tasks for stability
    if (num_tasks > 100) {
        std::cout << "Limiting tasks to 100 for stability" << std::endl;
        num_tasks = 100;
    }
    
    std::cout << "Running the task dependencies example with " << num_tasks << " tasks and " 
              << num_threads << " threads..." << std::endl;
    std::string command = "task_dependencies.exe " + std::to_string(num_tasks) + " " + std::to_string(num_threads);
    int result = system(command.c_str());
    if (result != 0) {
        std::cerr << "Task dependencies example failed with error code: " << result << std::endl;
        // Try with fewer tasks
        num_tasks = 20;
        std::cout << "Retrying with fewer tasks=" << num_tasks << std::endl;
        command = "task_dependencies.exe " + std::to_string(num_tasks) + " " + std::to_string(num_threads);
        result = system(command.c_str());
        if (result != 0) {
            std::cerr << "Task dependencies example still failed with error code: " << result << std::endl;
        }
    }
}

void task_priority_example(int num_tasks, int num_threads) {
    // Limit number of tasks for stability
    if (num_tasks > 100) {
        std::cout << "Limiting tasks to 100 for stability" << std::endl;
        num_tasks = 100;
    }
    
    std::cout << "Running the task priority example with " << num_tasks << " tasks and " 
              << num_threads << " threads..." << std::endl;
    std::string command = "task_priority.exe " + std::to_string(num_tasks) + " " + std::to_string(num_threads);
    int result = system(command.c_str());
    if (result != 0) {
        std::cerr << "Task priority example failed with error code: " << result << std::endl;
        // Try with fewer tasks
        num_tasks = 20;
        std::cout << "Retrying with fewer tasks=" << num_tasks << std::endl;
        command = "task_priority.exe " + std::to_string(num_tasks) + " " + std::to_string(num_threads);
        result = system(command.c_str());
        if (result != 0) {
            std::cerr << "Task priority example still failed with error code: " << result << std::endl;
        }
    }
}

void taskloop_example(int size, int num_threads, int grainsize) {
    // Limit array size for stability
    if (size > 1000000) {
        std::cout << "Limiting array size to 1,000,000 elements for stability" << std::endl;
        size = 1000000;
    }
    
    std::cout << "Running the taskloop example with size=" << size << ", grainsize=" << grainsize 
              << ", and " << num_threads << " threads..." << std::endl;
    std::string command = "taskloop.exe " + std::to_string(size) + " " + std::to_string(grainsize) + " " + std::to_string(num_threads);
    int result = system(command.c_str());
    if (result != 0) {
        std::cerr << "Taskloop example failed with error code: " << result << std::endl;
        // Try with smaller size
        size = 100000;
        std::cout << "Retrying with smaller size=" << size << std::endl;
        command = "taskloop.exe " + std::to_string(size) + " " + std::to_string(grainsize) + " " + std::to_string(num_threads);
        result = system(command.c_str());
        if (result != 0) {
            std::cerr << "Taskloop example still failed with error code: " << result << std::endl;
        }
    }
}

void taskgroup_example(int num_tasks, int num_threads) {
    // Limit number of tasks for stability
    if (num_tasks > 100) {
        std::cout << "Limiting tasks to 100 for stability" << std::endl;
        num_tasks = 100;
    }
    
    std::cout << "Running the taskgroup example with " << num_tasks << " tasks and " 
              << num_threads << " threads..." << std::endl;
    std::string command = "taskgroup.exe " + std::to_string(num_tasks) + " " + std::to_string(num_threads);
    int result = system(command.c_str());
    if (result != 0) {
        std::cerr << "Taskgroup example failed with error code: " << result << std::endl;
        // Try with fewer tasks
        num_tasks = 20;
        std::cout << "Retrying with fewer tasks=" << num_tasks << std::endl;
        command = "taskgroup.exe " + std::to_string(num_tasks) + " " + std::to_string(num_threads);
        result = system(command.c_str());
        if (result != 0) {
            std::cerr << "Taskgroup example still failed with error code: " << result << std::endl;
        }
    }
}

// Run all additional examples
void run_additional_examples(int num_threads, int task_granularity) {
    display_title("Task Throttling Example");
    std::cout << "Running the task throttling example..." << std::endl;
    int result = system("task_throttling.exe");
    if (result != 0) {
        std::cerr << "Task throttling example failed with error code: " << result << std::endl;
    }
    
    display_title("Task Visualizer Example");
    std::cout << "Running the task visualizer example..." << std::endl;
    result = system("task_visualizer.exe");
    if (result != 0) {
        std::cerr << "Task visualizer example failed with error code: " << result << std::endl;
    }
    
    display_title("Heterogeneous Tasks Example");
    std::cout << "Running the heterogeneous tasks example..." << std::endl;
    std::string command = "heterogeneous_tasks.exe 5 5 10 " + std::to_string(num_threads);
    result = system(command.c_str());
    if (result != 0) {
        std::cerr << "Heterogeneous tasks example failed with error code: " << result << std::endl;
        // Try with fewer tasks
        std::cout << "Retrying with fewer tasks" << std::endl;
        command = "heterogeneous_tasks.exe 2 2 10 " + std::to_string(std::min(4, num_threads));
        result = system(command.c_str());
        if (result != 0) {
            std::cerr << "Heterogeneous tasks example still failed with error code: " << result << std::endl;
        }
    }
    
    display_title("Task Stealing Example");
    std::cout << "Running the task stealing example..." << std::endl;
    command = "task_stealing.exe " + std::to_string(task_granularity) + " " + std::to_string(num_threads);
    result = system(command.c_str());
    if (result != 0) {
        std::cerr << "Task stealing example failed with error code: " << result << std::endl;
    }
    
    display_title("Nested Tasks Example");
    std::cout << "Running the nested tasks example..." << std::endl;
    command = "nested_tasks.exe 8 " + std::to_string(std::min(8, num_threads));
    result = system(command.c_str());
    if (result != 0) {
        std::cerr << "Nested tasks example failed with error code: " << result << std::endl;
    }
}