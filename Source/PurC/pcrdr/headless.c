/*
 * purcmc.c -- The implementation of HEADLESS protocol.
 *
 * Copyright (c) 2022 FMSoft (http://www.fmsoft.cn)
 *
 * Authors:
 *  Vincent Wei (https://github.com/VincentWei), 2022
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

#include "config.h"
#include "purc-pcrdr.h"
#include "private/list.h"
#include "private/debug.h"
#include "private/utils.h"
#include "private/ports.h"

#include "connect.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <assert.h>

#define NR_WORKSPACES           8
#define NR_TABBEDWINDOWS        8
#define NR_TABBEDPAGES          32
#define NR_PLAINWINDOWS         256
#define NR_WINDOWLEVELS         2
#define NAME_WINDOW_LEVEL_0     "normal"
#define NAME_WINDOW_LEVEL_1     "topmost"

#define __STRING(x) #x

#define RENDERER_FEATURES                           \
    "HEADLESS:100\n"                                \
    "HTML:5.3/XGML:1.0/XML:1.0\n"                   \
    "workspace:" __STRING(NR_WORKSPACES)            \
    "/tabbedWindow:" __STRING(NR_TABBEDWINDOWS)     \
    "/tabbedPage:" __STRING(NR_TABBEDPAGES)         \
    "/plainWindow:" __STRING(NR_PLAINWINDOWS)       \
    "/windowLevel:" __STRING(NR_WINDOWLEVELS) "\n"  \
    "windowLevels:" NAME_WINDOW_LEVEL_0 "," NAME_WINDOW_LEVEL_1

struct tabbed_window_info {
    // handle of this tabbedWindow; NULL for not used slot.
    void *handle;

    // number of tabpages in this tabbedWindow
    int nr_tabpages;

    // handles of all tabpages in this tabbedWindow
    void *tabpages[NR_TABBEDPAGES];

    // handles of all DOM documents in all tabpages.
    void *domdocs[NR_TABBEDPAGES];
};

struct workspace_info {
    // handle of this workspace; NULL for not used slot
    void *handle;

    // number of tabbed windows in this workspace
    int nr_tabbed_windows;

    // number of plain windows in this workspace
    int nr_plain_windows;

    // information of all tabbed windows in this workspace.
    struct tabbed_window_info tabbed_windows[NR_TABBEDWINDOWS];

    // handles of all plain windows in this workspace.
    void *plain_windows[NR_PLAINWINDOWS];

    // handles of DOM documents in all plain windows.
    void *domdocs[NR_PLAINWINDOWS];
};

enum {
    PCRDR_HEADLESS_STATE_INITIAL = 0,
    PCRDR_HEADLESS_STATE_STARTED,   // session started
};

struct pcrdr_prot_data {
    // FILE pointer to serialize the message.
    FILE *fp;

    // current state
    int state;

    // number of workspaces
    int nr_workspaces;

    // workspaces
    struct workspace_info workspaces[NR_WORKSPACES];
};

static int my_wait_message(pcrdr_conn* conn, int timeout_ms)
{
    if (list_empty(&conn->pending_requests)) {
        if (timeout_ms > 1000) {
            pcutils_sleep(timeout_ms / 1000);
        }

        if (timeout_ms > 0) {
            unsigned int ms = timeout_ms % 1000;
            if (ms) {
                pcutils_usleep(ms * 1000);
            }
        }

        return 0;
    }

    // it's time to read a fake response message.
    return 1;
}

static pcrdr_msg *my_read_message(pcrdr_conn* conn)
{
    pcrdr_msg* msg = NULL;
    UNUSED_PARAM(conn);

    return msg;
}

static ssize_t write_to_log(void *ctxt, const void *buf, size_t count)
{
    FILE *fp = (FILE *)ctxt;

    if (fwrite(buf, 1, count, fp) == count)
        return 0;

    return -1;
}

static int my_send_message(pcrdr_conn* conn, pcrdr_msg *msg)
{
    if (pcrdr_serialize_message(msg,
                (cb_write)write_to_log, conn->prot_data->fp) < 0) {
        goto failed;
    }

    fputs("", conn->prot_data->fp);
    return 0;

failed:
    return -1;
}

static int my_ping_peer(pcrdr_conn* conn)
{
    UNUSED_PARAM(conn);
    return 0;
}

static int my_disconnect(pcrdr_conn* conn)
{
    fclose(conn->prot_data->fp);
    free(conn->prot_data);
    return 0;
}

/* returns 0 if all OK, -1 on error */
int pcrdr_headless_connect(const char* app_name, const char* runner_name,
        pcrdr_conn** conn)
{
    int err_code = PCRDR_ERROR_NOMEM;

    *conn = NULL;
    if (!purc_is_valid_app_name(app_name) ||
            !purc_is_valid_runner_name(runner_name)) {
        err_code = PURC_EXCEPT_INVALID_VALUE;
        goto failed;
    }

    if ((*conn = calloc(1, sizeof(pcrdr_conn))) == NULL) {
        PC_DEBUG ("Failed to allocate space for connection: %s\n",
                strerror (errno));
        err_code = PCRDR_ERROR_NOMEM;
        goto failed;
    }

    if (((*conn)->prot_data =
                calloc(1, sizeof(struct pcrdr_prot_data))) == NULL) {
        PC_DEBUG ("Failed to allocate space for protocol data: %s\n",
                strerror (errno));
        err_code = PCRDR_ERROR_NOMEM;
        goto failed;
    }

    // TODO: open log file here.

    (*conn)->prot = PURC_RDRPROT_HEADLESS;
    (*conn)->type = CT_PLAIN_FILE;
    (*conn)->fd = -1;
    (*conn)->srv_host_name = NULL;
    (*conn)->own_host_name = strdup(PCRDR_LOCALHOST);
    (*conn)->app_name = app_name;
    (*conn)->runner_name = runner_name;

    (*conn)->wait_message = my_wait_message;
    (*conn)->read_message = my_read_message;
    (*conn)->send_message = my_send_message;
    (*conn)->ping_peer = my_ping_peer;
    (*conn)->disconnect = my_disconnect;

    list_head_init (&(*conn)->pending_requests);
    return 0;

failed:
    if (*conn) {
        if ((*conn)->prot_data)
           free((*conn)->prot_data);
        if ((*conn)->own_host_name)
           free((*conn)->own_host_name);
        free(*conn);
        *conn = NULL;
    }

    purc_set_error(err_code);
    return -1;
}

