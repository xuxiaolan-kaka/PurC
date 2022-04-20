/*
 * @file purc-runloop.h
 * @author XueShuming
 * @date 2021/12/14
 * @brief The C api for RunLoop.
 *
 * Copyright (C) 2021, 2022 FMSoft <https://www.fmsoft.cn>
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

#ifndef PURC_RUNLOOP_H
#define PURC_RUNLOOP_H

#include <stdint.h>

typedef void* purc_runloop_t;
typedef enum purc_runloop_io_event
{
    PCRUNLOOP_IO_IN = 0x01,
    PCRUNLOOP_IO_PRI = 0x02,
    PCRUNLOOP_IO_OUT = 0x04,
    PCRUNLOOP_IO_ERR = 0x08,
    PCRUNLOOP_IO_HUP = 0x10,
    PCRUNLOOP_IO_NVAL = 0x20,
} purc_runloop_io_event;

PCA_EXTERN_C_BEGIN

/**
 * Init main runloop
 *
 * Return: void
 *
 * Since: 0.1.1
 */
PCA_EXPORT
void purc_runloop_init_main(void);

/**
 * Stop main runloop
 *
 * Return: void
 *
 * Since: 0.1.1
 */
PCA_EXPORT
void purc_runloop_stop_main(void);

/**
 * Check if the man runloop is initialized.
 *
 * Returns: The function returns a boolean indicating whether main RunLoop
 *  is initialized.
 *
 * Since: 0.1.1
 */
PCA_EXPORT
bool purc_runloop_is_main_initialized(void);

/**
 * Get the runloop of current thread
 *
 * Returns: The runloop of current thread
 *
 * Since: 0.1.1
 */
PCA_EXPORT
purc_runloop_t purc_runloop_get_current(void);

/**
 * Check if current is on main runloop.
 *
 * Returns: The function returns a boolean indicating whether current is on main
 *  runloop.
 *
 * Since: 0.1.1
 */
PCA_EXPORT
bool purc_runloop_is_on_main(void);

/**
 * Start the current runloop
 *
 * Returns: void
 *
 * Since: 0.1.1
 */
PCA_EXPORT
void purc_runloop_run(void);

/**
 * Stop the runloop
 *
 * @param runloop: the runloop.
 *
 * Returns: void
 *
 * Since: 0.1.1
 */
PCA_EXPORT
void purc_runloop_stop(purc_runloop_t runloop);

/**
 * Warkup the runloop
 *
 * @param runloop: the runloop.
 *
 * Returns: void
 *
 * Since: 0.1.1
 */
PCA_EXPORT
void purc_runloop_wakeup(purc_runloop_t runloop);

typedef int (*purc_runloop_func)(void *ctxt);

/**
 * Dispatch function on the runloop
 *
 * @param runloop: the runloop.
 * @param func: the function to dispatch.
 * @param ctxt: the data to pass to the function
 *
 * Returns: void
 *
 * Since: 0.1.1
 */
PCA_EXPORT
void purc_runloop_dispatch(purc_runloop_t runloop, purc_runloop_func func,
        void *ctxt);

/**
 * Set the idle function on the runloop which will be called on idle.
 *
 * @param runloop: the runloop.
 * @param func: the function.
 * @param ctxt: the data to pass to the function
 *
 * Returns: void
 *
 * Since: 0.1.1
 */
PCA_EXPORT
void purc_runloop_set_idle_func(purc_runloop_t runloop, purc_runloop_func func,
        void *ctxt);

typedef bool (*purc_runloop_io_callback)(int fd,
        purc_runloop_io_event event, void *ctxt, void *stack);

/**
 * Add file descriptors monitor on the runloop
 *
 * @param runloop: the runloop
 * @param fd: the file descriptors
 * @param event: the io event
 * @param callback: the callback function
 * @param ctxt: the data to pass to the function
 *
 * Returns: the handle of the monitor
 *
 * Since: 0.1.1
 */
PCA_EXPORT
uintptr_t purc_runloop_add_fd_monitor(purc_runloop_t runloop, int fd,
        purc_runloop_io_event event, purc_runloop_io_callback callback,
        void *ctxt);

/**
 * Remove file descriptors monitor on the runloop
 *
 * @param runloop: the runloop
 * @param handle: the monitor handle
 *
 * Returns: void
 *
 * Since: 0.1.1
 */
PCA_EXPORT
void purc_runloop_remove_fd_monitor(purc_runloop_t runloop, uintptr_t handle);

/**
 * Dispatch message
 *
 * @param runloop: the runloop
 * @param source: the source of the message
 * @param type: the type of the message
 * @param sub_type: the sub type of the message
 * @param extra: the extra of the message
 * @param stack: the pointer to the stack
 *
 * Return: denote if the function succeeds or not
 *         0:  Success
 *         -1: Failed
 *
 * Since: 0.1.1
 */
PCA_EXPORT
int purc_runloop_dispatch_message(purc_runloop_t runloop, purc_variant_t source,
        purc_variant_t type, purc_variant_t sub_type, purc_variant_t extra,
        void  *stack);

PCA_EXTERN_C_END

#endif /* not defined PURC_RUNLOOP_H */
