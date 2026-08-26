#include "render_worker.h"
#include <algorithm>

namespace FreeEffect {

RenderWorker::RenderWorker() {
    setThreadCount(static_cast<int>(std::thread::hardware_concurrency()));
}

RenderWorker::~RenderWorker() {
    cancel();
}

void RenderWorker::setThreadCount(int count) {
    cancel();

    m_stop = false;
    int threads = std::max(1, count);
    m_workers.reserve(threads);
    for (int i = 0; i < threads; ++i) {
        m_workers.emplace_back(&RenderWorker::workerThread, this);
    }
}

int RenderWorker::getThreadCount() const {
    return static_cast<int>(m_workers.size());
}

std::future<void> RenderWorker::submit(std::function<void()> task) {
    auto promise = std::make_shared<std::promise<void>>();
    auto future = promise->get_future();

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_tasks.push([promise, task = std::move(task)]() {
            task();
            promise->set_value();
        });
        m_pendingTasks++;
    }
    m_condition.notify_one();
    return future;
}

void RenderWorker::waitForAll() {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_doneCondition.wait(lock, [this] {
        return m_tasks.empty() && m_pendingTasks.load() == 0;
    });
}

void RenderWorker::cancel() {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_stop = true;
    }
    m_condition.notify_all();
    for (auto& t : m_workers) {
        if (t.joinable()) t.join();
    }
    m_workers.clear();
}

int RenderWorker::getPendingCount() const {
    return m_pendingTasks.load();
}

bool RenderWorker::isIdle() const {
    return m_pendingTasks.load() == 0;
}

void RenderWorker::workerThread() {
    while (true) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_condition.wait(lock, [this] {
                return m_stop || !m_tasks.empty();
            });

            if (m_stop && m_tasks.empty()) return;

            task = std::move(m_tasks.front());
            m_tasks.pop();
        }

        task();
        m_pendingTasks--;

        if (m_pendingTasks.load() == 0 && m_tasks.empty()) {
            m_doneCondition.notify_all();
        }
    }
}

} // namespace FreeEffect
