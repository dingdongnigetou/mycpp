#ifndef MYCPP_NO_COPY_ABLE_H
#define MYCPP_NO_COPY_ABLE_H

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

#endif  // MYCPP_NO_COPY_ABLE_H

