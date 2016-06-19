#ifndef __MYCPP_THREAD_POOL_HPP__
#define __MYCPP_THREAD_POOL_HPP__

#include <vector>
#include <list>
#include <memory>

#include "Thread.hpp"
#include "../Time/DateTime.hpp"

namespace mycpp
{
	class ThreadPool
	{
	public:
		/*
		minCapacity �̳߳ؿ���ʱ��С�������߳���
		maxCapacity �̳߳���������߳���
		idleTime �߳̿��ж�ú�����٣� ��λ �� , <1 ��ʾ���˳�
		*/
		ThreadPool(int minCapacity = 2, int maxCapacity = 16, int idleTime = 60);
		~ThreadPool();

		//���ӻ�����߳������Ƿ���Ҫ��
		void AddCapacity(int n);
		//�����̳߳ص�ǰ�߳���
		int GetCapacity() const;
		//ֹͣ�̳߳�
		void StopAll(); 
		//�ȴ������߳̽���
		void JoinAll();

		template<typename Fun, typename... Args>
		bool Start(Fun&& fun, Args&&... args);

		bool IsSelfThread(long threaID);

		//���صȴ����ȵ��߳���
		int GetWaits();
	private:
		void threadEntry(Thread& thread);
		void addThread();

	private:
		std::list<std::function<void()>> tasks_;

		int minCapacity_ = 0;
		int maxCapacity_ = 0;
		int nCapacity_ = 0;
		int idleTime_ = 0; //��λ��
		MyMutex mutex_;
		MyCond cond_;
		std::vector<std::shared_ptr<Thread>> threads_;
		bool isrun_ = true;
		int idles_ = 0; //�����߳�����
	};
} // end namespace 

#include "impl/ThreadPool.ipp"

#endif // !__MYCPP_THREAD_POOL_HPP__
