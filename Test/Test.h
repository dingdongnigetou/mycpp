#ifndef MYCPP_TEST
#define MYCPP_TEST

// 在vs2105库下  
// C++标准库mutex与临界区的性能比较，thread与Windows Api的比较
// 测试结果表明，标准库的性能并不亚于windows api, 当然，这是在
// Release的情况下，而在Debug的情况下则剧情反转，临界区秒杀mutex
// 这可能是由于vs在Release做了某些优化导致。

// 而在vs2013库下
// 结果却是相反， 临界区的性能相比标准库的mutex有很大的提升,无论是debug还是release

// 所以一般推荐使用临界区，在不跨进程锁的情况下。
void use_std_mutex();
void use_win_critical();
void use_win_thread();

#endif // MYCPP_TEST
