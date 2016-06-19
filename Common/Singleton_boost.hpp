//
// Singletonģ��
// �ο� boost 1-36-0/pool/detail/singleton.hpp
// ˵�� http://www.yulefox.com/index.php/20081119/design-patterns-03.html/
//

#ifndef __SINGLETON_BOOST_HPP__
#define __SINGLETON_BOOST_HPP__

// �����ྲ̬��Ա����create_object,������̶߳�γ�ʼ������
// ��create_object���þֲ������ʼ��������໥���ö������ĶԷ�����δ��ʼ��������

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
