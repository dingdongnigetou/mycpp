//
// Singleton模板
// 参考 boost 1-36-0/pool/detail/singleton.hpp
// 说明 http://www.yulefox.com/index.php/20081119/design-patterns-03.html/
//

#ifndef __SINGLETON_BOOST_HPP__
#define __SINGLETON_BOOST_HPP__

// 构造类静态成员变量create_object,解决多线程多次初始化问题
// 在create_object调用局部对象初始化解决类相互调用而产生的对方对象未初始化的问题

namespace mycpp
{
	template <typename T>
	class singleton_default
	{
	public:
		typedef T object_type;
		static object_type& instance()
		{
			static object_type obj;
			create_object.do_nothing();
			return obj;
		}

	private:
		struct object_creator
		{
			object_creator() { singleton_default<T>::instance(); }
			inline void do_nothing() const {}
		};
		static object_creator create_object;
		singleton_default();
	};

	template <typename T>
	typename singleton_default<T>::object_creator
		singleton_default<T>::create_object;

} // end mycpp

#define INSTANCE(class_name) mycpp::singleton_default<class_name>::instance()

#endif 
