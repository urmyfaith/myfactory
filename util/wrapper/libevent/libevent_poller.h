#ifndef __LIBEVENT_POLLER_H__
#define __LIBEVENT_POLLER_H__

typedef void (*libevent_poller_cb)(int fd, short events, void *arg);

int libevent_poller_network_init(void);

void libevent_poller_network_uninit(void);

int libevent_poller_alloc(void **handle);

int libevent_poller_add_event(void *handle, int sock, libevent_poller_cb cb, void* usrdata);

int libevent_poller_del_event(void *handle, int sock);

int libevent_poller_free(void *handle);

#endif