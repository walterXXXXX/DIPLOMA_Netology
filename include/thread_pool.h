#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <vector>
#include <thread>
#include <mutex>
#include <queue>
#include <functional>

class ThreadPool {
public:
	ThreadPool();
	void newTask(std::function<void()> task);
	void join();
	~ThreadPool();

private:
	void threadPoolWorker();

	std::vector<std::thread> threadPool;
	std::mutex mtx;
	std::condition_variable cv;
	std::queue<std::function<void()>> tasks;
	bool exitThreadPool = false;
	int numThreads;

};

#endif THREAD_POOL_H