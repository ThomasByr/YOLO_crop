/*********************************************************
Copyright (c) 2022, ThomasByr.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice,
  this list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of the crop_img authors nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*********************************************************/

#ifndef __ctpl_thread_pool_H__
#define __ctpl_thread_pool_H__

#include <atomic>
#include <exception>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

// thread pool to run user's functors with signature
//      ret func(int id, other_params)
// where id is the index of the thread that runs the functor
// ret is some return type

namespace ctpl {

namespace detail {

template <typename T> class Queue {
public:
  bool push(T const &value) {
    std::unique_lock<std::mutex> lock(this->mutex);
    this->q.push(value);
    return true;
  }
  // deletes the retrieved element, do not use for non integral types
  bool pop(T &v) {
    std::unique_lock<std::mutex> lock(this->mutex);
    if (this->q.empty()) return false;
    v = this->q.front();
    this->q.pop();
    return true;
  }
  bool empty() {
    std::unique_lock<std::mutex> lock(this->mutex);
    return this->q.empty();
  }

private:
  std::queue<T> q;
  std::mutex mutex;
};

} // namespace detail

class thread_pool {

public:
  thread_pool() { this->init(); }
  thread_pool(int n_thrds) {
    this->init();
    this->resize(n_thrds);
  }

  // the destructor waits for all the functions in the queue to be finished
  ~thread_pool() { this->stop(true); }

  // get the number of running threads in the pool
  int size() { return static_cast<int>(this->threads.size()); }

  // number of idle threads
  int n_idle() { return this->n_waiting; }
  std::thread &get_thread(int i) { return *this->threads[i]; }

  // change the number of threads in the pool
  // should be called from one thread, otherwise be careful to not interleave,
  // also with this->stop() n_thrds must be >= 0
  void resize(int n_thrds) {
    if (!this->is_stop && !this->is_done) {
      int oldn_thrds = static_cast<int>(this->threads.size());
      if (oldn_thrds <= n_thrds) { // if the number of threads is increased
        this->threads.resize(n_thrds);
        this->flags.resize(n_thrds);

        for (int i = oldn_thrds; i < n_thrds; ++i) {
          this->flags[i] = std::make_shared<std::atomic<bool>>(false);
          this->set_thread(i);
        }
      } else { // the number of threads is decreased
        for (int i = oldn_thrds - 1; i >= n_thrds; --i) {
          *this->flags[i] = true; // this thread will finish
          this->threads[i]->detach();
        }
        {
          // stop the detached threads that were waiting
          std::unique_lock<std::mutex> lock(this->mutex);
          this->cv.notify_all();
        }
        // safe to delete because the threads are detached
        this->threads.resize(n_thrds);
        // safe to delete because the threads have copies of
        // shared_ptr of the flags, not originals
        this->flags.resize(n_thrds);
      }
    }
  }

  // empty the queue
  void clear_queue() {
    std::function<void(int id)> *_f;
    while (this->q.pop(_f))
      delete _f; // empty the queue
  }

  // pops a functional wrapper to the original function
  std::function<void(int)> pop() {
    std::function<void(int id)> *_f = nullptr;
    this->q.pop(_f);
    std::unique_ptr<std::function<void(int id)>> func(
        _f); // at return, delete the function even if an exception occurred
    std::function<void(int)> f;
    if (_f) f = *_f;
    return f;
  }

  // wait for all computing threads to finish and stop all threads
  // may be called asynchronously to not pause the calling thread while waiting
  // if wait == true, all the functions in the queue are run, otherwise the
  // queue is cleared without running the functions
  void stop(bool wait = false) {
    if (!wait) {
      if (this->is_stop) return;
      this->is_stop = true;
      for (int i = 0, n = this->size(); i < n; ++i) {
        *this->flags[i] = true; // command the threads to stop
      }
      this->clear_queue(); // empty the queue
    } else {
      if (this->is_done || this->is_stop) return;
      this->is_done = true; // give the waiting threads a command to finish
    }
    {
      std::unique_lock<std::mutex> lock(this->mutex);
      this->cv.notify_all(); // stop all waiting threads
    }
    // wait for the computing threads to finish
    for (int i = 0; i < static_cast<int>(this->threads.size()); ++i) {
      if (this->threads[i]->joinable()) this->threads[i]->join();
    }
    // if there were no threads in the pool but some functors in the queue, the
    // functors are not deleted by the threads therefore delete them here
    this->clear_queue();
    this->threads.clear();
    this->flags.clear();
  }

  template <typename F, typename... Rest>
  auto push(F &&f, Rest &&...rest) -> std::future<decltype(f(0, rest...))> {
    auto pck =
        std::make_shared<std::packaged_task<decltype(f(0, rest...))(int)>>(
            std::bind(std::forward<F>(f), std::placeholders::_1,
                      std::forward<Rest>(rest)...));
    auto _f = new std::function<void(int id)>([pck](int id) { (*pck)(id); });
    this->q.push(_f);
    std::unique_lock<std::mutex> lock(this->mutex);
    this->cv.notify_one();
    return pck->get_future();
  }

  // run the user's function that excepts argument int - id of the running
  // thread. returned value is templatized operator returns std::future, where
  // the user can get the result and rethrow the catched exceptions
  template <typename F> auto push(F &&f) -> std::future<decltype(f(0))> {
    auto pck = std::make_shared<std::packaged_task<decltype(f(0))(int)>>(
        std::forward<F>(f));
    auto _f = new std::function<void(int id)>([pck](int id) { (*pck)(id); });
    this->q.push(_f);
    std::unique_lock<std::mutex> lock(this->mutex);
    this->cv.notify_one();
    return pck->get_future();
  }

private:
  // deleted
  thread_pool(const thread_pool &);            // = delete;
  thread_pool(thread_pool &&);                 // = delete;
  thread_pool &operator=(const thread_pool &); // = delete;
  thread_pool &operator=(thread_pool &&);      // = delete;

  void set_thread(int i) {
    // a copy of the shared ptr to the flag
    std::shared_ptr<std::atomic<bool>> flag(this->flags[i]);
    auto f = [this, i, flag /* a copy of the shared ptr to the flag */]() {
      std::atomic<bool> &_flag = *flag;
      std::function<void(int id)> *_f;
      bool is_pop = this->q.pop(_f);
      while (true) {
        while (is_pop) { // if there is anything in the queue

          // at return, delete the function even if an exception
          // occurred
          std::unique_ptr<std::function<void(int id)>> func(_f);
          (*_f)(i);
          if (_flag)
            return; // the thread is wanted to stop, return even if the queue is
                    // not empty yet
          else
            is_pop = this->q.pop(_f);
        }
        // the queue is empty here, wait for the next command
        std::unique_lock<std::mutex> lock(this->mutex);
        ++this->n_waiting;
        this->cv.wait(lock, [this, &_f, &is_pop, &_flag]() {
          is_pop = this->q.pop(_f);
          return is_pop || this->is_done || _flag;
        });
        --this->n_waiting;
        if (!is_pop)
          return; // if the queue is empty and this->is_done == true or *flag
                  // then return
      }
    };
    this->threads[i].reset(
        new std::thread(f)); // compiler may not support std::make_unique()
  }

  void init() {
    this->n_waiting = 0;
    this->is_stop = false;
    this->is_done = false;
  }

  std::vector<std::unique_ptr<std::thread>> threads;
  std::vector<std::shared_ptr<std::atomic<bool>>> flags;
  detail::Queue<std::function<void(int id)> *> q;
  std::atomic<bool> is_done;
  std::atomic<bool> is_stop;
  std::atomic<int> n_waiting; // how many threads are waiting

  std::mutex mutex;
  std::condition_variable cv;
};

} // namespace ctpl

#endif // __ctpl_thread_pool_H__
