#pragma once

#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <vector>
using namespace std;

class ThreadPool {
public:
  ThreadPool(int n) {
    for (int i = 0; i < n; i++) {
      workers.push_back(thread([this]() {
        while (true) {
          function<void()> task;
          {
            unique_lock<mutex> lock(queueMutex);
            cv.wait(lock, [this]() { return stop || !tasks.empty(); });
            if (stop && tasks.empty()) return;
            task = move(tasks.front());
            tasks.pop();
          }
          task();
        }
      }));
    }
  }

  template <typename F, typename... Args>
  auto submit(F&& f, Args&&... args) -> future<decltype(f(args...))> {
    using R = decltype(f(args...));
    auto bound = bind(forward<F>(f), forward<Args>(args)...);
    auto task = make_shared<packaged_task<R()>>(bound);
    future<R> result = task->get_future();

    {
      lock_guard<mutex> lock(queueMutex);
      if (stop) throw runtime_error("submit on a stopped ThreadPool");
      tasks.push([task]() { (*task)(); });
    }
    cv.notify_one();
    return result;
  }

  void shutdown() {
    {
      lock_guard<mutex> lock(queueMutex);
      stop = true;
    }
    cv.notify_all();
    for (auto& t : workers) {
      if (t.joinable()) t.join();
    }
  }

  ~ThreadPool() { shutdown(); }

  ThreadPool(const ThreadPool&) = delete;
  ThreadPool& operator=(const ThreadPool&) = delete;

private:
  vector<thread> workers;
  queue<function<void()>> tasks;
  mutex queueMutex;
  condition_variable cv;
  bool stop = false;
};