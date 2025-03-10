/*
 * @file fetcher.c
 * @author XueShuming
 * @date 2021/11/16
 * @brief The impl for fetcher api.
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

#include "config.h"

#include "private/fetcher.h"
#include "private/instance.h"

#include "fetcher-internal.h"

#include <wtf/Lock.h>

static Lock s_fetcher_lock;
static struct pcfetcher* s_remote_fetcher = NULL;
static struct pcfetcher* s_local_fetcher = NULL;

static struct pcfetcher* get_fetcher(void)
{
    return s_remote_fetcher ? s_remote_fetcher : s_local_fetcher;
}

bool pcfetcher_is_init(void)
{
    return s_remote_fetcher || s_local_fetcher;
}

const char* pcfetcher_set_base_url(const char* base_url)
{
    struct pcfetcher* fetcher = get_fetcher();
    return fetcher ? fetcher->set_base_url(fetcher, base_url) : NULL;
}

void pcfetcher_cookie_set(const char* domain,
        const char* path, const char* name, const char* content,
        time_t expire_time, bool secure)
{
    struct pcfetcher* fetcher = get_fetcher();
    if (fetcher) {
        fetcher->cookie_set(fetcher, domain, path, name, content,
                expire_time, secure);
    }
}

const char* pcfetcher_cookie_get(const char* domain,
        const char* path, const char* name, time_t *expire, bool *secure)
{
    struct pcfetcher* fetcher = get_fetcher();
    return fetcher ? fetcher->cookie_get(fetcher, domain, path,
            name, expire, secure) : NULL;
}

const char* pcfetcher_cookie_remove(const char* domain,
        const char* path, const char* name)
{
    struct pcfetcher* fetcher = get_fetcher();
    if (fetcher) {
        return fetcher->cookie_remove(fetcher, domain, path, name);
    }
    return NULL;
}

purc_variant_t pcfetcher_request_async(
        const char* url,
        enum pcfetcher_request_method method,
        purc_variant_t params,
        uint32_t timeout,
        pcfetcher_response_handler handler,
        void* ctxt)
{
    struct pcfetcher* fetcher = get_fetcher();
    return fetcher ? fetcher->request_async(fetcher, url, method,
            params, timeout, handler, ctxt) : PURC_VARIANT_INVALID;
}

purc_rwstream_t pcfetcher_request_sync(
        const char* url,
        enum pcfetcher_request_method method,
        purc_variant_t params,
        uint32_t timeout,
        struct pcfetcher_resp_header *resp_header)
{
    struct pcfetcher* fetcher = get_fetcher();
    return fetcher ? fetcher->request_sync(fetcher, url, method,
            params, timeout, resp_header) : NULL;
}


int pcfetcher_check_response(uint32_t timeout_ms)
{
    struct pcfetcher* fetcher = get_fetcher();
    return fetcher ? fetcher->check_response(fetcher,
            timeout_ms) : 0;
}

void pcfetcher_cancel_async(purc_variant_t request)
{
    struct pcfetcher* fetcher = get_fetcher();
    if (fetcher) {
        fetcher->cancel_async(fetcher, request);
    }
}

struct pcfetcher_callback_info *pcfetcher_create_callback_info()
{
    return (struct pcfetcher_callback_info*) calloc(1,
            sizeof(struct pcfetcher_callback_info));
}

void pcfetcher_destroy_callback_info(struct pcfetcher_callback_info *info)
{
    if (!info) {
        return;
    }
    if (info->header.mime_type) {
        free(info->header.mime_type);
    }
    if (info->rws) {
        purc_rwstream_destroy(info->rws);
    }
    free(info);
}

static int _local_init_once(void)
{
    return 0;
}

static int _local_init_instance(struct pcinst* curr_inst,
        const purc_instance_extra_info* extra_info)
{
    UNUSED_PARAM(extra_info);
    auto locker = holdLock(s_fetcher_lock);
    if (!s_local_fetcher) {
        s_local_fetcher = pcfetcher_local_init(curr_inst->max_conns,
                curr_inst->cache_quota);
    }

    return 0;
}

static void _local_cleanup_instance(struct pcinst* curr_inst)
{
    UNUSED_PARAM(curr_inst);

    if (s_local_fetcher) {
        s_local_fetcher->term(s_local_fetcher);
        s_local_fetcher = NULL;
    }
}

struct pcmodule _module_fetcher_local = {
    .id              = PURC_HAVE_FETCHER,
    .module_inited   = 0,

    .init_once              = _local_init_once,
    .init_instance          = _local_init_instance,
    .cleanup_instance       = _local_cleanup_instance,
};

static int _remote_init_once(void)
{
    return 0;
}

static int _remote_init_instance(struct pcinst* curr_inst,
        const purc_instance_extra_info* extra_info)
{
    UNUSED_PARAM(extra_info);

#if ENABLE(REMOTE_FETCHER)                /* { */
    auto locker = holdLock(s_fetcher_lock);
    if (curr_inst->enable_remote_fetcher && !s_remote_fetcher) {
        s_remote_fetcher = pcfetcher_remote_init(curr_inst->max_conns,
                curr_inst->cache_quota);
        if (!s_remote_fetcher)
            return PURC_ERROR_OUT_OF_MEMORY;
    }
#endif                                    /* } */

    return 0;
}

static void _remote_cleanup_instance(struct pcinst* curr_inst)
{
    UNUSED_PARAM(curr_inst);

    if (s_remote_fetcher) {
        s_remote_fetcher->term(s_remote_fetcher);
        s_remote_fetcher = NULL;
    }
}

struct pcmodule _module_fetcher_remote = {
    .id              = PURC_HAVE_FETCHER_R,
    .module_inited   = 0,

    .init_once              = _remote_init_once,
    .init_instance          = _remote_init_instance,
    .cleanup_instance       = _remote_cleanup_instance,
};

