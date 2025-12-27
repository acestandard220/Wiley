#pragma once
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <type_traits>
#include <utility>
#include <iostream>
#include <memory>

namespace Wiley {
	
	class ThreadPool
	{
		enum class TaskPriority : int
		{
			Low = 0,
			Minor = 10,
			Normal = 20,
			High = 30,
			Urgent = 40,
			Critical = 50
		};

		struct Task
		{
			int priority;
			std::function<void()> task;

			bool operator<(const Task& other)const
			{
				return priority < other.priority;
			}
		};

	public:
		ThreadPool();
		~ThreadPool();

		void Initialize();

		void Submit(std::function<void()> job, TaskPriority priority = TaskPriority::Minor);

		template<typename F, typename...Args>
		auto Submit(F&& f, TaskPriority priority, Args&&... args)
			-> std::future<typename std::invoke_result<F, Args...>::type>
		{
			using return_type = typename std::invoke_result<F, Args...>::type;

			auto task = std::make_shared<std::packaged_task<return_type()>>(
				std::bind(std::forward<F>(f), std::forward<Args>(args)...)
			);

			std::future<return_type> result = task->get_future();

			{
				std::unique_lock<std::mutex> lock(mutex);
				tasks.push({ static_cast<int>(priority), [task]() { (*task)(); } });
			}

			condition.notify_one();

			return result;
		}

		void WaitForAll();

		size_t GetActiveThreadCount();
		size_t GetCompletedThreadCount();
		size_t GetIdleThreadCount();

		static ThreadPool& GetThreadPool();
	private:
		std::vector<std::thread> workers;
		std::priority_queue<Task> tasks;
		std::mutex mutex;
		std::condition_variable condition;
		bool stop = false;

		inline static int nThreads = 8;

		std::atomic<size_t> activeThreads{ 0 };
		std::atomic<size_t> completedThreads{ 0 };
	};

#define gThreadPool ThreadPool::GetThreadPool()

}

