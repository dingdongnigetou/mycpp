#ifndef __MYCPP_NO_COPY_ABLE_H__
#define __MYCPP_NO_COPY_ABLE_H__

namespace mycpp
{
	class noncopyable
	{
	protected:
		noncopyable() {}
		~noncopyable() {}
	private:  // emphasize the following members are private
		noncopyable(const noncopyable&);
		const noncopyable& operator=(const noncopyable&);
	};

}// end namespace mycpp 

#endif  // __MYCPP_NO_COPY_ABLE_H__

