/**
 * @file net/sock.c  Networking sockets code
 *
 * Copyright (C) 2010 Creytiv.com
 */
#include <stdio.h>
#include <unistd.h>
#include <thread>
#include <map>

#include<event.h>
#include<event2/util.h>

static bool inited = false;

struct libevent_poller_tag_t
{
	struct event_base* base;
	std::thread threadeventloop;

	std::map<int, struct event*> map_events;

	int is_running;
};
///////////////////////////////////////////////////////////////
#ifdef WIN32
static int wsa_init(void)
{
	WORD wVersionRequested = MAKEWORD(2, 2);
	WSADATA wsaData;
	int err;

	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0) {
		printf("Could not load winsock (%m)\n", err);
		return err;
	}

	/* Confirm that the WinSock DLL supports 2.2.*/
	/* Note that if the DLL supports versions greater    */
	/* than 2.2 in addition to 2.2, it will still return */
	/* 2.2 in wVersion since that is the version we      */
	/* requested.                                        */
	if (LOBYTE(wsaData.wVersion) != 2 ||
	    HIBYTE(wsaData.wVersion) != 2 ) {
		WSACleanup();
		printf("Bad winsock verion (%d.%d)\n",
			      HIBYTE(wsaData.wVersion),
			      LOBYTE(wsaData.wVersion));
		return EINVAL;
	}

	return 0;
}
#endif

int event_threadproc(void *arg)
{
	libevent_poller_tag_t *inst = (libevent_poller_tag_t*)arg;

	while( inst->is_running )
	{
		if( event_base_got_exit(inst->base) )
			break;

		usleep(1);
		
//		event_base_loop(inst->base, EVLOOP_NONBLOCK|EVLOOP_ONCE);
		event_base_loop(inst->base, 0);
//	    event_base_dispatch(inst->base);
	}

	return 0;
}
///////////////////////////////////////////////////////////////////
/**
 * Initialise network sockets
 *
 * @return 0 if success, otherwise errorcode
 */
int libevent_poller_network_init(void)
{
	int err = 0;

	if (inited)
		return 0;

#ifdef WIN32
	err = wsa_init();
#endif

	inited = true;

	return err;
}

/**
 * Cleanup network sockets
 */
void libevent_poller_network_uninit(void)
{
#ifdef WIN32
	const int err = WSACleanup();
	if (0 != err) {
		printf("sock close: WSACleanup (%d)\n", err);
	}
#endif

	inited = false;
}

int libevent_poller_alloc(void **handle)
{
    struct event_base* base = event_base_new();
    if( !base )
    {
    	printf("event_base_new error \n");
    	return -1;
    }

	libevent_poller_tag_t *inst = new (std::nothrow) libevent_poller_tag_t;
	if( !inst )
		return -1;

	inst->base = base;
	inst->is_running = 1;

    inst->threadeventloop = std::thread(event_threadproc, (void*)inst);

//    evutil_make_listen_socket_reuseable(listener);

	return 0;
}

int libevent_poller_add_event(void *handle, int sock, libevent_poller_cb cb, void* usrdata)
{
	libevent_poller_tag_t *inst = (libevent_poller_tag_t*)handle;
	
	evutil_make_socket_nonblocking(sock);
	
	struct event* ev = event_new(inst->base, sock, EV_READ | EV_PERSIST,
                                        cb, usrdata);

//	event_base_set(inst->base, ev);
    event_add(ev, NULL);

    inst->map_events[sock] = ev;

	return 0;
}

int libevent_poller_del_event(void *handle, int sock)
{
	libevent_poller_tag_t *inst = (libevent_poller_tag_t*)handle;
	if( inst->map_events.empty() )
		return -1;

	auto iterev = inst->map_events.find(sock);
	if( iterev == inst->map_events.end() )
		return -1;

	event_del(iterev->second)
	event_free(iter->second);
	inst->map_events.erase(iterev);

	return 0;	
}

int libevent_poller_free(void *handle)
{
	libevent_poller_tag_t *inst = (libevent_poller_tag_t*)handle;

	inst->is_running = 0;
	event_base_loopexit();

	inst->threadeventloop.join();
	
	for( auto iter = inst->map_events.begin();
		iter != inst->map_events.end();
		++iter)
	{
		event_del(iter->second)
		event_free(iter->second);
	}
	iinst->map_events.clear();

	event_base_free(inst->base);
	
	delete inst;

	return 0;
}
