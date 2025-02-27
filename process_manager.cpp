// command to execute the program : g++ process_manager.cpp -o process_manager -pthread -std=c++14

#include <iostream>
#include <vector>
#include <algorithm>
#include <queue>
#include <numeric> // For std::accumulate
#include <memory> // For std::unique_ptr
#include <stdexcept>
#include <mutex>  // Needed for std::mutex and std::unique_lock
#include <condition_variable> // Needed for std::condition_variable
#include <functional> //Needed for std::function

// --- Process Definition ---

enum ProcessState {
    NEW,
    READY,
    RUNNING,
    WAITING,
    TERMINATED
};

struct Process {
    int pid;
    ProcessState state;
    int priority; 
    int burstTime;
    int arrivalTime;
    int remainingTime;
    int waitingTime;
    int turnaroundTime;
    std::vector<int> resources; 

    Process(int id, int arrival, int burst) :
        pid(id), state(NEW), priority(0), burstTime(burst), arrivalTime(arrival),
        remainingTime(burst), waitingTime(0), turnaroundTime(0) {}

    void printProcessInfo() const {
        std::cout << "PID: " << pid << ", State: " << state
                  << ", Arrival: " << arrivalTime << ", Burst: " << burstTime
                  << ", Remaining: " << remainingTime << ", Waiting: " << waitingTime
                  << ", Turnaround: " << turnaroundTime << std::endl;
    }
    bool operator<(const Process& other) const {
        return remainingTime > other.remainingTime; // For SJF (priority queue needs > for min-heap)
    }
};

// --- Scheduler Definition ---
class Scheduler {
public:
    virtual void addProcess(Process process) = 0;
    virtual Process getNextProcess() = 0;
    virtual bool hasProcesses() const = 0;
    virtual ~Scheduler() {} 
    virtual std::string getName() const = 0; 
};

// --- FCFS Scheduler ---
class FCFSScheduler : public Scheduler {
private:
    std::queue<Process> readyQueue;

public:
    void addProcess(Process process) override {
        readyQueue.push(process);
    }

    Process getNextProcess() override {
        if (!readyQueue.empty()) {
            Process next = readyQueue.front();
            readyQueue.pop();
            return next;
        }
        return Process(-1, -1, -1); 
    }

    bool hasProcesses() const override {
        return !readyQueue.empty();
    }
    std::string getName() const override {
        return "FCFS";
    }
};

// --- SJF Scheduler ---
class SJFScheduler : public Scheduler {
private:
  std::priority_queue<Process> readyQueue; 

public:
    void addProcess(Process process) override {
        readyQueue.push(process); 
    }

    Process getNextProcess() override {
        if(!readyQueue.empty()){
            Process next = readyQueue.top();
            readyQueue.pop();
            return next;
        }
        return Process(-1,-1,-1); 
    }

    bool hasProcesses() const override {
        return !readyQueue.empty();
    }
     std::string getName() const override {
        return "SJF";
    }
};

// --- Round Robin Scheduler ---

class RRScheduler : public Scheduler {
private:
    std::queue<Process> readyQueue;
    int timeQuantum;

public:
    RRScheduler(int quantum) : timeQuantum(quantum) {}

    void addProcess(Process process) override {
        readyQueue.push(process);
    }

  Process getNextProcess() override {
        if (!readyQueue.empty()) {
            Process next = readyQueue.front();
            readyQueue.pop();

            if (next.remainingTime > timeQuantum) {
                next.remainingTime -= timeQuantum;
                readyQueue.push(next); 
            }
            return next;
        }
        return Process(-1, -1, -1); 
    }

    bool hasProcesses() const override {
        return !readyQueue.empty();
    }
    std::string getName() const override {
        return "RR";
    }
};

// --- Simplified Shared Memory (for IPC) ---

class SharedMemory {
private:
    std::vector<int> data; 
    int size;
    static constexpr int DEFAULT_SIZE = 10; 
public:
    SharedMemory(int size_ = DEFAULT_SIZE) : size(size_) {
      data.resize(size);
      // Initialize shared memory 
      for(int i = 0; i < size; i++)
        data[i] = 0;
    }


    // Simplified read and write 
    int read(int index) {
      if(index < 0 || index >= size)
          throw std::out_of_range("SharedMemory::read: Index out of range");
        return data[index];
    }

    void write(int index, int value) {
      if(index < 0 || index >= size)
            throw std::out_of_range("SharedMemory::write: Index out of range");
        data[index] = value;
    }
};
// --- Semaphore ---
class Semaphore {
    private:
        int count;
        std::mutex mtx;
        std::condition_variable cv;

    public:
        Semaphore(int initialCount) : count(initialCount) {}

        void wait() {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [this] { return count > 0; }); 
            count--;
        }

        void signal() {
            std::unique_lock<std::mutex> lock(mtx);
            count++;
            cv.notify_one();  
        }
};

// --- Simple Deadlock Detection (Circular Wait) ---

bool detectDeadlock(const std::vector<Process>& processes) {
    
    std::vector<std::vector<int>> allocationGraph(processes.size());
    for (size_t i = 0; i < processes.size(); ++i) {
        for (int resource : processes[i].resources) {
            for (size_t j = 0; j < processes.size(); ++j) {
                if (i != j) {
                   
                    if (std::find(processes[j].resources.begin(), processes[j].resources.end(), resource) != processes[j].resources.end())
                        allocationGraph[i].push_back(j); // Add edge i -> j

                }
            }
        }
    }

    // Simple cycle detection using Depth-First Search (DFS)
    std::vector<bool> visited(processes.size(), false);
    std::vector<bool> recursionStack(processes.size(), false);

    std::function<bool(int)> isCyclicUtil = [&](int v) {
        if (!visited[v]) {
            visited[v] = true;
            recursionStack[v] = true;

            for (int neighbor : allocationGraph[v]) {
                if (!visited[neighbor] && isCyclicUtil(neighbor))
                    return true;
                else if (recursionStack[neighbor])
                    return true;
            }
        }
        recursionStack[v] = false; 
        return false;
    };

    for (size_t i = 0; i < processes.size(); ++i) {
        if (isCyclicUtil(i))
            return true; 
    }

    return false; 
}


// --- Main Simulation Function ---

void simulate(std::unique_ptr<Scheduler>& scheduler, int numProcesses, int maxBurstTime, int maxArrivalTime) {
    // Create processes
    std::vector<Process> processes;
    for (int i = 0; i < numProcesses; ++i) {
        int arrivalTime = rand() % (maxArrivalTime + 1);
        int burstTime = 1 + (rand() % maxBurstTime); 
        processes.emplace_back(i, arrivalTime, burstTime);
        scheduler->addProcess(processes.back()); 
    }

    //for storing the completed process
    std::vector<Process> completedProcesses;


    SharedMemory sharedMem(20); 
     Semaphore semaphore(1);   


    int currentTime = 0;
    while (scheduler->hasProcesses() || !processes.empty())
    {
         // Add processes that have arrived to the scheduler
        for (auto it = processes.begin(); it != processes.end(); ) {
            if (it->arrivalTime <= currentTime) {
                it->state = READY; 
                scheduler->addProcess(*it);
                it = processes.erase(it); 
            } else {
                ++it;
            }
        }
        if(scheduler->hasProcesses()){
            Process currentProcess = scheduler->getNextProcess();
            currentProcess.state = RUNNING;
            // Simulate running the process for one time unit (or until completion/preemption)

            // --- Critical Section (Accessing Shared Memory) ---
            semaphore.wait(); // Acquire the semaphore
            // Simulate writing to shared memory (for demonstration)
            sharedMem.write(currentProcess.pid % 20, currentProcess.pid * 10); 
             // Simulate reading from shared memory
            int readValue = sharedMem.read(currentProcess.pid % 20);
            semaphore.signal(); //release lock
            // --- End Critical Section ---

            currentProcess.remainingTime--;

            // Update waiting time for processes still in scheduler
            // you cannot access the internal readyQueue

            //check deadlock detection for every 10 time unit
            if(currentTime % 10 == 0){
                if(detectDeadlock(processes))
                  std::cerr << "Deadlock detected at time: " << currentTime << "!\n";
            }

            if(currentProcess.remainingTime <= 0){
                currentProcess.state = TERMINATED; //process completed
                currentProcess.turnaroundTime = currentTime + 1 - currentProcess.arrivalTime;
                currentProcess.waitingTime = currentProcess.turnaroundTime - currentProcess.burstTime;
                completedProcesses.push_back(currentProcess); // Store completed process
            } else {
                currentProcess.state = READY;
                scheduler->addProcess(currentProcess);
            }

        }
        currentTime++;

    }

    //print statistics
    std::cout << "\n--- Simulation Results (" << scheduler->getName() << ") ---\n";
    double totalWaitingTime = 0;
    double totalTurnaroundTime = 0;

    for(const auto& process : completedProcesses){
        process.printProcessInfo();
        totalWaitingTime += process.waitingTime;
        totalTurnaroundTime += process.turnaroundTime;
    }

    double avgWaitingTime = totalWaitingTime / completedProcesses.size();
    double avgTurnaroundTime = totalTurnaroundTime / completedProcesses.size();

    std::cout << "Average Waiting Time: " << avgWaitingTime << std::endl;
    std::cout << "Average Turnaround Time: " << avgTurnaroundTime << std::endl;

}


int main() {
    srand(time(0)); 

    int numProcesses, maxBurstTime, maxArrivalTime, schedulerChoice, timeQuantum;

    std::cout << "Enter the number of processes: ";
    std::cin >> numProcesses;
    std::cout << "Enter the maximum burst time: ";
    std::cin >> maxBurstTime;
    std::cout << "Enter the maximum arrival time: ";
    std::cin >> maxArrivalTime;

    std::cout << "Choose a scheduling algorithm:\n";
    std::cout << "1. FCFS\n2. SJF\n3. Round Robin\n";
    std::cout << "Enter your choice (1-3): ";
    std::cin >> schedulerChoice;

    std::unique_ptr<Scheduler> scheduler; 

    switch (schedulerChoice) {
        case 1:
            scheduler = std::make_unique<FCFSScheduler>();
            break;
        case 2:
            scheduler = std::make_unique<SJFScheduler>();
            break;
        case 3:
            std::cout << "Enter the time quantum for Round Robin: ";
            std::cin >> timeQuantum;
            scheduler = std::make_unique<RRScheduler>(timeQuantum);
            break;
        default:
            std::cerr << "Invalid scheduler choice.  Exiting.\n";
            return 1;
    }

     try {
        simulate(scheduler, numProcesses, maxBurstTime, maxArrivalTime);
    } catch (const std::exception& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
        return 1;
    }


    return 0;
}
