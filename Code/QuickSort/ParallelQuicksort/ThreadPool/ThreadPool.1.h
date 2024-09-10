/*
 * Altered version by Simon Lang: Now using a stack instead of a queue
 * 
 * Original copyright (c) 2012 Jakob Progsch, Václav Zeman
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 * 
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 * 
 * 	1. The origin of this software must not be misrepresented; you must not
 * 	claim that you wrote the original software. If you use this software
 * 	in a product, an acknowledgment in the product documentation would be
 * 	appreciated but is not required.
 * 	
 * 	2. Altered source versions must be plainly marked as such, and must not be
 * 	misrepresented as being the original software.
 * 	3. This notice may not be removed or altered from any source
 * 	distribution.
 */

#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <vector>
#include <stack>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>


class ThreadPool {
public:
    ThreadPool(size_t);
		uint64_t size;
    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args) 
        -> std::future<typename std::result_of<F(Args...)>::type>;
		void waitUntilComplete();
    ~ThreadPool();
private:
    // need to keep track of threads so we can join them
    std::vector< std::thread > workers;
    // the task stack
    std::vector<std::stack< std::function<void()> >> tasks;
    
    // synchronization
    std::vector<std::mutex> stack_mutexes;
    std::vector<std::condition_variable> conditions;
    bool stop;

		std::atomic<uint64_t> working;
		std::mutex wait_all_mutex;
		std::condition_variable wait_all;
};
 
// the constructor just launches some amount of workers
inline ThreadPool::ThreadPool(size_t threads)
    :  size(threads), stop(false), working(0)
{
    for(size_t i = 0;i<threads;++i) {
				tasks.emplace_back(std::stack<std::function<void()>>());
				stack_mutexes.emplace_back(std::mutex());

        workers.emplace_back(
            [i, this]
            {
                for(;;)
                {

                    std::function<void()> task;

                    {
                        std::unique_lock<std::mutex> lock(this->stack_mutexes[i]);
                        this->conditions[i].wait(lock,
                            [this]{ return this->stop || !this->tasks.empty(); });
                        if(this->stop && this->tasks.empty())
                            return;
												working++;
												//std::cout << "+" << working << std::endl;
                        task = std::move(this->tasks[i].top());
                        this->tasks[i].pop();
                    }

                    task();

										working--;
										//std::cout << "-" << working << std::endl;
										bool running = false;
										for (uint32_t j=0; j<threads; j++) {
											running |= !tasks[j].empty();
										}
										if (working<=0&&!running) wait_all.notify_all();
                }
            }
        );
		}
}

// add new work item to the pool
template<class F, class... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args, uint64_t thIndex) 
    -> std::future<typename std::result_of<F(Args...)>::type>
{
    using return_type = typename std::result_of<F(Args...)>::type;

    auto task = std::make_shared< std::packaged_task<return_type()> >(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        
    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(stack_mutexes[thIndex]);

        // don't allow enqueueing after stopping the pool
        if(stop)
            throw std::runtime_error("enqueue on stopped ThreadPool");

        tasks[thIndex].emplace([task](){ (*task)(); });
    }
    condition[i].notify_one();
    return res;
}

void ThreadPool::waitUntilComplete()
{
	std::unique_lock<std::mutex> lock(this->wait_all_mutex);
	wait_all.wait(lock);
}

// the destructor joins all threads
inline ThreadPool::~ThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(stack_mutex);
        stop = true;
    }

    condition.notify_all();
    for(std::thread &worker: workers)
        worker.join();
}

#endif
