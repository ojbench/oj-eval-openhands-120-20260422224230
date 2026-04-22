#pragma once
#include <vector>
#include <unordered_map>
#include <cstddef>
#include <string>
#include <iostream>

class Task {
public:
    Task(std::string name, std::size_t time, std::size_t period)
        : name(name), first_interval(time), period(period) {}
    void set() {}
    std::size_t getFirstInterval() const { return first_interval; }
    std::size_t getPeriod() const { return period; }
    void execute() { std::cout << "Task: " << name << " excuted" << std::endl; }
    static void incTime() {}
    static std::size_t getCnt() { return 0; }
private:
    std::string name;
    const std::size_t first_interval;
    const std::size_t period;
};

class TaskNode {
    friend class Timer;
public:
    TaskNode() = default;
private:
    Task* task{nullptr};
    TaskNode* next{nullptr};
    TaskNode* prev{nullptr};
    long long time{0};
    bool active{true};
    long long period{0};
};

class TimingWheel {
    friend class Timer;
public:
    TimingWheel(std::size_t s, std::size_t itv) : size(s), interval(itv) {}
    ~TimingWheel() = default;
private:
    const std::size_t size;
    const std::size_t interval;
    std::size_t current_slot{0};
};

class Timer {
public:
    Timer() : current_time(0) {}
    ~Timer() {
        for (auto p : all_nodes) delete p;
    }

    TaskNode* addTask(Task* task) {
        if (!task) return nullptr;
        auto* node = new TaskNode();
        node->task = task;
        node->period = static_cast<long long>(task->getPeriod());
        node->active = true;
        long long first = static_cast<long long>(task->getFirstInterval());
        long long exec_time = current_time + first;
        if (exec_time < current_time) exec_time = current_time;
        node->time = exec_time;
        buckets[exec_time].push_back(node);
        all_nodes.push_back(node);
        return node;
    }

    void cancelTask(TaskNode* p) {
        if (!p) return;
        p->active = false;
    }

    std::vector<Task*> tick() {
        ++current_time;
        std::vector<Task*> ready;
        auto it = buckets.find(current_time);
        if (it != buckets.end()) {
            auto& vec = it->second;
            for (TaskNode* node : vec) {
                if (!node || !node->active) continue;
                ready.push_back(node->task);
                if (node->period > 0 && node->active) {
                    node->time = current_time + node->period;
                    buckets[node->time].push_back(node);
                } else {
                    node->active = false;
                }
            }
            buckets.erase(it);
        }
        return ready;
    }

private:
    long long current_time;
    std::unordered_map<long long, std::vector<TaskNode*>> buckets;
    std::vector<TaskNode*> all_nodes;
};
