/*
 * Copyright (c) 2010, NanChang BangTeng Inc
 * All Rights Reserved.
 *
 * You may use the Software for free for non-commercial use
 * under the License Restrictions.
 *
 * You may modify the source code(if being provieded) or interface
 * of the Software under the License Restrictions.
 *
 * You may use the Software for commercial use after purchasing the
 * commercial license.Moreover, according to the license you purchased
 * you may get specified term, manner and content of technical
 * support from NanChang BangTeng Inc
 *
 * See COPYING file for detail.
 */
#ifndef KEPOLLSELECTOR_H_
#define KEPOLLSELECTOR_H_
#include "global.h"
#ifdef HAVE_SYS_EPOLL_H
#include <sys/epoll.h>
//#define ENABLE_ONESHOT_MODEL   1
#include "KSelector.h"
#include "malloc_debug.h"

class KEpollSelector : public KSelector
{
public:
	const char *getName()
	{
#ifdef ENABLE_ONESHOT_MODEL
		return "epoll(one shot)";
#else
		return "epoll";
#endif
	}
	KEpollSelector();
	virtual ~KEpollSelector();
	void select();
	bool listen(KServer *st,resultEvent result);
	bool read(KSelectable *st,resultEvent result,bufferEvent buffer,void *arg);
        bool write(KSelectable *st,resultEvent result,bufferEvent buffer,void *arg);
        bool next(KSelectable *st,resultEvent result,void *arg);
	bool connect(KSelectable *st,resultEvent result,void *arg);
protected:
	bool addSocket(KSelectable *rq,int op);
	void removeSocket(KSelectable *rq);
private:
	int kdpfd;

};
#endif
#endif /*KEPOLLSELECTOR_H_*/
