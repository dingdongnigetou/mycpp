#ifndef __SINGLETON_HPP__
#define __SINGLETON_HPP__

#include <atomic>
#include <memory>

namespace mycpp 
{
	template <class TYPE>
	class Singleton
	{
	private:
		static TYPE* s_singleton;
		static std::atomic_flag s_instantiated;
		static volatile bool s_instantiating;

	public:
		static TYPE* Instance()
		{
			if (!s_instantiated.test_and_set())
			{
				s_singleton = new TYPE();
				s_instantiating = false;
			}

			while (s_instantiating);

			return s_singleton;
		}

		template <typename ...P>
		static TYPE* Instance(P... args)
		{
			if (!s_instantiated.test_and_set())
			{
				s_singleton = new TYPE(args...);
				s_instantiating = false;
			}

			while (s_instantiating);

			return s_singleton;
		}
	};

	template<typename TYPE>
	TYPE* Singleton<TYPE>::s_singleton ;

	template<typename TYPE>
	std::atomic_flag Singleton<TYPE>::s_instantiated = ATOMIC_FLAG_INIT;

	template<typename TYPE>
	volatile bool Singleton<TYPE>::s_instantiating = true;

}// end common 



#endif// end __SINGLETON_HPP__