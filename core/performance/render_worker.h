#pragma once
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>
#include <future>
#include <vector>
#include <atomic>

namespace FreeEffect {

class RenderWorker {
public:
    RenderWorker();
    ~RenderWorker();

    void setThreadCount(int count);
    int getThreadCount() const;

    std::future<void> submit(std::function<void()> task);

    void waitForAll();
    void cancel();

    int getPendingCount() const;
    bool isIdle() const;

private:
    void workerThread();

    std::vector<std::thread> m_workers;
    std::queue<std::function<void()>> m_tasks;
    std::mutex m_mutex;
    std::condition_variable m_condition;
    std::condition_variable m_doneCondition;
    bool m_stop = false;
    std::atomic<int> m_pendingTasks{0};
};

} // namespace FreeEffect
