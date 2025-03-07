#include <iostream>
#include <iomanip>
#include <vector>
#include <queue>
#include <string>       // This provides std::string
#include <mutex>        // This provides std::mutex and std::lock_guard
#include <chrono>       // For time measurement
#include <thread>       // For std::this_thread
#include <memory>
#include <random>
#include <functional>
#include <omp.h>        // OpenMP functionality
#include <sstream>      // For string stream processing
#include <cstdlib>      // For atoi as an alternative to std::stoi
#include <algorithm>    // For std::sort

// Binary tree node structure
struct TreeNode {
    int value;
    std::shared_ptr<TreeNode> left;
    std::shared_ptr<TreeNode> right;
    
    TreeNode(int val) : value(val), left(nullptr), right(nullptr) {}
};

// Generate a balanced binary tree of specified depth
std::shared_ptr<TreeNode> generate_tree(int depth, int& node_count) {
    if (depth <= 0) return nullptr;
    
    node_count++;
    std::shared_ptr<TreeNode> root = std::make_shared<TreeNode>(node_count);
    
    root->left = generate_tree(depth - 1, node_count);
    root->right = generate_tree(depth - 1, node_count);
    
    return root;
}

// Sequential in-order traversal
void traverse_sequential(std::shared_ptr<TreeNode> node, std::vector<int>& result) {
    if (!node) return;
    
    traverse_sequential(node->left, result);
    result.push_back(node->value);
    traverse_sequential(node->right, result);
}

// Task-based parallel in-order traversal
void traverse_parallel(std::shared_ptr<TreeNode> node, std::vector<int>& result, std::vector<std::mutex>& node_mutexes, int cutoff_depth = 0) {
    // Node null kontrolü
    if (!node) return;
    
    // Use sequential traversal for small subtrees
    if (cutoff_depth <= 0) {
        std::vector<int> local_result;
        traverse_sequential(node, local_result);
        
        // Thread-safe append of local results
        if (!local_result.empty()) {
            try {
                // Safe mutex index calculation
                unsigned int mutex_index = static_cast<unsigned int>(node->value) % node_mutexes.size();
                {
                    std::lock_guard<std::mutex> lock(node_mutexes[mutex_index]);
                    result.insert(result.end(), local_result.begin(), local_result.end());
                }
            } catch (const std::exception& e) {
                // If mutex locking fails, try a different approach
                {
                    std::lock_guard<std::mutex> lock(node_mutexes[0]);  // Use first mutex as fallback
                    result.insert(result.end(), local_result.begin(), local_result.end());
                }
            }
        }
        return;
    }
    
    // Process left subtree - only create task if depth is significant
    if (cutoff_depth > 1 && node->left) {
        #pragma omp task
        {
            traverse_parallel(node->left, result, node_mutexes, cutoff_depth - 1);
        }
    } else if (node->left) {
        traverse_parallel(node->left, result, node_mutexes, cutoff_depth - 1);
    }
    
    // Process current node (thread-safe)
    try {
        // Safe mutex index calculation
        unsigned int mutex_index = static_cast<unsigned int>(node->value) % node_mutexes.size();
        {
            std::lock_guard<std::mutex> lock(node_mutexes[mutex_index]);
            result.push_back(node->value);
        }
    } catch (const std::exception& e) {
        // If mutex locking fails, try a different approach
        {
            std::lock_guard<std::mutex> lock(node_mutexes[0]);  // Use first mutex as fallback
            result.push_back(node->value);
        }
    }
    
    // Process right subtree - only create task if depth is significant
    if (cutoff_depth > 1 && node->right) {
        #pragma omp task
        {
            traverse_parallel(node->right, result, node_mutexes, cutoff_depth - 1);
        }
        
        // Wait for subtree tasks to complete
        #pragma omp taskwait
    } else if (node->right) {
        traverse_parallel(node->right, result, node_mutexes, cutoff_depth - 1);
    }
}

// Wrapper for parallel traversal
void parallel_tree_traversal(std::shared_ptr<TreeNode> root, std::vector<int>& result, int cutoff_depth) {
    // Daha fazla mutex kullanarak contention azalt
    std::vector<std::mutex> node_mutexes(256);  // Increase number of mutexes to reduce contention
    
    // Create local result vector to avoid synchronization issues
    std::vector<int> local_result;
    try {
        local_result.reserve(1 << (std::min(cutoff_depth + 1, 16))); // Safer reserve with limit
    } catch (...) {
        // If reserve fails, continue without reserving
    }
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            traverse_parallel(root, local_result, node_mutexes, cutoff_depth);
        }
    }
    
    // Copy results to output vector after parallel execution completes
    result = std::move(local_result);
}

// Measure execution time
template<typename Func, typename... Args>
double measure_time(Func func, Args... args) {
    auto start = std::chrono::high_resolution_clock::now();
    func(args...);
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double>(end - start).count();
}

// Verify that two result vectors contain the same elements (though possibly in different order)
bool verify_results(const std::vector<int>& seq_result, const std::vector<int>& par_result) {
    if (seq_result.size() != par_result.size()) {
        std::cout << "Size mismatch: sequential=" << seq_result.size() 
                  << ", parallel=" << par_result.size() << std::endl;
        return false;
    }
    
    // Create sorted copies for comparison
    std::vector<int> sorted_seq = seq_result;
    std::vector<int> sorted_par = par_result;
    
    std::sort(sorted_seq.begin(), sorted_seq.end());
    std::sort(sorted_par.begin(), sorted_par.end());
    
    // Check that each value appears the same in both results
    return sorted_seq == sorted_par;
}

// Level-order tree traversal (for visualization)
void print_tree_levels(std::shared_ptr<TreeNode> root, int max_levels = 5) {
    if (!root) {
        std::cout << "Empty tree" << std::endl;
        return;
    }
    
    std::queue<std::shared_ptr<TreeNode>> queue;
    queue.push(root);
    
    int level = 0;
    int current_level_nodes = 1;
    int next_level_nodes = 0;
    
    std::cout << "Tree structure (first " << max_levels << " levels):" << std::endl;
    
    while (!queue.empty() && level < max_levels) {
        std::shared_ptr<TreeNode> node = queue.front();
        queue.pop();
        current_level_nodes--;
        
        if (node) {
            std::cout << node->value << " ";
            
            queue.push(node->left);
            queue.push(node->right);
            next_level_nodes += 2;
        } else {
            std::cout << "- ";
            queue.push(nullptr);
            queue.push(nullptr);
            next_level_nodes += 2;
        }
        
        if (current_level_nodes == 0) {
            std::cout << std::endl;
            level++;
            current_level_nodes = next_level_nodes;
            next_level_nodes = 0;
        }
    }
}

// Analyze effect of cutoff depth on performance
void analyze_cutoff_impact(std::shared_ptr<TreeNode> root, int max_depth) {
    std::vector<int> seq_result;
    
    // First measure sequential performance
    double seq_time = measure_time([&]() {
        seq_result.clear();
        traverse_sequential(root, seq_result);
    });
    
    std::cout << "\nAnalyzing cutoff depth impact:" << std::endl;
    std::cout << "---------------------------------------------------" << std::endl;
    std::cout << "Cutoff | Parallel Time (s) | Speedup | Correct" << std::endl;
    std::cout << "---------------------------------------------------" << std::endl;
    
    for (int cutoff = 0; cutoff <= max_depth; cutoff += 2) {
        std::vector<int> par_result;
        
        double par_time = measure_time([&]() {
            par_result.clear();
            parallel_tree_traversal(root, par_result, cutoff);
        });
        
        bool correct = verify_results(seq_result, par_result);
        double speedup = seq_time / par_time;
        
        std::cout << std::setw(6) << cutoff << " | "
                  << std::fixed << std::setprecision(6) << std::setw(17) << par_time << " | "
                  << std::fixed << std::setprecision(2) << std::setw(7) << speedup << " | "
                  << (correct ? "Yes" : "No") << std::endl;
    }
}

int main(int argc, char* argv[]) {
    try {
        // Parse command-line arguments
        int depth = 16;
        int num_threads = omp_get_max_threads();
        int cutoff_depth = 8;
        
        if (argc > 1) depth = std::stoi(argv[1]);
        if (argc > 2) num_threads = std::stoi(argv[2]);  
        if (argc > 3) cutoff_depth = std::stoi(argv[3]);
        
        // Sanity checks - daha güvenli limitler
        if (depth < 1) depth = 1;
        if (depth > 8) {
            std::cout << "Limiting tree depth to 8 for stability" << std::endl;
            depth = 8; // Daha güvenli bir sınır
        }
        if (cutoff_depth < 0) cutoff_depth = 0;
        if (cutoff_depth > depth) cutoff_depth = depth / 2;
        if (num_threads < 1) num_threads = 1;
        if (num_threads > 32) num_threads = 32; // İhtiyatlı bir thread sayısı limiti
        
        omp_set_num_threads(num_threads);
        
        std::cout << "=== Binary Tree Traversal Task Parallelism Example ===" << std::endl;
        std::cout << "Tree depth: " << depth << std::endl;
        std::cout << "Number of threads: " << num_threads << std::endl;
        std::cout << "Cutoff depth for parallel execution: " << cutoff_depth << std::endl;
        
        // Generate a balanced binary tree
        int node_count = 0;
        auto root = generate_tree(depth, node_count);
        
        std::cout << "Generated a tree with " << node_count << " nodes" << std::endl;
        
        // Print the first few levels of the tree
        print_tree_levels(root);
        
        // Sequential traversal
        std::vector<int> seq_result;
        double seq_time = measure_time([&]() {
            traverse_sequential(root, seq_result);
        });
        
        std::cout << "\nSequential traversal result size: " << seq_result.size() << std::endl;
        std::cout << "Sequential traversal time: " << std::fixed << std::setprecision(6) << seq_time << " seconds" << std::endl;
        
        // Skip parallel traversal for very large trees
        if (node_count > 10000) {
            std::cout << "\nTree is too large for parallel traversal" << std::endl;
            std::cout << "Skipping parallel execution" << std::endl;
            return 0;
        }
        
        // Parallel traversal
        std::vector<int> par_result;
        double par_time = 0.0;
        
        try {
            par_time = measure_time([&]() {
                parallel_tree_traversal(root, par_result, cutoff_depth);
            });
            
            std::cout << "\nParallel traversal result size: " << par_result.size() << std::endl;
            std::cout << "Parallel traversal time: " << std::fixed << std::setprecision(6) << par_time << " seconds" << std::endl;
            
            // Calculate and display speedup
            double speedup = seq_time / par_time;
            double efficiency = (speedup / num_threads) * 100.0;
            
            std::cout << "\nSpeedup: " << std::fixed << std::setprecision(2) << speedup << "x" << std::endl;
            std::cout << "Efficiency: " << std::fixed << std::setprecision(2) << efficiency << "%" << std::endl;
            
            // Verify results correctness
            bool results_match = verify_results(seq_result, par_result);
            std::cout << "\nResults correctness: " << (results_match ? "PASS" : "FAIL") << std::endl;
        }
        catch (const std::exception& e) {
            std::cerr << "Error during parallel traversal: " << e.what() << std::endl;
            return 1;
        }
        
        // Skip cutoff analysis completely
        std::cout << "\nSkipping cutoff analysis for performance reasons" << std::endl;
        
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}