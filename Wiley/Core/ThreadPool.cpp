#include "ThreadPool.h"

namespace Wiley
{
	ThreadPool threadPool;

	ThreadPool::ThreadPool()
	{
	}

	ThreadPool::~ThreadPool() {
		{
			std::unique_lock<std::mutex> lock(mutex);
			stop = true;
		}

		condition.notify_all();

		for (auto& worker : workers)
			worker.join();
	}

	void ThreadPool::Initialize()
	{
		nThreads = std::thread::hardware_concurrency() - 1;
		for (int i = 0; i < nThreads; i++)
		{
			workers.emplace_back([this] {

				while (true)
				{
					Task task;

					{
						std::unique_lock<std::mutex> lock(mutex);
						condition.wait(lock, [this]() {return stop || !tasks.empty(); });

						if (stop && tasks.empty()) return;

						task = tasks.top();
						tasks.pop();
					}

					activeThreads++;
					task.task();
					activeThreads--;
				}
				});
		}
	}

	void ThreadPool::Submit(std::function<void()> job, TaskPriority priority)
	{
		{
			std::unique_lock<std::mutex> lock(mutex);
			tasks.push({ static_cast<int>(priority), job });
		}

		condition.notify_one();
	}

	void ThreadPool::WaitForAll()
	{
		std::unique_lock<std::mutex> lock(mutex);
		condition.wait(lock, [this]() {return tasks.empty() && activeThreads.load() == 0; });
	}

	size_t ThreadPool::GetActiveThreadCount()
	{
		return activeThreads.load();
	}

	size_t ThreadPool::GetCompletedThreadCount()
	{
		return completedThreads.load();
	}

	size_t ThreadPool::GetIdleThreadCount() {
		return workers.size() - activeThreads.load();
	}

	ThreadPool& ThreadPool::GetThreadPool()
	{
		return threadPool;
	}



}
