#ifndef __MYCPP_META_HPP__
#define __MYCPP_META_HPP__

#include <string>
#include <tuple>
#include <array>

template<unsigned N>
std::array<std::string, N> split(const std::string& s, const char delimiter)
{
    size_t start = 0;
    size_t end = s.find_first_of(delimiter);

    std::array<std::string, N> output;

    size_t i = 0;
    while (end <= std::string::npos)
    {
        output[i++] = std::move(s.substr(start, end - start));
        if (end == std::string::npos)
            break;

        start = end + 2;
        end = s.find_first_of(delimiter, start);
    }

    return output;
}

template<unsigned N, typename T>
constexpr static inline auto make(const std::array<std::string, N>&ar, unsigned index, T& args)
{
    return args;
}

template<unsigned N, typename T, typename T1, typename... Args>
constexpr static inline auto make(const std::array<std::string, N>&ar, unsigned index, T const & t, T1& first, Args&... args)
{
    return make(ar, index + 1, std::tuple_cat(t, std::make_tuple(std::pair<std::string, T1&>(ar[index], first))), args...);
}

#define VA_ARGS_NUM(...) std::tuple_size<decltype(std::make_tuple(__VA_ARGS__))>::value

#define META(...) auto meta(){\
    auto ar = split<VA_ARGS_NUM(__VA_ARGS__)>(#__VA_ARGS__, ',');\
    return make(ar, 0, std::tuple<>(), __VA_ARGS__);\
}

#endif // !__MYCPP_META_HPP__
