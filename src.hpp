#pragma once
#include <vector>
#include "Task.hpp"

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
        if (exec_time >= static_cast<long long>(buckets.size())) buckets.resize(exec_time + 1);
        buckets[exec_time].push_back(node);
        all_nodes.push_back(node);
        return node;
    }

    void cancelTask(TaskNode* p) {
        if (!p) return;
        p->active = false;
    }

    std::vector<Task*> tick() {
        Task::incTime();
        ++current_time;
        std::vector<Task*> ready;
        if (current_time < static_cast<long long>(buckets.size())) {
            auto& vec = buckets[current_time];
            for (TaskNode* node : vec) {
                if (!node || !node->active) continue;
                ready.push_back(node->task);
                if (node->period > 0 && node->active) {
                    long long nt = current_time + node->period;
                    node->time = nt;
                    if (nt >= static_cast<long long>(buckets.size())) buckets.resize(nt + 1);
                    buckets[nt].push_back(node);
                } else {
                    node->active = false;
                }
            }
            vec.clear();
        }
        return ready;
    }

private:
    long long current_time;
    std::vector<std::vector<TaskNode*>> buckets;
    std::vector<TaskNode*> all_nodes;
};
