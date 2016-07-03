#include <atomic>
#include <string>
#include <memory>
#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <exception>
using namespace std;

template <typename T>
struct Node
{
	T Data;
	Node* Next = nullptr;
	
	Node(const T& dat) : Data(dat) {}
};


template <typename T>
class Stack
{
	std::atomic<Node<T>*> head;

public:
	void IsLockFree()
	{
		cout << "Is lock free: " << head.is_lock_free() << endl;
	}
	void Push(const T& data)
	{
		auto new_node = new Node<T>(data);

		new_node->Next = head.load(std::memory_order_relaxed);

		while(!head.compare_exchange_weak(new_node->Next, new_node, std::memory_order_release,
			std::memory_order_relaxed));
	}

	T Pop()
	{
		auto result = head.load(std::memory_order_relaxed);
		while (result != nullptr && !head.compare_exchange_weak(result, result->Next,
			std::memory_order_release, std::memory_order_relaxed));
		
		if (nullptr != result) {
			auto res = result->Data;
			// ����ò��ü�����ʽ���ͷ�
			//delete result;
			//result = nullptr;
			return res;
		}
		else {
			return T();
		}
	}
};

template <typename T>
class Stack_M
{
	Node<T>* head = nullptr;
	std::mutex mtx_;

public:
	void IsLockFree()
	{
		cout << "Is lock free: " << 0 << endl;
	}
	void Push(const T& data)
	{
		auto new_node = new Node<T>(data);

		std::unique_lock<std::mutex> l(mtx_);
		new_node->Next = head;
		head = new_node;
	}

	T Pop()
	{
		std::unique_lock<std::mutex> l(mtx_);
		auto result = head;
		if (nullptr != result) {
			head = head->Next;
			auto data = result->Data;
			delete result;
			result = nullptr;
			return data;
		}
		else {
			return T();
		}

	}
};

//Stack<std::string> g_stack;
Stack_M<std::string> g_stack;

int main()
{
	auto begin = std::chrono::high_resolution_clock::now(); 
		
	for (int i = 0; i < 4; ++i) {
		std::thread t([]{ 
				int k = 10000;
				while(k--){
					g_stack.Push("hello" + std::to_string(k));
				}
			 });
		t.detach();
	}

	for (int i = 0; i < 4; ++i) {
		std::thread t([]{ 
				int k = 10000;
				while(k--){
					auto res = g_stack.Pop();
					//std::cout << res << std::endl;
				}
			 });
		t.detach();
	}

	auto end = chrono::high_resolution_clock::now(); 
	cout << "cost : " << chrono::duration_cast<chrono::microseconds>(end-begin).count() << endl;
	g_stack.IsLockFree();

	getchar();

	return 0;
}


