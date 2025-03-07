# OpenMP Task Parallelism Guide

This guide provides a comprehensive overview of task-based parallelism in OpenMP, including best practices, common patterns, and optimization strategies.

## Table of Contents

1. [Introduction to OpenMP Tasks](#introduction-to-openmp-tasks)
2. [Task Creation and Execution](#task-creation-and-execution)
3. [Task Data Environment](#task-data-environment)
4. [Task Synchronization](#task-synchronization)
5. [Task Dependencies](#task-dependencies)
6. [Taskloop Construct](#taskloop-construct)
7. [Task Priorities](#task-priorities)
8. [Nested Tasks](#nested-tasks)
9. [Best Practices](#best-practices)
10. [Common Pitfalls](#common-pitfalls)

## Introduction to OpenMP Tasks

OpenMP tasks provide a way to express parallelism in irregular patterns that are difficult to parallelize with work-sharing constructs like `parallel for`. Tasks are particularly useful for:

- Recursive algorithms (e.g., tree traversal, divide-and-conquer)
- Producer-consumer patterns
- While loops and unbounded loops
- Complex dependency graphs

Tasks allow the programmer to specify units of work that can be executed in parallel, leaving the scheduling decisions to the OpenMP runtime.

## Task Creation and Execution

### Basic Task Syntax

```cpp
#pragma omp parallel
{
    #pragma omp single
    {
        // Only one thread creates tasks
        for (int i = 0; i < n; i++) {
            #pragma omp task
            {
                // This code will be executed as a task
                process(i);
            }
        }
    }
    // All threads participate in executing tasks
}
```

In this pattern:
- The `parallel` directive creates a team of threads
- The `single` directive ensures that only one thread creates tasks
- The `task` directive creates tasks that can be executed by any thread in the team

### Task Execution Model

When a thread encounters a task directive, it creates a task instance but does not immediately execute it. Instead:

1. The task is placed in a task pool
2. The OpenMP runtime decides when and by which thread the task will be executed
3. Tasks may be executed immediately, delayed, or stolen by other threads

This dynamic scheduling allows for better load balancing, especially for irregular workloads.

## Task Data Environment

Understanding how data is shared between tasks is crucial for correct parallel execution.

### Data-Sharing Attributes

Tasks inherit data-sharing attributes from their enclosing context, but with some important rules:

- `firstprivate`: By default, variables referenced in a task are firstprivate (each task gets its own copy with the original value)
- `shared`: Variables can be explicitly shared between tasks
- `private`: Tasks can have private variables not visible to other tasks

Example:

```cpp
int x = 10;
#pragma omp parallel shared(x)
{
    #pragma omp single
    {
        #pragma omp task shared(x)
        {
            x = 20;  // Modifies the shared variable
        }
        
        #pragma omp task firstprivate(x)
        {
            x = 30;  // Modifies a local copy, original x is unchanged
        }
    }
}
```

### Data Capture

Tasks capture variables at the time of task creation, not at the time of execution:

```cpp
#pragma omp parallel
{
    #pragma omp single
    {
        for (int i = 0; i < 5; i++) {
            #pragma omp task firstprivate(i)
            {
                // Each task captures its own value of i
                printf("Task %d\n", i);
            }
        }
    }
}
```

## Task Synchronization

### Taskwait

The `taskwait` directive creates a synchronization point where the current task waits for all its child tasks to complete:

```cpp
#pragma omp task
{
    #pragma omp task
    {
        // Child task 1
    }
    
    #pragma omp task
    {
        // Child task 2
    }
    
    #pragma omp taskwait  // Wait for both child tasks to complete
    
    // Continue after child tasks are done
}
```

Important note: `taskwait` only waits for immediate child tasks, not for descendants.

### Taskgroup

The `taskgroup` construct creates a group of tasks and waits for all tasks in the group (including descendants) to complete:

```cpp
#pragma omp taskgroup
{
    #pragma omp task
    {
        // Task 1
        #pragma omp task
        {
            // Nested task
        }
    }
    
    #pragma omp task
    {
        // Task 2
    }
    
    // Implicit wait for all tasks and their descendants
}
```

## Task Dependencies

OpenMP 4.0 introduced task dependencies, allowing the programmer to specify the order in which tasks should execute.

### Dependency Types

- `depend(in:var)`: The task depends on the value of `var`
- `depend(out:var)`: The task modifies the value of `var`
- `depend(inout:var)`: The task both reads and modifies `var`

Example:

```cpp
int x = 0;

#pragma omp parallel
{
    #pragma omp single
    {
        #pragma omp task depend(out:x)
        {
            x = compute();  // Produces x
        }
        
        #pragma omp task depend(in:x)
        {
            use(x);  // Consumes x, will run after the first task
        }
        
        #pragma omp task depend(inout:x)
        {
            x = transform(x);  // Both consumes and produces x
                               // Will run after the second task
        }
    }
}
```

### Array Section Dependencies

Dependencies can also be specified on array sections:

```cpp
double A[100];

#pragma omp parallel
{
    #pragma omp single
    {
        for (int i = 1; i < 100; i++) {
            #pragma omp task depend(in:A[i-1]) depend(out:A[i])
            {
                A[i] = A[i-1] + 1;
            }
        }
    }
}
```

## Taskloop Construct

The taskloop construct combines loop parallelism with the flexibility of tasks:

```cpp
#pragma omp taskloop [clauses]
for (int i = 0; i < n; i++) {
    // Each iteration becomes a task
}
```

### Grainsize vs. Num_tasks

Taskloop supports two ways to control chunking:

- `grainsize(n)`: Each task will process approximately n iterations
- `num_tasks(n)`: The loop will be divided into approximately n tasks

Example:

```cpp
#pragma omp taskloop grainsize(100)
for (int i = 0; i < 1000; i++) {
    process(i);  // Will create around 10 tasks, each processing 100 iterations
}
```

## Task Priorities

OpenMP 4.5 introduced task priorities to give hints to the runtime about which tasks should be executed first:

```cpp
#pragma omp task priority(n)
{
    // Higher priority task (n is a non-negative integer)
}
```

The exact interpretation of priorities is implementation-defined, but generally:
- Higher priority tasks are executed earlier
- Priorities are hints, not strict requirements
- Priorities only affect scheduling, not dependencies

Example:

```cpp
#pragma omp parallel
{
    #pragma omp single
    {
        #pragma omp task priority(0)
        {
            // Low priority task
        }
        
        #pragma omp task priority(10)
        {
            // Higher priority task, likely to execute first
        }
    }
}
```

## Nested Tasks

Tasks can create other tasks, leading to nested task parallelism:

```cpp
void process_tree(Node* node) {
    if (node == nullptr) return;
    
    // Process current node
    process(node);
    
    // Create tasks for children
    #pragma omp task
    process_tree(node->left);
    
    #pragma omp task
    process_tree(node->right);
    
    #pragma omp taskwait
}

// Usage
#pragma omp parallel
{
    #pragma omp single
    process_tree(root);
}
```

### Task Cutoff Strategies

For recursive tasks, it's important to implement a cutoff to avoid creating too many small tasks:

```cpp
void quicksort(int* arr, int low, int high) {
    if (low < high) {
        int pivot = partition(arr, low, high);
        
        if (high - low > CUTOFF) {
            // Use tasks for large arrays
            #pragma omp task
            quicksort(arr, low, pivot - 1);
            
            #pragma omp task
            quicksort(arr, pivot + 1, high);
            
            #pragma omp taskwait
        } else {
            // Use sequential code for small arrays
            quicksort(arr, low, pivot - 1);
            quicksort(arr, pivot + 1, high);
        }
    }
}
```

## Best Practices

### 1. Task Granularity

Choose an appropriate task size to balance parallelism and overhead:
- Too small: Overhead of task creation exceeds benefits
- Too large: Insufficient parallelism for good load balancing

Rule of thumb: A task should perform work equivalent to at least several thousand CPU cycles.

### 2. Task Creation

- Use `single` construct to create tasks from a single thread
- Create tasks recursively for divide-and-conquer algorithms
- Consider using taskloop for regular loops

### 3. Data Locality

- Group related data to improve cache utilization
- Consider memory layout and access patterns
- Use firstprivate for better locality

### 4. Synchronization

- Minimize synchronization points
- Use taskwait only when necessary
- Consider taskgroup for waiting on nested tasks

### 5. Task Dependencies

- Use task dependencies to express the computation flow
- Avoid over-constraining with unnecessary dependencies
- Consider using array sections for fine-grained dependencies

## Common Pitfalls

### 1. Task Creation Overhead

**Problem**: Creating too many small tasks leads to high overhead.

**Solution**: Implement a cutoff strategy for recursive tasks or increase task granularity.

### 2. Limited Parallelism

**Problem**: Not creating enough tasks to keep all threads busy.

**Solution**: Create more tasks or use different decomposition strategies.

### 3. Data Races

**Problem**: Multiple tasks accessing shared data without proper synchronization.

**Solution**: Use task dependencies, explicit synchronization, or make data private.

### 4. Over-synchronization

**Problem**: Excessive use of taskwait or taskgroup limiting parallelism.

**Solution**: Synchronize only when necessary, use task dependencies instead when possible.

### 5. Deadlocks

**Problem**: Circular dependencies between tasks leading to deadlocks.

**Solution**: Ensure that task dependencies form a directed acyclic graph (DAG).

### 6. Priority Inversion

**Problem**: Low-priority tasks blocking high-priority tasks.

**Solution**: Carefully design task dependencies and priorities, avoiding complex priority schemes.