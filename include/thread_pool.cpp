#include "thread_pool.h"

ThreadPool::ThreadPool() {

	numThreads = std::thread::hardware_concurrency();
	for (int i = 0; i < numThreads; ++i) {
		threadPool.emplace_back(std::thread(&ThreadPool::threadPoolWorker, this));
	}
}

void ThreadPool::newTask(std::function<void()> task) {

	std::lock_guard<std::mutex> lock(mtx);
	tasks.push(task);
	cv.notify_one();
}

void ThreadPool::join() {
	{
		std::lock_guard<std::mutex> lock(mtx);
		exitThreadPool = true;
		cv.notify_all();
	}

	for (auto& t : threadPool) {
		t.join();
	}
}

ThreadPool::~ThreadPool() {

}

void ThreadPool::threadPoolWorker() {

	std::unique_lock<std::mutex> lock(mtx);
	while (!exitThreadPool || !tasks.empty()) {
		if (tasks.empty()) {
			cv.wait(lock);
		}
		else {
			auto task = tasks.front();
			tasks.pop();
			lock.unlock();
			task();
			lock.lock();
		}
	}
}
