#include <iostream>
#include <iomanip>
#include <vector>
#include <queue>
#include <unordered_set>
#include <unordered_map>
#include <random>
#include <chrono>
#include <algorithm>
#include <memory>
#include <mutex>
#include <atomic>
#include <omp.h>
#include "../include/task_utils.h"

//==============================================================================
// Graph Data Structures
//==============================================================================

// Graph represented as adjacency list
class Graph {
private:
    int num_vertices;
    std::vector<std::vector<int>> adjacency_list;
    
public:
    Graph(int n) : num_vertices(n), adjacency_list(n) {}
    
    void add_edge(int u, int v) {
        if (u >= 0 && u < num_vertices && v >= 0 && v < num_vertices) {
            adjacency_list[u].push_back(v);
            adjacency_list[v].push_back(u); // For undirected graph
        }
    }
    
    const std::vector<int>& get_neighbors(int vertex) const {
        return adjacency_list[vertex];
    }
    
    int get_num_vertices() const {
        return num_vertices;
    }
    
    int get_num_edges() const {
        int count = 0;
        for (const auto& neighbors : adjacency_list) {
            count += static_cast<int>(neighbors.size());
        }
        return count / 2; // Each edge is counted twice for undirected graph
    }
    
    double get_average_degree() const {
        double total_degree = 0;
        for (const auto& neighbors : adjacency_list) {
            total_degree += static_cast<double>(neighbors.size());
        }
        return total_degree / num_vertices;
    }
    
    void print_stats() const {
        std::cout << "Graph Statistics:" << std::endl;
        std::cout << "- Vertices: " << num_vertices << std::endl;
        std::cout << "- Edges: " << get_num_edges() << std::endl;
        std::cout << "- Average degree: " << std::fixed << std::setprecision(2) 
                  << get_average_degree() << std::endl;
        
        // Degree distribution
        std::map<int, int> degree_counts;
        for (const auto& neighbors : adjacency_list) {
            degree_counts[static_cast<int>(neighbors.size())]++;
        }
        
        std::cout << "- Degree distribution:" << std::endl;
        for (const auto& [degree, count] : degree_counts) {
            std::cout << "  " << degree << ": " << count << " vertices (" 
                      << std::fixed << std::setprecision(1)
                      << (100.0 * count / num_vertices) << "%)" << std::endl;
        }
    }
};

// Generate a random graph
Graph generate_random_graph(int num_vertices, int num_edges, int seed = 42) {
    std::mt19937 gen(seed);
    std::uniform_int_distribution<> dist(0, num_vertices - 1);
    
    Graph graph(num_vertices);
    int added_edges = 0;
    
    // Add edges
    while (added_edges < num_edges) {
        int u = dist(gen);
        int v = dist(gen);
        
        // Avoid self-loops and duplicate edges
        if (u != v) {
            bool already_exists = false;
            for (int neighbor : graph.get_neighbors(u)) {
                if (neighbor == v) {
                    already_exists = true;
                    break;
                }
            }
            
            if (!already_exists) {
                graph.add_edge(u, v);
                added_edges++;
            }
        }
    }
    
    return graph;
}

// Generate a scale-free graph using preferential attachment
Graph generate_scale_free_graph(int num_vertices, int min_edges_per_vertex, int seed = 42) {
    std::mt19937 gen(seed);
    
    Graph graph(num_vertices);
    
    // Start with a small complete graph (e.g., 5 vertices)
    int initial_vertices = std::min(5, num_vertices);
    for (int i = 0; i < initial_vertices; i++) {
        for (int j = i + 1; j < initial_vertices; j++) {
            graph.add_edge(i, j);
        }
    }
    
    // Add remaining vertices with preferential attachment
    for (int new_vertex = initial_vertices; new_vertex < num_vertices; new_vertex++) {
        // Each new vertex connects to 'min_edges_per_vertex' existing vertices
        for (int e = 0; e < min_edges_per_vertex; e++) {
            // Select an existing vertex with probability proportional to its degree
            std::vector<int> degree_sum(new_vertex);
            int total_degree = 0;
            
            for (int i = 0; i < new_vertex; i++) {
                int degree = static_cast<int>(graph.get_neighbors(i).size());
                total_degree += degree > 0 ? degree : 1; // Ensure even isolated vertices have some probability
                degree_sum[i] = total_degree;
            }
            
            std::uniform_int_distribution<> dist(1, total_degree);
            int random_value = dist(gen);
            
            // Find the vertex corresponding to the random value
            int target_vertex = 0;
            for (int i = 0; i < new_vertex; i++) {
                if (random_value <= degree_sum[i]) {
                    target_vertex = i;
                    break;
                }
            }
            
            // Add edge between new_vertex and target_vertex
            bool already_exists = false;
            for (int neighbor : graph.get_neighbors(new_vertex)) {
                if (neighbor == target_vertex) {
                    already_exists = true;
                    break;
                }
            }
            
            if (!already_exists && new_vertex != target_vertex) {
                graph.add_edge(new_vertex, target_vertex);
            }
        }
    }
    
    return graph;
}

// Generate a regular grid graph
Graph generate_grid_graph(int width, int height) {
    int num_vertices = width * height;
    Graph graph(num_vertices);
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int vertex = y * width + x;
            
            // Connect to right neighbor
            if (x < width - 1) {
                graph.add_edge(vertex, vertex + 1);
            }
            
            // Connect to bottom neighbor
            if (y < height - 1) {
                graph.add_edge(vertex, vertex + width);
            }
        }
    }
    
    return graph;
}

//==============================================================================
// Graph Algorithms - Sequential Implementations
//==============================================================================

// Sequential Breadth-First Search (BFS)
std::vector<int> bfs_sequential(const Graph& graph, int start_vertex) {
    int num_vertices = graph.get_num_vertices();
    std::vector<int> distances(num_vertices, -1); // -1 indicates unvisited
    std::queue<int> q;
    
    distances[start_vertex] = 0;
    q.push(start_vertex);
    
    while (!q.empty()) {
        int current = q.front();
        q.pop();
        
        for (int neighbor : graph.get_neighbors(current)) {
            if (distances[neighbor] == -1) {
                distances[neighbor] = distances[current] + 1;
                q.push(neighbor);
            }
        }
    }
    
    return distances;
}

// Sequential Depth-First Search (DFS)
void dfs_sequential_recursive(const Graph& graph, int vertex, std::vector<bool>& visited) {
    visited[vertex] = true;
    
    // Simulate work
    task_utils::do_compute_work(100); // Small amount of work per vertex
    
    for (int neighbor : graph.get_neighbors(vertex)) {
        if (!visited[neighbor]) {
            dfs_sequential_recursive(graph, neighbor, visited);
        }
    }
}

std::vector<bool> dfs_sequential(const Graph& graph, int start_vertex) {
    int num_vertices = graph.get_num_vertices();
    std::vector<bool> visited(num_vertices, false);
    
    dfs_sequential_recursive(graph, start_vertex, visited);
    
    return visited;
}

// Sequential Connected Components
std::vector<int> connected_components_sequential(const Graph& graph) {
    int num_vertices = graph.get_num_vertices();
    std::vector<int> component_ids(num_vertices, -1);
    int current_component = 0;
    
    for (int v = 0; v < num_vertices; v++) {
        if (component_ids[v] == -1) {
            // Start a new component with BFS
            std::queue<int> q;
            component_ids[v] = current_component;
            q.push(v);
            
            while (!q.empty()) {
                int current = q.front();
                q.pop();
                
                for (int neighbor : graph.get_neighbors(current)) {
                    if (component_ids[neighbor] == -1) {
                        component_ids[neighbor] = current_component;
                        q.push(neighbor);
                    }
                }
            }
            
            current_component++;
        }
    }
    
    return component_ids;
}

// Sequential PageRank
std::vector<double> pagerank_sequential(const Graph& graph, int iterations, double damping_factor = 0.85) {
    int num_vertices = graph.get_num_vertices();
    std::vector<double> ranks(num_vertices, 1.0 / num_vertices);
    std::vector<double> new_ranks(num_vertices, 0.0);
    
    for (int iter = 0; iter < iterations; iter++) {
        // Reset new ranks
        std::fill(new_ranks.begin(), new_ranks.end(), (1.0 - damping_factor) / num_vertices);
        
        // Distribute rank along outgoing edges
        for (int v = 0; v < num_vertices; v++) {
            const auto& neighbors = graph.get_neighbors(v);
            if (!neighbors.empty()) {
                double contribution = damping_factor * ranks[v] / neighbors.size();
                for (int neighbor : neighbors) {
                    new_ranks[neighbor] += contribution;
                }
            }
        }
        
        // Update ranks
        ranks.swap(new_ranks);
    }
    
    return ranks;
}

//==============================================================================
// Graph Algorithms - Task-Based Parallel Implementations
//==============================================================================

// Task-based Breadth-First Search (level by level)
std::vector<int> bfs_parallel_task(const Graph& graph, int start_vertex) {
    int num_vertices = graph.get_num_vertices();
    std::vector<int> distances(num_vertices, -1);
    
    std::vector<std::vector<int>> levels(1); // Vertices by level
    levels[0].push_back(start_vertex);
    distances[start_vertex] = 0;
    
    int current_level = 0;
    
    while (!levels[current_level].empty()) {
        std::vector<int> next_level;
        std::mutex next_level_mutex;
        
        #pragma omp parallel
        {
            #pragma omp single
            {
                for (int vertex : levels[current_level]) {
                    #pragma omp task firstprivate(vertex)
                    {
                        std::vector<int> local_next_level;
                        
                        for (int neighbor : graph.get_neighbors(vertex)) {
                            if (distances[neighbor] == -1) {
                                #pragma omp critical
                                {
                                    if (distances[neighbor] == -1) { // Check again to avoid race conditions
                                        distances[neighbor] = current_level + 1;
                                        local_next_level.push_back(neighbor);
                                    }
                                }
                            }
                        }
                        
                        if (!local_next_level.empty()) {
                            std::lock_guard<std::mutex> lock(next_level_mutex);
                            next_level.insert(next_level.end(), local_next_level.begin(), local_next_level.end());
                        }
                    }
                }
                
                #pragma omp taskwait
            }
        }
        
        if (next_level.empty()) {
            break;
        }
        
        current_level++;
        levels.push_back(std::move(next_level));
    }
    
    return distances;
}

// Task-based Depth-First Search
void dfs_parallel_task_recursive(const Graph& graph, int vertex, std::vector<bool>& visited, 
                              int& processed_count, std::mutex& visited_mutex, int cutoff_depth = 3) {
    // Mark vertex as visited
    {
        std::lock_guard<std::mutex> lock(visited_mutex);
        if (visited[vertex]) {
            return; // Already visited by another task
        }
        visited[vertex] = true;
    }
    
    // Simulate work
    task_utils::do_compute_work(100); // Small amount of work per vertex
    
    // Increment processed count
    #pragma omp atomic
    processed_count++;
    
    // Create tasks for neighbors
    std::vector<int> unvisited_neighbors;
    
    for (int neighbor : graph.get_neighbors(vertex)) {
        bool already_visited;
        {
            std::lock_guard<std::mutex> lock(visited_mutex);
            already_visited = visited[neighbor];
        }
        
        if (!already_visited) {
            unvisited_neighbors.push_back(neighbor);
        }
    }
    
    if (cutoff_depth > 0) {
        // Still within task creation depth, create tasks
        for (int neighbor : unvisited_neighbors) {
            #pragma omp task
            {
                dfs_parallel_task_recursive(graph, neighbor, visited, processed_count, visited_mutex, cutoff_depth - 1);
            }
        }
    } else {
        // Beyond cutoff depth, process sequentially
        for (int neighbor : unvisited_neighbors) {
            dfs_parallel_task_recursive(graph, neighbor, visited, processed_count, visited_mutex, 0);
        }
    }
}

std::vector<bool> dfs_parallel_task(const Graph& graph, int start_vertex, int cutoff_depth = 3) {
    int num_vertices = graph.get_num_vertices();
    std::vector<bool> visited(num_vertices, false);
    int processed_count = 0;
    std::mutex visited_mutex;
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            dfs_parallel_task_recursive(graph, start_vertex, visited, processed_count, visited_mutex, cutoff_depth);
        }
    }
    
    return visited;
}

// Task-based Connected Components
std::vector<int> connected_components_parallel_task(const Graph& graph, int batch_size = 64) {
    int num_vertices = graph.get_num_vertices();
    std::vector<int> component_ids(num_vertices, -1);
    std::atomic<int> current_component(0);
    
    // Process vertices in batches
    #pragma omp parallel
    {
        #pragma omp single
        {
            for (int start_v = 0; start_v < num_vertices; start_v += batch_size) {
                #pragma omp task firstprivate(start_v)
                {
                    int end_v = std::min(start_v + batch_size, num_vertices);
                    
                    for (int v = start_v; v < end_v; v++) {
                        if (component_ids[v] == -1) {
                            // Start a new component
                            int comp_id = current_component++;
                            
                            // Assign the component ID
                            component_ids[v] = comp_id;
                            
                            // Process this component with BFS
                            std::queue<int> q;
                            q.push(v);
                            
                            while (!q.empty()) {
                                int current = q.front();
                                q.pop();
                                
                                for (int neighbor : graph.get_neighbors(current)) {
                                    if (component_ids[neighbor] == -1) {
                                        #pragma omp critical
                                        {
                                            if (component_ids[neighbor] == -1) {
                                                component_ids[neighbor] = comp_id;
                                                q.push(neighbor);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    return component_ids;
}

// Task-based PageRank
std::vector<double> pagerank_parallel_task(const Graph& graph, int iterations, double damping_factor = 0.85, int batch_size = 64) {
    int num_vertices = graph.get_num_vertices();
    std::vector<double> ranks(num_vertices, 1.0 / num_vertices);
    std::vector<double> new_ranks(num_vertices, 0.0);
    
    for (int iter = 0; iter < iterations; iter++) {
        // Reset new ranks
        std::fill(new_ranks.begin(), new_ranks.end(), (1.0 - damping_factor) / num_vertices);
        
        #pragma omp parallel
        {
            #pragma omp single
            {
                // Process vertices in batches
                for (int start_v = 0; start_v < num_vertices; start_v += batch_size) {
                    #pragma omp task firstprivate(start_v)
                    {
                        int end_v = std::min(start_v + batch_size, num_vertices);
                        
                        // For each vertex in this batch
                        for (int v = start_v; v < end_v; v++) {
                            const auto& neighbors = graph.get_neighbors(v);
                            if (!neighbors.empty()) {
                                double contribution = damping_factor * ranks[v] / neighbors.size();
                                
                                for (int neighbor : neighbors) {
                                    #pragma omp atomic
                                    new_ranks[neighbor] += contribution;
                                }
                            }
                        }
                    }
                }
                
                #pragma omp taskwait
            }
        }
        
        // Update ranks
        ranks.swap(new_ranks);
    }
    
    return ranks;
}

//==============================================================================
// Graph Algorithms - Parallel For Implementations (for comparison)
//==============================================================================

// Parallel (non-task) BFS
std::vector<int> bfs_parallel_for(const Graph& graph, int start_vertex) {
    int num_vertices = graph.get_num_vertices();
    std::vector<int> distances(num_vertices, -1);
    
    std::vector<std::vector<int>> levels(1); // Vertices by level
    levels[0].push_back(start_vertex);
    distances[start_vertex] = 0;
    
    int current_level = 0;
    
    while (!levels[current_level].empty()) {
        std::vector<int> next_level;
        std::mutex next_level_mutex;
        
        #pragma omp parallel for
        for (int i = 0; i < static_cast<int>(levels[current_level].size()); i++) {
            int vertex = levels[current_level][i];
            std::vector<int> local_next_level;
            
            for (int neighbor : graph.get_neighbors(vertex)) {
                if (distances[neighbor] == -1) {
                    #pragma omp critical
                    {
                        if (distances[neighbor] == -1) { // Check again to avoid race conditions
                            distances[neighbor] = current_level + 1;
                            local_next_level.push_back(neighbor);
                        }
                    }
                }
            }
            
            if (!local_next_level.empty()) {
                std::lock_guard<std::mutex> lock(next_level_mutex);
                next_level.insert(next_level.end(), local_next_level.begin(), local_next_level.end());
            }
        }
        
        if (next_level.empty()) {
            break;
        }
        
        current_level++;
        levels.push_back(std::move(next_level));
    }
    
    return distances;
}

// Parallel Connected Components (using parallel for)
std::vector<int> connected_components_parallel_for(const Graph& graph) {
    int num_vertices = graph.get_num_vertices();
    std::vector<int> component_ids(num_vertices, -1);
    std::atomic<int> current_component(0);
    
    #pragma omp parallel for schedule(dynamic, 64)
    for (int v = 0; v < num_vertices; v++) {
        if (component_ids[v] == -1) {
            // Start a new component
            int comp_id = current_component++;
            
            // Assign the component ID
            component_ids[v] = comp_id;
            
            // Process this component with BFS
            std::queue<int> q;
            q.push(v);
            
            while (!q.empty()) {
                int current = q.front();
                q.pop();
                
                for (int neighbor : graph.get_neighbors(current)) {
                    if (component_ids[neighbor] == -1) {
                        #pragma omp critical
                        {
                            if (component_ids[neighbor] == -1) {
                                component_ids[neighbor] = comp_id;
                                q.push(neighbor);
                            }
                        }
                    }
                }
            }
        }
    }
    
    return component_ids;
}

// Parallel PageRank (using parallel for)
std::vector<double> pagerank_parallel_for(const Graph& graph, int iterations, double damping_factor = 0.85) {
    int num_vertices = graph.get_num_vertices();
    std::vector<double> ranks(num_vertices, 1.0 / num_vertices);
    std::vector<double> new_ranks(num_vertices, 0.0);
    
    for (int iter = 0; iter < iterations; iter++) {
        // Reset new ranks
        std::fill(new_ranks.begin(), new_ranks.end(), (1.0 - damping_factor) / num_vertices);
        
        #pragma omp parallel for schedule(dynamic, 64)
        for (int v = 0; v < num_vertices; v++) {
            const auto& neighbors = graph.get_neighbors(v);
            if (!neighbors.empty()) {
                double contribution = damping_factor * ranks[v] / neighbors.size();
                
                for (int neighbor : neighbors) {
                    #pragma omp atomic
                    new_ranks[neighbor] += contribution;
                }
            }
        }
        
        // Update ranks
        ranks.swap(new_ranks);
    }
    
    return ranks;
}

//==============================================================================
// Benchmarking Functions
//==============================================================================

// Function to count number of vertices reached
int count_reached_vertices(const std::vector<int>& distances) {
    int count = 0;
    for (int d : distances) {
        if (d != -1) {
            count++;
        }
    }
    return count;
}

// Function to count number of components
int count_components(const std::vector<int>& component_ids) {
    std::unordered_set<int> unique_components;
    for (int id : component_ids) {
        if (id != -1) {
            unique_components.insert(id);
        }
    }
    return static_cast<int>(unique_components.size());
}

// Check if two distance arrays are equivalent
bool are_distances_equivalent(const std::vector<int>& dist1, const std::vector<int>& dist2) {
    if (dist1.size() != dist2.size()) {
        return false;
    }
    
    for (size_t i = 0; i < dist1.size(); i++) {
        if ((dist1[i] == -1 && dist2[i] != -1) || (dist1[i] != -1 && dist2[i] == -1)) {
            return false;
        }
    }
    
    return true;
}

// Check if two component ID arrays represent the same partitioning
bool are_components_equivalent(const std::vector<int>& comp1, const std::vector<int>& comp2) {
    if (comp1.size() != comp2.size()) {
        return false;
    }
    
    std::unordered_map<int, int> component_mapping;
    
    for (size_t i = 0; i < comp1.size(); i++) {
        if (comp1[i] == -1 && comp2[i] == -1) {
            continue;
        }
        
        if (comp1[i] == -1 || comp2[i] == -1) {
            return false;
        }
        
        if (component_mapping.find(comp1[i]) == component_mapping.end()) {
            component_mapping[comp1[i]] = comp2[i];
        } else if (component_mapping[comp1[i]] != comp2[i]) {
            return false;
        }
    }
    
    return true;
}

// Check if two PageRank arrays are approximately equal
bool are_pageranks_equivalent(const std::vector<double>& pr1, const std::vector<double>& pr2, double tolerance = 1e-5) {
    if (pr1.size() != pr2.size()) {
        return false;
    }
    
    for (size_t i = 0; i < pr1.size(); i++) {
        if (std::abs(pr1[i] - pr2[i]) > tolerance) {
            return false;
        }
    }
    
    return true;
}

// Benchmark BFS algorithm
void benchmark_bfs(const Graph& graph, int start_vertex, int num_threads) {
    std::cout << "\nBenchmarking Breadth-First Search from vertex " << start_vertex << ":" << std::endl;
    std::cout << "--------------------------------------------------" << std::endl;
    
    // Sequential BFS
    auto start_time = std::chrono::high_resolution_clock::now();
    auto seq_distances = bfs_sequential(graph, start_vertex);
    auto end_time = std::chrono::high_resolution_clock::now();
    double seq_time = std::chrono::duration<double>(end_time - start_time).count();
    
    int reached_vertices = count_reached_vertices(seq_distances);
    std::cout << "Sequential BFS: " << std::fixed << std::setprecision(4) << seq_time << " seconds" << std::endl;
    std::cout << "Reached " << reached_vertices << " vertices out of " << graph.get_num_vertices() << std::endl;
    
    // Parallel For BFS
    omp_set_num_threads(num_threads);
    start_time = std::chrono::high_resolution_clock::now();
    auto par_for_distances = bfs_parallel_for(graph, start_vertex);
    end_time = std::chrono::high_resolution_clock::now();
    double par_for_time = std::chrono::duration<double>(end_time - start_time).count();
    
    bool par_for_correct = are_distances_equivalent(seq_distances, par_for_distances);
    double par_for_speedup = seq_time / par_for_time;
    
    std::cout << "Parallel For BFS: " << std::fixed << std::setprecision(4) << par_for_time << " seconds" 
              << " (speedup: " << std::fixed << std::setprecision(2) << par_for_speedup << "x)" 
              << (par_for_correct ? "" : " - INCORRECT") << std::endl;
    
    // Task-based Parallel BFS
    start_time = std::chrono::high_resolution_clock::now();
    auto task_distances = bfs_parallel_task(graph, start_vertex);
    end_time = std::chrono::high_resolution_clock::now();
    double task_time = std::chrono::duration<double>(end_time - start_time).count();
    
    bool task_correct = are_distances_equivalent(seq_distances, task_distances);
    double task_speedup = seq_time / task_time;
    
    std::cout << "Task-based BFS: " << std::fixed << std::setprecision(4) << task_time << " seconds" 
              << " (speedup: " << std::fixed << std::setprecision(2) << task_speedup << "x)" 
              << (task_correct ? "" : " - INCORRECT") << std::endl;
}

// Benchmark DFS algorithm
void benchmark_dfs(const Graph& graph, int start_vertex, int num_threads) {
    std::cout << "\nBenchmarking Depth-First Search from vertex " << start_vertex << ":" << std::endl;
    std::cout << "--------------------------------------------------" << std::endl;
    
    // Sequential DFS
    auto start_time = std::chrono::high_resolution_clock::now();
    auto seq_visited = dfs_sequential(graph, start_vertex);
    auto end_time = std::chrono::high_resolution_clock::now();
    double seq_time = std::chrono::duration<double>(end_time - start_time).count();
    
    int visited_count = static_cast<int>(std::count(seq_visited.begin(), seq_visited.end(), true));
    std::cout << "Sequential DFS: " << std::fixed << std::setprecision(4) << seq_time << " seconds" << std::endl;
    std::cout << "Visited " << visited_count << " vertices out of " << graph.get_num_vertices() << std::endl;
    
    // Task-based Parallel DFS with different cutoff depths
    omp_set_num_threads(num_threads);
    std::vector<int> cutoff_depths = {1, 2, 3, 5, 10};
    
    for (int cutoff : cutoff_depths) {
        start_time = std::chrono::high_resolution_clock::now();
        auto task_visited = dfs_parallel_task(graph, start_vertex, cutoff);
        end_time = std::chrono::high_resolution_clock::now();
        double task_time = std::chrono::duration<double>(end_time - start_time).count();
        
        int task_visited_count = static_cast<int>(std::count(task_visited.begin(), task_visited.end(), true));
        bool task_correct = (task_visited_count == visited_count);
        double task_speedup = seq_time / task_time;
        
        std::cout << "Task-based DFS (cutoff=" << cutoff << "): " 
                  << std::fixed << std::setprecision(4) << task_time << " seconds" 
                  << " (speedup: " << std::fixed << std::setprecision(2) << task_speedup << "x)" 
                  << (task_correct ? "" : " - INCORRECT") << std::endl;
    }
}

// Benchmark Connected Components algorithm
void benchmark_connected_components(const Graph& graph, int num_threads) {
    std::cout << "\nBenchmarking Connected Components:" << std::endl;
    std::cout << "--------------------------------------------------" << std::endl;
    
    // Sequential Connected Components
    auto start_time = std::chrono::high_resolution_clock::now();
    auto seq_components = connected_components_sequential(graph);
    auto end_time = std::chrono::high_resolution_clock::now();
    double seq_time = std::chrono::duration<double>(end_time - start_time).count();
    
    int num_components = count_components(seq_components);
    std::cout << "Sequential Connected Components: " << std::fixed << std::setprecision(4) 
              << seq_time << " seconds" << std::endl;
    std::cout << "Found " << num_components << " components" << std::endl;
    
    // Parallel For Connected Components
    omp_set_num_threads(num_threads);
    start_time = std::chrono::high_resolution_clock::now();
    auto par_for_components = connected_components_parallel_for(graph);
    end_time = std::chrono::high_resolution_clock::now();
    double par_for_time = std::chrono::duration<double>(end_time - start_time).count();
    
    bool par_for_correct = (count_components(par_for_components) == num_components);
    double par_for_speedup = seq_time / par_for_time;
    
    std::cout << "Parallel For Connected Components: " << std::fixed << std::setprecision(4) 
              << par_for_time << " seconds" 
              << " (speedup: " << std::fixed << std::setprecision(2) << par_for_speedup << "x)" 
              << (par_for_correct ? "" : " - INCORRECT") << std::endl;
    
    // Task-based Connected Components with different batch sizes
    std::vector<int> batch_sizes = {16, 32, 64, 128, 256};
    
    for (int batch_size : batch_sizes) {
        start_time = std::chrono::high_resolution_clock::now();
        auto task_components = connected_components_parallel_task(graph, batch_size);
        end_time = std::chrono::high_resolution_clock::now();
        double task_time = std::chrono::duration<double>(end_time - start_time).count();
        
        bool task_correct = (count_components(task_components) == num_components);
        double task_speedup = seq_time / task_time;
        
        std::cout << "Task-based Connected Components (batch=" << batch_size << "): " 
                  << std::fixed << std::setprecision(4) << task_time << " seconds" 
                  << " (speedup: " << std::fixed << std::setprecision(2) << task_speedup << "x)" 
                  << (task_correct ? "" : " - INCORRECT") << std::endl;
    }
}

// Benchmark PageRank algorithm
void benchmark_pagerank(const Graph& graph, int num_threads, int iterations = 20) {
    std::cout << "\nBenchmarking PageRank (" << iterations << " iterations):" << std::endl;
    std::cout << "--------------------------------------------------" << std::endl;
    
    // Sequential PageRank
    auto start_time = std::chrono::high_resolution_clock::now();
    auto seq_pagerank = pagerank_sequential(graph, iterations);
    auto end_time = std::chrono::high_resolution_clock::now();
    double seq_time = std::chrono::duration<double>(end_time - start_time).count();
    
    std::cout << "Sequential PageRank: " << std::fixed << std::setprecision(4) 
              << seq_time << " seconds" << std::endl;
    
    // Parallel For PageRank
    omp_set_num_threads(num_threads);
    start_time = std::chrono::high_resolution_clock::now();
    auto par_for_pagerank = pagerank_parallel_for(graph, iterations);
    end_time = std::chrono::high_resolution_clock::now();
    double par_for_time = std::chrono::duration<double>(end_time - start_time).count();
    
    bool par_for_correct = are_pageranks_equivalent(seq_pagerank, par_for_pagerank);
    double par_for_speedup = seq_time / par_for_time;
    
    std::cout << "Parallel For PageRank: " << std::fixed << std::setprecision(4) 
              << par_for_time << " seconds" 
              << " (speedup: " << std::fixed << std::setprecision(2) << par_for_speedup << "x)" 
              << (par_for_correct ? "" : " - INCORRECT") << std::endl;
    
    // Task-based PageRank with different batch sizes
    std::vector<int> batch_sizes = {16, 32, 64, 128, 256};
    
    for (int batch_size : batch_sizes) {
        start_time = std::chrono::high_resolution_clock::now();
        auto task_pagerank = pagerank_parallel_task(graph, iterations, 0.85, batch_size);
        end_time = std::chrono::high_resolution_clock::now();
        double task_time = std::chrono::duration<double>(end_time - start_time).count();
        
        bool task_correct = are_pageranks_equivalent(seq_pagerank, task_pagerank);
        double task_speedup = seq_time / task_time;
        
        std::cout << "Task-based PageRank (batch=" << batch_size << "): " 
                  << std::fixed << std::setprecision(4) << task_time << " seconds" 
                  << " (speedup: " << std::fixed << std::setprecision(2) << task_speedup << "x)" 
                  << (task_correct ? "" : " - INCORRECT") << std::endl;
    }
}

// Run all benchmarks
void run_benchmarks(const Graph& graph, int num_threads) {
    // Choose a start vertex for traversal algorithms (use vertex 0)
    int start_vertex = 0;
    
    // Run benchmarks
    benchmark_bfs(graph, start_vertex, num_threads);
    benchmark_dfs(graph, start_vertex, num_threads);
    benchmark_connected_components(graph, num_threads);
    benchmark_pagerank(graph, num_threads);
    
    // Display overall performance summary
    std::cout << "\nOverall Performance Summary:" << std::endl;
    std::cout << "--------------------------------------------------" << std::endl;
    std::cout << "Task-based parallelism generally performs better for graph algorithms due to:" << std::endl;
    std::cout << "1. Irregular workloads and dependencies" << std::endl;
    std::cout << "2. Better load balancing through work stealing" << std::endl;
    std::cout << "3. More efficient handling of nested parallelism" << std::endl;
    std::cout << "4. Lower synchronization overhead" << std::endl;
    
    std::cout << "\nRecommendations for graph processing with tasks:" << std::endl;
    std::cout << "- Use batch processing for vertices when possible" << std::endl;
    std::cout << "- Limit task creation depth for recursive algorithms" << std::endl;
    std::cout << "- Use dynamic task granularity based on graph structure" << std::endl;
    std::cout << "- Consider using task dependencies for algorithms with structured dependencies" << std::endl;
}

//==============================================================================
// Main Function
//==============================================================================

int main(int argc, char* argv[]) {
    // Parse command-line arguments
    int num_vertices = 1000;
    int num_edges = 5000;
    int num_threads = omp_get_max_threads();
    int graph_type = 0;  // 0=random, 1=scale-free, 2=grid
    int seed = 42;
    
    if (argc > 1) num_vertices = std::stoi(argv[1]);
    if (argc > 2) num_edges = std::stoi(argv[2]);
    if (argc > 3) num_threads = std::stoi(argv[3]);
    if (argc > 4) graph_type = std::stoi(argv[4]);
    
    std::cout << "=== OpenMP Graph Processing with Task Parallelism ===" << std::endl;
    std::cout << "Number of vertices: " << num_vertices << std::endl;
    std::cout << "Number of threads: " << num_threads << std::endl;
    
    // Generate the appropriate graph
    Graph graph(num_vertices);
    
    switch (graph_type) {
        case 0:
            std::cout << "Graph type: Random" << std::endl;
            std::cout << "Number of edges: " << num_edges << std::endl;
            graph = generate_random_graph(num_vertices, num_edges, seed);
            break;
        case 1: {
            std::cout << "Graph type: Scale-free" << std::endl;
            int min_edges_per_vertex = 2;
            std::cout << "Min edges per vertex: " << min_edges_per_vertex << std::endl;
            graph = generate_scale_free_graph(num_vertices, min_edges_per_vertex, seed);
            std::cout << "Generated approximately " << graph.get_num_edges() << " edges" << std::endl;
            break;
        }
        case 2: {
            std::cout << "Graph type: Grid" << std::endl;
            int width = static_cast<int>(std::sqrt(num_vertices));
            int height = num_vertices / width;
            std::cout << "Grid dimensions: " << width << " x " << height << " (" 
                      << width * height << " vertices)" << std::endl;
            graph = generate_grid_graph(width, height);
            std::cout << "Generated " << graph.get_num_edges() << " edges" << std::endl;
            break;
        }
        default:
            std::cerr << "Unknown graph type: " << graph_type << std::endl;
            return 1;
    }
    
    // Print graph statistics
    graph.print_stats();
    
    // Run only BFS benchmark
    int start_vertex = 0;
    benchmark_bfs(graph, start_vertex, num_threads);
    
    return 0;
}