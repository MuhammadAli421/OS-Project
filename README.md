# OS-Project
OS Project Process Management System/Process Scheduling

Process Management System

1. Introduction
This project is a process scheduler simulation that demonstrates various CPU scheduling algorithms. It models the behavior of processes in an operating system, including their arrival, execution, waiting, and completion.
•	Purpose and Objectives: The primary purpose is to illustrate and compare the performance of different scheduling algorithms (First-Come, First-Served (FCFS), Shortest Job First (SJF), and Round Robin (RR)). The objectives include:
•	Implementing the core logic of each scheduling algorithm.
•	Simulating process creation and management.
•	Calculating and displaying performance metrics (waiting time, turnaround time).
•	Demonstrating basic inter-process communication (IPC) using shared memory.
•	Implementing a semaphore for synchronization.
•	Implementing a basic deadlock detection mechanism.
•	Importance in Real-World Applications: Process scheduling is a fundamental function of any operating system. Efficient scheduling algorithms are crucial for maximizing CPU utilization, minimizing response time, and ensuring fairness among processes. This simulation provides a simplified model to understand the trade-offs involved in different scheduling approaches. The concepts of shared memory, semaphores, and deadlock detection are also critical in real-world operating systems for managing concurrent processes and preventing system failures.

2. Project Scope
•	Features and Functionalities:
•	Process Creation: Generates processes with random arrival and burst times.
•	Scheduling Algorithms: Implements FCFS, SJF, and RR scheduling.
•	Process State Management: Tracks process states (NEW, READY, RUNNING, WAITING, TERMINATED).
•	Performance Metrics: Calculates and displays average waiting time and average turnaround time.
•	Shared Memory: Simulates a shared memory region for inter-process communication.
•	Semaphore: Provides a semaphore implementation for synchronizing access to shared resources.
•	Deadlock Detection: includes simple circular wait deadlock detection.

•	Target Users or Systems: This simulation is primarily intended for educational purposes. It can be used by students learning about operating system concepts or anyone interested in understanding the basics of process scheduling. It is not intended for direct use in a production operating system.

3. Technology Stack
•	Programming Languages: C++14
•	Development Tools and IDEs: The code can be compiled with any C++14 compliant compiler (e.g., g++). Virtual Machine, Linux IDE is required.
•	Libraries and Frameworks:
•	Standard Template Library (STL): Uses standard C++ containers ( std::vector, std::queue, std::priority_queue ), algorithms ( std::algorithm ), and utilities ( std::unique_ptr, std::mutex, std::condition_variable, std::function ).
•	No external libraries are required.

4. Operating System Concepts Used
•	Process Management:
•	Scheduling: Implements three common scheduling algorithms: FCFS, SJF, and Round Robin.
•	Process States: Models the different states a process can be in (NEW, READY, RUNNING, WAITING, TERMINATED).
•	Threading: While not explicitly using multiple threads for process execution within the simulation, the code uses std::mutex and std::condition_variable (which are threading primitives) to implement the semaphore. This demonstrates the underlying concepts of thread synchronization.
•	Inter-Process Communication (IPC):
•	Shared Memory: a very basic shared memory is used.
•	Synchronization
•	Semaphore: Includes a semaphore implementation using mutex and condition variable.
•	Deadlock Detection:
•	Circular Wait: Implements a simple deadlock detection algorithm that checks for circular dependencies in resource allocation.

5. Implementation Details
•	Core Algorithms and Logic:
•	FCFSScheduler: Processes are managed in a FIFO queue. The process at the front of the queue is selected for execution.
•	SJFScheduler: Processes are stored in a priority queue, ordered by their remaining burst time (shortest remaining time first).
•	RRScheduler: Processes are managed in a FIFO queue. Each process is given a fixed time quantum. If a process doesn't complete within its time quantum, it's moved to the back of the queue.
•	SharedMemory: A simple std::vector simulates shared memory. Read and write operations are provided, but without any real memory protection mechanisms.
•	Semaphore: Implemented using a mutex (std::mutex) and a condition variable (std::condition_variable). The wait() method decrements the semaphore count and blocks if the count is zero. The signal() method increments the count and notifies a waiting thread (if any).
•	Deadlock Detection: A depth-first search is performed in resource allocation graph, to detect circular wait.

•	How Different Modules Interact:
•	The main function initializes the simulation, creates processes, and selects the scheduling algorithm.
•	The simulate function is the main loop of the simulation. It handles process arrival, scheduling, execution, and termination.
•	The Scheduler abstract class defines the interface for all scheduling algorithms. Concrete scheduler classes ( FCFSScheduler, SJFScheduler, RRScheduler ) implement this interface.
•	The Process struct represents a single process, storing its state, burst time, arrival time, and other relevant information.
•	The SharedMemory class simulates shared memory. Processes can access shared memory (though this access is simplified and not truly protected).
•	The Semaphore provides synchronization, used to protect access the shared memory.
•	detectDeadlock function checks for the deadlock.

•	Code Snippets:
      // Process struct
struct Process {
    int pid;
    ProcessState state;
    // ... other members ...
};

// Scheduler interface
class Scheduler {
public:
    virtual void addProcess(Process process) = 0;
    virtual Process getNextProcess() = 0;
    // ... other methods ...
};

// Semaphore implementation (part)
class Semaphore {
    int count;
    std::mutex mtx;
    std::condition_variable cv;
public:
    void wait() {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this] { return count > 0; });
        count--;
    }
};
    

6. Conclusion
•	Summary of Project Outcomes: This project successfully implements a basic process scheduler simulation. It demonstrates the core concepts of process management, scheduling algorithms, inter-process communication, synchronization and deadlock detection. The simulation allows for a comparison of FCFS, SJF, and Round Robin scheduling in terms of average waiting time and average turnaround time.

•	Key Takeaways and Learnings:
•	Different scheduling algorithms have different performance characteristics. SJF generally minimizes average waiting time, but requires knowledge of future burst times. Round Robin provides fairness but can have higher average waiting times if the time quantum is not chosen appropriately.
•	Synchronization mechanisms (like semaphores) are essential for coordinating access to shared resources in a concurrent environment.
•	Deadlocks are a potential problem in multi-process systems, and mechanisms are needed to detect and prevent them.
•	C++ standard library provides powerful tools for implementing operating system concepts.
•	Using unique_ptr is good practice for the resource management.


