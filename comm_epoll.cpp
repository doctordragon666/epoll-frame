#include "comm_epoll.h"

CommEpoll::CommEpoll(fde*& fd_table)
{
	m_fde_table = fd_table;
	epoll_fds = 0;
	printf("comm_epoll, fd_table%p init\n",m_fde_table);
}

CommEpoll::~CommEpoll()
{
}

const char* CommEpoll::epolltype_atoi(int x)
{
	switch (x)
	{
	case EPOLL_CTL_ADD:
		return "EPOLL_CTL_ADD";

	case EPOLL_CTL_DEL:
		return "EPOLL_CTL_DEL";

	case EPOLL_CTL_MOD:
		return "EPOLL_CTL_MOD";

	default:
		return "UNKNOWN_EPOLLCTL_OP";
	}
}

void CommEpoll::comm_call_handlers(int fd, int read_event, int write_event)
{
	if (fd < 0)
		fprintf(stderr, "file is error");

	fprintf(stderr, "\n%p\n", m_fde_table[fd].read_handler);
	fde F = m_fde_table[fd];

	//DEBUG(5, 8) ("comm_call_handlers(): got fd=%d read_event=%x write_event=%x F->read_handler=%p F->write_handler=%p\n", fd, read_event, write_event, F->read_handler, F->write_handler);

	if (F.read_handler == nullptr)
		fprintf(stderr, "error!!!!!!");

	if ((read_event)) {
		PF* hdl = F.read_handler;
		//void* hdl_data = F->read_data;
		/*�����������Ҫ�ӳٵģ���Ҫ���� */

		DEBUG(5) ("comm_call_handlers(): Calling read handler on fd=%d\n", fd);
		hdl(fd, F.read_data);
	}

	if (F.write_handler && (write_event)) {

		PF* hdl = F.write_handler;
		void* hdl_data = F.write_data;

		hdl(fd, hdl_data);
	}
}

void CommEpoll::do_epoll_init(int max_fd)
{
	epoll_instance = epoll_create(max_fd);
	if (epoll_instance < 0)
		fprintf(stderr, "do_epoll_init: epoll_create(): %s\n", xstrerror());
	epoll_state = (unsigned int*)calloc(max_fd, sizeof(unsigned int));
	if(epoll_state == nullptr)
		fprintf(stderr, "do_epoll_init: epoll_create(): %s\n", xstrerror());
	if (Debug) fprintf(stderr, "do_epoll_init: create success.\nepoll_instance:%d, epoll_state:%p\n", epoll_instance, epoll_state);
}

void CommEpoll::do_epoll_shutdown()
{
	close(epoll_instance);
	epoll_instance = -1;
	safe_free(epoll_state);
}

int CommEpoll::do_epoll_select(int msec)
{
	DEBUG(3) ("do_epoll_select...\n");

	int num = epoll_wait(epoll_instance, events, MAX_EVENTS, msec);
	if (num < 0)
	{
		getCurrentTime();
		if (ignoreErrno(errno))
			return COMM_OK;

		DEBUG(5)("comm_select: epoll failure: %s\n", xstrerror());
		return COMM_ERROR;
	}

	if (num == 0)
	{
		DEBUG(5)("select time out or no connect\n");
		return COMM_TIMEOUT;
	}
		

	if (!events)
	{
		printf("event has not assign");
	}

	//fprintf(stderr, "\n%p\n", m_fde_table[3].read_handler);

	int i;
	int fd;
	struct epoll_event* tmp_events;
	for (i = 0, tmp_events = events; i < num; i++, tmp_events++)
	{
		fd = tmp_events->data.fd;
		comm_call_handlers(fd, tmp_events->events & EPOLLOUT, tmp_events->events & EPOLLIN);
	}

	return COMM_OK;
}

void CommEpoll::epollSetEvents(int fd, int need_read, int need_write)
{
	int epoll_ctl_type = 0;
	struct epoll_event ev;

	assert(fd >= 0);
	DEBUG(5)("commSetEvents(fd=%d)\n", fd);

	memset(&ev, 0, sizeof(ev));

	ev.events = 0;
	ev.data.fd = fd;

	if (need_read)
		ev.events |= EPOLLIN;

	if (need_write)
		ev.events |= EPOLLOUT;

	if (ev.events)
		ev.events |= EPOLLHUP | EPOLLERR;

	if (ev.events != epoll_state[fd])
	{
		/* ���epoll���¼��Ѿ�������״̬ */
		if (!ev.events)
		{
			epoll_ctl_type = EPOLL_CTL_DEL;
		}
		else if (epoll_state[fd])
		{
			epoll_ctl_type = EPOLL_CTL_MOD;
		}//������ԭ�����иı�
		else
		{
			epoll_ctl_type = EPOLL_CTL_ADD;
		}

		/* Update the state */
		epoll_state[fd] = ev.events;

		if (epoll_ctl(epoll_instance, epoll_ctl_type, fd, &ev) < 0)
		{
			DEBUG(5)("commSetEvents: epoll_ctl(%s): failed on fd=%d: %s\n",
				epolltype_atoi(epoll_ctl_type), fd, xstrerror());
			return;
		}	
		switch (epoll_ctl_type)
		{
		case EPOLL_CTL_ADD:
			epoll_fds++;
			break;
		case EPOLL_CTL_DEL:
			epoll_fds--;
			break;
		default:
			break;
		}
		DEBUG(5)("commSetEvents: epoll_ctl%d has %d event \n", epoll_instance, epoll_fds);
	}
}