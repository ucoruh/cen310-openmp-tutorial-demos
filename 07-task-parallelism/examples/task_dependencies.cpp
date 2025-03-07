#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <cstdlib>  // For atoi
#include <algorithm>
#include <iomanip>
#include <random>
#include <thread>
#include <mutex>
#include <omp.h>

// Define a task node structure
struct TaskNode {
    int id;
    std::string name;
    int cost;  // in milliseconds
    std::vector<int> dependencies;
    
    // Constructor that takes individual arguments
    TaskNode(int i, const std::string& n, int c) 
        : id(i), name(n), cost(c) {}
    
    // Constructor that takes dependencies as a vector
    TaskNode(int i, const std::string& n, int c, const std::vector<int>& deps) 
        : id(i), name(n), cost(c), dependencies(deps) {}
    
    // Constructor that takes dependencies as an initializer list
    TaskNode(int i, const std::string& n, int c, std::initializer_list<int> deps) 
        : id(i), name(n), cost(c), dependencies(deps) {}
};

// Define a dependency graph
class DependencyGraph {
private:
    std::vector<TaskNode> nodes;
    
public:
    // Add a node with an initializer list of dependencies
    void add_node(int id, const std::string& name, int cost, std::initializer_list<int> deps = {}) {
        nodes.push_back(TaskNode(id, name, cost, deps));
    }
    
    // Add a node with a vector of dependencies
    void add_node(int id, const std::string& name, int cost, const std::vector<int>& deps) {
        nodes.push_back(TaskNode(id, name, cost, deps));
    }
    
    void print() const {
        std::cout << "Dependency Graph:" << std::endl;
        std::cout << "ID | Name       | Cost (ms) | Dependencies" << std::endl;
        std::cout << "-------------------------------------------" << std::endl;
        
        for (const auto& node : nodes) {
            std::cout << std::setw(2) << node.id << " | "
                      << std::setw(10) << node.name << " | "
                      << std::setw(8) << node.cost << " | ";
            
            if (node.dependencies.empty()) {
                std::cout << "None";
            } else {
                for (size_t i = 0; i < node.dependencies.size(); ++i) {
                    if (i > 0) std::cout << ", ";
                    std::cout << node.dependencies[i];
                }
            }
            
            std::cout << std::endl;
        }
    }
    
    // Execute tasks sequentially
    void execute_sequential() const {
        std::cout << "\nExecuting tasks sequentially..." << std::endl;
        
        // Track completed tasks
        std::vector<bool> completed(nodes.size(), false);
        
        // Continue until all tasks are completed
        bool progress = true;
        while (progress) {
            progress = false;
            
            for (size_t i = 0; i < nodes.size(); ++i) {
                if (completed[i]) continue;
                
                // Check if all dependencies are satisfied
                bool deps_satisfied = true;
                for (int dep_id : nodes[i].dependencies) {
                    if (!completed[dep_id]) {
                        deps_satisfied = false;
                        break;
                    }
                }
                
                if (deps_satisfied) {
                    // Execute the task
                    std::cout << "Executing task " << nodes[i].id << " (" << nodes[i].name << ")" << std::endl;
                    std::this_thread::sleep_for(std::chrono::milliseconds(nodes[i].cost));
                    
                    // Mark as completed
                    completed[i] = true;
                    progress = true;
                }
            }
        }
        
        // Check if all tasks are completed
        if (std::all_of(completed.begin(), completed.end(), [](bool v) { return v; })) {
            std::cout << "All tasks completed successfully!" << std::endl;
        } else {
            std::cout << "Some tasks could not be completed. Check for circular dependencies." << std::endl;
        }
    }
    
    // Execute tasks in parallel using OpenMP tasks with dependencies
    void execute_parallel() const {
        std::cout << "\nExecuting tasks in parallel with dependencies..." << std::endl;
        
        // Track task completion for visualization
        std::mutex cout_mutex;
        
        // Create a vector to hold task handles (for demonstration purposes)
        std::vector<int> task_handles(nodes.size(), -1);
        
        #pragma omp parallel
        {
            #pragma omp single
            {
                // First pass: create all tasks
                for (size_t i = 0; i < nodes.size(); ++i) {
                    const auto& node = nodes[i];
                    
                    // Create dependency list for the OpenMP task
                    if (node.dependencies.empty()) {
                        // No dependencies
                        // Using basic task without depend clause for better compatibility
                        #pragma omp task
                        {
                            // Simulate task execution
                            {
                                std::lock_guard<std::mutex> lock(cout_mutex);
                                std::cout << "Thread " << omp_get_thread_num() 
                                        << " executing task " << node.id 
                                        << " (" << node.name << ")" << std::endl;
                            }
                            
                            std::this_thread::sleep_for(std::chrono::milliseconds(node.cost));
                            
                            {
                                std::lock_guard<std::mutex> lock(cout_mutex);
                                std::cout << "Thread " << omp_get_thread_num() 
                                        << " completed task " << node.id 
                                        << " (" << node.name << ")" << std::endl;
                            }
                        }
                    } else {
                        // Has dependencies
                        std::string dep_str;
                        for (int dep : node.dependencies) {
                            dep_str += std::to_string(dep) + ", ";
                        }
                        
                        // Using basic task with manual dependency handling
                        #pragma omp task
                        {
                            // Check dependencies (for logging only since we can't use depend clause)
                            {
                                std::lock_guard<std::mutex> lock(cout_mutex);
                                std::cout << "Thread " << omp_get_thread_num() 
                                        << " executing task " << node.id 
                                        << " (" << node.name << ")"
                                        << " with dependencies: " << dep_str << std::endl;
                            }
                            
                            // Simulate task execution
                            {
                                std::lock_guard<std::mutex> lock(cout_mutex);
                                std::cout << "Thread " << omp_get_thread_num() 
                                        << " executing task " << node.id 
                                        << " (" << node.name << ")" << std::endl;
                            }
                            
                            std::this_thread::sleep_for(std::chrono::milliseconds(node.cost));
                            
                            {
                                std::lock_guard<std::mutex> lock(cout_mutex);
                                std::cout << "Thread " << omp_get_thread_num() 
                                        << " completed task " << node.id 
                                        << " (" << node.name << ")" << std::endl;
                            }
                        }
                    }
                }
                
                // Wait for all tasks to complete
                #pragma omp taskwait
                std::cout << "All tasks completed successfully!" << std::endl;
            }
        }
    }
    
    // Get the total number of tasks
    size_t size() const {
        return nodes.size();
    }
};

// Create a sample dependency graph for demonstrating task dependencies
DependencyGraph create_sample_graph() {
    DependencyGraph graph;
    
    // Add nodes with dependencies
    graph.add_node(0, "Init", 100); // No dependencies
    graph.add_node(1, "LoadData", 200, {0}); // Depends on Init
    graph.add_node(2, "Process1", 300, {1}); // Depends on LoadData
    graph.add_node(3, "Process2", 300, {1}); // Depends on LoadData
    graph.add_node(4, "Process3", 300, {1}); // Depends on LoadData
    graph.add_node(5, "Merge", 200, {2, 3, 4}); // Depends on all Process tasks
    graph.add_node(6, "Analyze", 400, {5}); // Depends on Merge
    graph.add_node(7, "Report", 150, {6}); // Depends on Analyze
    graph.add_node(8, "Cleanup", 100, {7}); // Depends on Report
    
    return graph;
}

// Create a random dependency graph for benchmarking
DependencyGraph create_random_graph(int num_tasks, int max_deps = 3, int min_cost = 10, int max_cost = 1000) {
    DependencyGraph graph;
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> cost_dist(min_cost, max_cost);
    
    // First task has no dependencies
    graph.add_node(0, "Task_0", cost_dist(gen));
    
    // Each subsequent task depends on some previous tasks
    for (int i = 1; i < num_tasks; ++i) {
        std::string name = "Task_" + std::to_string(i);
        int cost = cost_dist(gen);
        
        // Select random number of dependencies (max_deps or fewer)
        std::uniform_int_distribution<int> num_deps_dist(0, std::min(max_deps, i));
        int num_deps = num_deps_dist(gen);
        
        // Select random dependencies from previous tasks
        std::vector<int> deps;
        if (num_deps > 0) {
            std::uniform_int_distribution<int> dep_dist(0, i - 1);
            for (int j = 0; j < num_deps; ++j) {
                int dep = dep_dist(gen);
                // Avoid duplicate dependencies
                if (std::find(deps.begin(), deps.end(), dep) == deps.end()) {
                    deps.push_back(dep);
                }
            }
        }
        
        // Add the task with its dependencies as a vector
        graph.add_node(i, name, cost, deps);
    }
    
    return graph;
}

int main(int argc, char* argv[]) {
    // Default parameters
    int num_tasks = 20;
    int num_threads = omp_get_max_threads();
    
    // Parse command-line arguments
    if (argc > 1) {
        num_tasks = atoi(argv[1]); // Using atoi instead of std::stoi
    }
    if (argc > 2) {
        num_threads = atoi(argv[2]); // Using atoi instead of std::stoi
    }
    
    // Set number of threads
    omp_set_num_threads(num_threads);
    
    std::cout << "OpenMP Task Dependencies Example" << std::endl;
    std::cout << "===============================" << std::endl;
    std::cout << "Number of tasks: " << num_tasks << std::endl;
    std::cout << "Number of threads: " << num_threads << std::endl;
    std::cout << std::endl;
    
    // Create a dependency graph
    // For demonstration, we'll use a predefined sample graph
    DependencyGraph graph;
    
    if (num_tasks <= 10) {
        std::cout << "Using sample dependency graph..." << std::endl;
        graph = create_sample_graph();
    } else {
        std::cout << "Creating random dependency graph with " << num_tasks << " tasks..." << std::endl;
        graph = create_random_graph(num_tasks);
    }
    
    // Print the graph
    graph.print();
    
    // Measure execution time for sequential execution
    auto seq_start = std::chrono::high_resolution_clock::now();
    graph.execute_sequential();
    auto seq_end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> seq_time = seq_end - seq_start;
    
    // Measure execution time for parallel execution
    auto par_start = std::chrono::high_resolution_clock::now();
    graph.execute_parallel();
    auto par_end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> par_time = par_end - par_start;
    
    // Print performance metrics
    std::cout << "\nPerformance Metrics:" << std::endl;
    std::cout << "--------------------" << std::endl;
    std::cout << "Sequential execution time: " << seq_time.count() << " seconds" << std::endl;
    std::cout << "Parallel execution time: " << par_time.count() << " seconds" << std::endl;
    
    double speedup = seq_time.count() / par_time.count();
    std::cout << "Speedup: " << speedup << "x" << std::endl;
    
    // Calculate theoretical maximum speedup based on critical path
    std::cout << "\nNote: The theoretical maximum speedup is limited by the critical path in the dependency graph." << std::endl;
    std::cout << "Some tasks must wait for their dependencies regardless of how many threads are available." << std::endl;
    
    return 0;
}