#ifndef __SINGLETON_C11_HPP__
#define __SINGLETON_C11_HPP__

// c++11新标准提出的局部静态变量保证多线程安全
// 仅适用单线程或者c++新标准的程序

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