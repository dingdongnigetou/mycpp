#ifndef __SINGLETON_C11_HPP__
#define __SINGLETON_C11_HPP__

// c++11�±�׼����ľֲ���̬������֤���̰߳�ȫ
// �����õ��̻߳���c++�±�׼�ĳ���

namespace mycpp
{
	template <typename T>
	class Sigleton
	{
	public:
		static T& Instance()
		{
			static T obj;
			return obj;
		}

		template <typename ...P>
		static T& Instance(P... args)
		{
			static T obj(args...);
			return obj;
		}

	};
}

#endif 