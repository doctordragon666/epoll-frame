#ifndef _COMM_EPOLL_H_
#define _COMM_EPOLL_H_

#include <sys/epoll.h>
/* epoll��Ҫ��ȫ�ֱ��� */
#include "global.h"
/* epollһ�δ����������¼� */

class CommEpoll
{
public:
	CommEpoll(fde*& fd_table);
	~CommEpoll();
	int epoll_instance;//epollʵ�����
	struct epoll_event events[MAX_EVENTS];//���epoll�¼�������
	int epoll_fds;
	unsigned int* epoll_state; /* ����ÿ��epoll ���¼�״̬ */

	static const char* epolltype_atoi(int x);

	/// <summary>
	/// ����fd��������
	/// </summary>
	/// <param name="fd">��Ӧ��socket������</param>
	/// <param name="read_event">����־����Ҫִ�ж��¼�ʱ��Ϊ1</param>
	/// <param name="write_event">д��־����Ҫִ��д�¼�ʱ��Ϊ1</param>
	void comm_call_handlers(int fd, int read_event, int write_event);

	/// <summary>
	/// epoll��ʼ�������ҳ�ʼ��һ��epoll״̬
	/// </summary>
	/// <param name="max_fd">�������</param>
	void do_epoll_init(int max_fd);

	/// <summary>
	/// �ر�epoll���ӣ��ͷ�����״̬
	/// </summary>
	void do_epoll_shutdown();

	/// <summary>
	/// �ȴ�epoll�¼����ֲ��Ҵ���
	/// </summary>
	/// <param name="msec">��ʱֵ</param>
	/// <returns>COMM���͵ĵ�����Ϣ</returns>
	int do_epoll_select(int msec);

	/// <summary>
	/// epoll�����¼�
	/// </summary>
	/// <param name="fd">�ļ�������</param>
	/// <param name="need_read">�Ƿ���Ҫ��ȡ</param>
	/// <param name="need_write">�Ƿ���Ҫд��</param>
	void epollSetEvents(int fd, int need_read, int need_write);
private:
	fde* m_fde_table;
};

#endif // !_COMM_EPOLL_H_