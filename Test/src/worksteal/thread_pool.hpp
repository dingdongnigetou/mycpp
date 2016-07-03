#include "worker.hpp"

class thread_pool
{
public:

	thread_pool(size_t thread_num = std::thread::hardware_concurrency(), bool enable_steal = true)
	{
		workers_ = std::make_shared<std::vector<std::shared_ptr<worker_t>>>();
		for (size_t i = 0; i < thread_num; i++)
		{
			auto worker = std::make_shared<worker_t>(workers_, thread_num, enable_steal);
			workers_->push_back(worker);
		}
	}

	~thread_pool()
	{
		for (auto worker : *workers_)
		{
			worker->join();
		}

		//workers_->clear();
	}

	void add_task(task_t&& task)
	{
		auto worker = get_rand_worker();
		worker->assign(std::forward<task_t>(task));
	}

private:
	std::shared_ptr<worker_t> get_rand_worker()
	{
		int i = rand() % workers_->size();
		return workers_->at(i);
	}

	workers_ptr workers_;
};
