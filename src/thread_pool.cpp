#include <myexecution/schedulers/thread_pool_scheduler.hpp>

namespace execution {

auto thread_pool::run() -> void {
  for (;;) {
    std::function<void()> task{};
    {
      std::unique_lock guard(mtx_);
      cv_.wait(guard, [this]() { return done_ || !tasks_.empty(); });
      if (done_ || tasks_.empty()) {
        return;
      }
      task = tasks_.front();
      tasks_.pop();
    }
    task();
  }
}

// Notably, this enables currently running tasks to continue running, even
// after the thread pool exits scope. This may raise the concern of dangling
// references—the worker threads contain a reference to the thread pool, which
// may expire before the thread exits. However, the code ensures that this
// reference is never accessed. Thus, we maintain our invariant.
thread_pool::~thread_pool() {
  {
    std::lock_guard guard(mtx_);
    done_ = true;
  }
  cv_.notify_all();
  for (auto &worker : workers_) {
    worker.join();
  }
}

auto thread_pool::post(std::function<void()> task) -> void {
  {
    std::lock_guard guard(mtx_);
    tasks_.emplace(std::move(task));
  }
  cv_.notify_one();
}

// "deducing this" wasn't not convered in class, but I have seen
// it in conference talks and wanted to practice using it. Doing so eliminates
// the unecessary space overhead of more pointer copies (although the compiler
// might optimize them away anyways). Generally, I felt that sticking to
// references over pointers was a good idea.
auto thread_pool::get_scheduler(this thread_pool &self) -> scheduler {
  return scheduler{self};
}

} // namespace execution
