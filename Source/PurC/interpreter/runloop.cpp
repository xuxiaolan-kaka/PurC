/*
 * @file runloop.cpp
 * @author XueShuming
 * @date 2021/12/14
 * @brief The C api for RunLoop.
 *
 * Copyright (C) 2021 FMSoft <https://www.fmsoft.cn>
 *
 * This file is a part of PurC (short for Purring Cat), an HVML interpreter.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "purc.h"

#include "config.h"

#include "private/runloop.h"
#include "private/errors.h"

#include <wtf/Threading.h>
#include <wtf/RunLoop.h>
#include <wtf/threads/BinarySemaphore.h>


#include <stdlib.h>
#include <string.h>


#define MAIN_RUNLOOP_THREAD_NAME    "__purc_main_runloop_thread"

void pcrunloop_init_main(void)
{
    if (pcrunloop_is_main_initialized()) {
        return;
    }
    BinarySemaphore semaphore;
    Thread::create(MAIN_RUNLOOP_THREAD_NAME, [&] {
        RunLoop::initializeMain();
        RunLoop& runloop = RunLoop::main();
        semaphore.signal();
        runloop.run();
    })->detach();
    semaphore.wait();
}

void pcrunloop_stop_main(void)
{
    if (pcrunloop_is_main_initialized()) {
        BinarySemaphore semaphore;
        RunLoop& runloop = RunLoop::main();
        runloop.dispatch([&] {
            RunLoop::stopMain();
            semaphore.signal();
        });
        semaphore.wait();
    }
}

bool pcrunloop_is_main_initialized(void)
{
    return RunLoop::isMainInitizlized();
}

pcrunloop_t pcrunloop_get_current(void)
{
    return (pcrunloop_t)&RunLoop::current();
}

bool pcrunloop_is_on_main(void)
{
    return RunLoop::isMain();
}

void pcrunloop_run(void)
{
    RunLoop::run();
}

void pcrunloop_stop(pcrunloop_t runloop)
{
    if (runloop) {
        ((RunLoop*)runloop)->stop();
    }
}

void pcrunloop_wakeup(pcrunloop_t runloop)
{
    if (runloop) {
        ((RunLoop*)runloop)->wakeUp();
    }
}

void pcrunloop_dispatch(pcrunloop_t runloop, pcrunloop_func func, void* ctxt)
{
    if (runloop) {
        ((RunLoop*)runloop)->dispatch([func, ctxt]() {
            func(ctxt);
        });
    }
}

void pcrunloop_set_idle_func(pcrunloop_t runloop, pcrunloop_func func, void* ctxt)
{
    if (runloop) {
        ((RunLoop*)runloop)->setIdleCallback([func, ctxt]() {
            func(ctxt);
        });
    }
}

static enum pcrunloop_io_condition
to_io_condition(GIOCondition condition)
{
    switch (condition) {
    case G_IO_IN:
        return PCRUNLOOP_IO_IN;
    case G_IO_OUT:
        return PCRUNLOOP_IO_OUT;
    case G_IO_PRI:
        return PCRUNLOOP_IO_PRI;
    case G_IO_ERR:
        return PCRUNLOOP_IO_ERR;
    case G_IO_HUP:
        return PCRUNLOOP_IO_HUP;
    case G_IO_NVAL:
        return PCRUNLOOP_IO_NVAL;
    }
}

static GIOCondition
to_gio_condition(enum pcrunloop_io_condition condition)
{
    switch (condition) {
    case PCRUNLOOP_IO_IN:
        return G_IO_IN;
    case PCRUNLOOP_IO_OUT:
        return G_IO_OUT;
    case PCRUNLOOP_IO_PRI:
        return G_IO_PRI;
    case PCRUNLOOP_IO_ERR:
        return G_IO_ERR;
    case PCRUNLOOP_IO_HUP:
        return G_IO_HUP;
    case PCRUNLOOP_IO_NVAL:
        return G_IO_NVAL;
    }
}


uintptr_t pcrunloop_add_fd_monitor(pcrunloop_t runloop, int fd,
        enum pcrunloop_io_condition condition, pcrunloop_io_callback callback,
        void *ctxt)
{
    if (!runloop) {
        runloop = pcrunloop_get_current();
    }
    return ((RunLoop*)runloop)->addFdMonitor(fd, to_gio_condition(condition),
            [callback, ctxt] (gint fd, GIOCondition condition) -> gboolean {
            return callback(fd, to_io_condition(condition), ctxt);
        });
}

void pcrunloop_remove_fd_monitor(pcrunloop_t runloop, uintptr_t handle)
{
    if (!runloop) {
        runloop = pcrunloop_get_current();
    }
    ((RunLoop*)runloop)->removeFdMonitor(handle);
}

