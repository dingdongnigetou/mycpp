#ifndef __MYCPP_ENCODING_HPP__
#define __MYCPP_ENCODING_HPP__

#include "mydefs.h"
#include "Singleton.hpp"

namespace mycpp
{
	class Encoding : public Singleton<Encoding>
	{
		friend class Singleton<Encoding>;

	public:
#if defined(_MSWINDOWS_)
#endif

	private:
#if defined(_MSWINDOWS_)

#endif

	};
}

#define Encoding() mycpp::Encoding::instance()

#endif // ! __MYCPP_ENCODING_HPP__
