#ifndef MYCPP_TEST
#define MYCPP_TEST

// ��vs2105����  
// C++��׼��mutex���ٽ��������ܱȽϣ�thread��Windows Api�ıȽ�
// ���Խ����������׼������ܲ�������windows api, ��Ȼ��������
// Release������£�����Debug�����������鷴ת���ٽ�����ɱmutex
// �����������vs��Release����ĳЩ�Ż����¡�

// ����vs2013����
// ���ȴ���෴�� �ٽ�����������ȱ�׼���mutex�кܴ������,������debug����release

// ����һ���Ƽ�ʹ���ٽ������ڲ��������������¡�
void use_std_mutex();
void use_win_critical();
void use_win_thread();

#endif // MYCPP_TEST
