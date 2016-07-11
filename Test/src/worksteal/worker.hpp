#pragma once
#include <thread>
#include <memory>
#include <vector>
#include <functional>
#include <algorithm>
#include "sync_deque.hpp"

class worker_t;
using workers_ptr = std::shared_ptr<std::vector<std::shared_ptr<worker_t>>>;
using task_t = std::function<void()>;

class worker_t final
{
public:
	
	worker_t(workers_ptr workers, int work_num, bool enable_steal = true) : 
		workers_(workers), work_num_(work_num), enable_(true), enable_steal_(enable_steal)
	{
		thread_ = std::thread(&worker_t::execute, this);
	}

	~worker_t()
	{
	}

	void assign(task_t&& task)
	{
		queue_.push_front(std::forward<task_t>(task));
	}

	task_t steal()
	{
		return queue_.pop_back();
	}

	bool empty()
	{
		return queue_.empty();
	}

	void join()
	{
		enable_ = false;
		thread_.join();
	}

	const std::thread::id& get_id()
	{
		return thread_id_;
	}

private:
	void execute()
	{
		thread_id_ = std::this_thread::get_id();
		while (enable_ || !queue_.empty())
		{
			//not ready
			if (work_num_ != workers_->size())
			{
				std::this_thread::sleep_for(std::chrono::microseconds(10));
				continue;
			}
			
			//do work
			task_t t = queue_.pop_front();
			while (t!=nullptr)
			{
				t();
				t = queue_.pop_front();
			}
			//steal
			// 这里可以增加非本线程的判断，防止偷自己的任务
			if (enable_steal_) {

				bool no_task = std::all_of(workers_->begin(), workers_->end(), [](std::shared_ptr<worker_t> worker) {return worker->empty(); });
				if (no_task)
				{
					//std::cout<<"all tasks have been finished!"<<std::endl;
					std::this_thread::sleep_for(std::chrono::microseconds(50));
					continue;
				}

				int rand_select = rand() % workers_->size();
				if (workers_->at(rand_select)->get_id() == thread_id_ || workers_->at(rand_select) == nullptr)
					continue;

				t = workers_->at(rand_select)->steal();

				if (t != nullptr)
				{
					t();
				}
			}
		}
	}
	
	std::thread thread_;
	std::shared_ptr<std::vector<std::shared_ptr<worker_t>>> workers_;
	std::thread::id thread_id_;
	sync_deque<task_t> queue_;

	int work_num_;
	volatile bool enable_;
	bool enable_steal_;
};

