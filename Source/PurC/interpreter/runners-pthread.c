/*
 * @file runners-pthread.c
 * @author Vincent Wei
 * @date 2022/07/05
 * @brief The implementation of purc_inst_xxx APIs using POSIX Thread.
 *
 * Copyright (C) 2022 FMSoft <https://www.fmsoft.cn>
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

#if USE(PTHREADS)

#include "purc.h"
#include "private/runners.h"

#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <fcntl.h>           /* For O_* constants */
#include <assert.h>

static void create_coroutine(const pcrdr_msg *msg, pcrdr_msg *response)
{
    UNUSED_PARAM(msg);
    UNUSED_PARAM(response);

    if (msg->dataType != PCRDR_MSG_DATA_TYPE_JSON)
        return;

    assert(msg->data);

    purc_variant_t tmp;

    purc_vdom_t vdom = NULL;
    tmp = purc_variant_object_get_by_ckey(msg->data, "vdom");
    if (tmp && purc_variant_is_ulongint(tmp)) {
        uint64_t u64;
        purc_variant_cast_to_ulongint(tmp, &u64, false);
        vdom = (purc_vdom_t)(uintptr_t)u64;
    }

    if (vdom == NULL)
        return;

    purc_atom_t curator = 0;
    tmp = purc_variant_object_get_by_ckey(msg->data, "curator");
    if (tmp && purc_variant_is_ulongint(tmp)) {
        uint64_t u64;
        purc_variant_cast_to_ulongint(tmp, &u64, false);
        curator = (purc_atom_t)u64;
    }

    pcrdr_page_type page_type = PCRDR_PAGE_TYPE_NULL;
    tmp = purc_variant_object_get_by_ckey(msg->data, "pageType");
    if (tmp && purc_variant_is_ulongint(tmp)) {
        uint64_t u64;
        purc_variant_cast_to_ulongint(tmp, &u64, false);
        page_type = (pcrdr_page_type)u64;
    }

    purc_variant_t request;
    request = purc_variant_object_get_by_ckey(msg->data, "request");
    if (request)
        purc_variant_ref(request);

    const char *target_workspace;
    tmp = purc_variant_object_get_by_ckey(msg->data, "targetWorkspace");
    if (tmp) {
        target_workspace = purc_variant_get_string_const(tmp);
    }

    const char *target_group;
    tmp = purc_variant_object_get_by_ckey(msg->data, "targetGroup");
    if (tmp) {
        target_group = purc_variant_get_string_const(tmp);
    }

    const char *page_name;
    tmp = purc_variant_object_get_by_ckey(msg->data, "pageName");
    if (tmp) {
        page_name = purc_variant_get_string_const(tmp);
    }

    purc_renderer_extra_info extra_rdr_info = {};

    tmp = purc_variant_object_get_by_ckey(msg->data, "class");
    if (tmp) {
        extra_rdr_info.klass = purc_variant_get_string_const(tmp);
    }

    tmp = purc_variant_object_get_by_ckey(msg->data, "title");
    if (tmp) {
        extra_rdr_info.title = purc_variant_get_string_const(tmp);
    }

    tmp = purc_variant_object_get_by_ckey(msg->data, "layoutStyle");
    if (tmp) {
        extra_rdr_info.layout_style = purc_variant_get_string_const(tmp);
    }

    extra_rdr_info.toolkit_style =
        purc_variant_object_get_by_ckey(msg->data, "toolkitStyle");
    if (extra_rdr_info.toolkit_style)
        purc_variant_ref(extra_rdr_info.toolkit_style);

    tmp = purc_variant_object_get_by_ckey(msg->data, "pageGroups");
    if (tmp) {
        extra_rdr_info.page_groups = purc_variant_get_string_const(tmp);
    }

    const char *body_id = NULL;
    tmp = purc_variant_object_get_by_ckey(msg->data, "bodyId");
    if (tmp) {
        body_id = purc_variant_get_string_const(tmp);
    }

    purc_coroutine_t cor = purc_schedule_vdom(vdom, curator,
            request, page_type, target_workspace,
            target_group, page_name, &extra_rdr_info, body_id, NULL);

    if (cor) {
        purc_atom_t cor_atom = purc_coroutine_identifier(cor);

        const char *endpoint_name = purc_get_endpoint(NULL);
        response->type = PCRDR_MSG_TYPE_RESPONSE;
        response->requestId = purc_variant_ref(msg->requestId);
        response->sourceURI = purc_variant_make_string(endpoint_name, false);
        response->retCode = PCRDR_SC_OK;
        response->resultValue = (uint64_t)cor_atom;
        response->dataType = PCRDR_MSG_DATA_TYPE_VOID;
        response->data = PURC_VARIANT_INVALID;
    }
}

struct inst_arg {
    sem_t      *sync;
    const char *app;
    const char *run;
    const purc_instance_extra_info* extra_info;
    purc_atom_t atom;
};

static pcrdr_msg *inst_extra_message_source(pcrdr_conn* conn, void *ctxt)
{
    UNUSED_PARAM(conn);
    UNUSED_PARAM(ctxt);

    size_t n;

    int ret = purc_inst_holding_messages_count(&n);
    if (ret) {
        purc_log_error("Failed: %d\n", ret);
    }
    else if (n > 0) {
        return purc_inst_take_away_message(0);
    }

    return NULL;
}

/*
   A request message sent to the instance can be used to manage
   the coroutines, for example, create or kill a coroutine. This type
   of request can also be used to implement the debugger. The debugger
   can send the operations like `pauseCoroutine` or `resumeCoroutine`
   to control the execution of a coroutine.

   When controlling an existing coroutine, we use `elementValue` to
   pass the atom value of the target coroutine. In this situation,
   the `elementType` should be `PCRDR_MSG_ELEMENT_HANDLE`.

   When the target of a request is a coroutine, the target value should be
   the atom value of the coroutine identifier.

   Generally, a `callMethod` request sent to a coroutine should be handled by
   an operation group which is scoped at the specified element of the document.

   For this purpose,

   1. the `elementValue` of the message can contain the identifier of
   the element in vDOM; the `elementType` should be `PCRDR_MSG_ELEMENT_TYPE_ID`.

   2. the `data` of the message should be an object variant, which contains
   the variable name of the operation group and the argument for calling
   the operation group.

   When the instance got such a request message, it should dispatch the message
   to the target coroutine. And the coroutine should prepare a virtual
   stack frame to call the operation group in the scope of the specified
   element. The result of the operation group should be sent back to the caller
   as a response message.

   In this way, the coroutine can act as a service provider for others.
 */
static void inst_request_handler(pcrdr_conn* conn, const pcrdr_msg *msg)
{
    UNUSED_PARAM(conn);

    const char* source_uri;
    purc_atom_t requester;

    source_uri = purc_variant_get_string_const(msg->sourceURI);
    if (source_uri == NULL || (requester =
                purc_atom_try_string_ex(PURC_ATOM_BUCKET_USER,
                    source_uri)) == 0) {
        purc_log_error("No sourceURI or the requester disappeared\n");
        return;
    }

    pcrdr_msg *response = pcrdr_make_void_message();

    const char *op;
    op = purc_variant_get_string_const(msg->operation);
    assert(op);

    if (msg->target == PCRDR_MSG_TARGET_INSTANCE) {
        if (strcmp(op, PCRUN_OPERATION_createCoroutine) == 0) {
            create_coroutine(msg, response);
        }
        else if (strcmp(op, PCRUN_OPERATION_killCoroutine) == 0) {
            purc_log_warn("Not implemented operation: %s\n", op);
        }
        else if (strcmp(op, PCRUN_OPERATION_pauseCoroutine) == 0) {
            purc_log_warn("Not implemented operation: %s\n", op);
        }
        else if (strcmp(op, PCRUN_OPERATION_resumeCoroutine) == 0) {
            purc_log_warn("Not implemented operation: %s\n", op);
        }
        else if (strcmp(op, PCRUN_OPERATION_shutdownInstance) == 0) {
            purc_log_warn("Not implemented operation: %s\n", op);
        }
        else {
            purc_log_warn("Unknown operation: %s\n", op);
        }
    }
    else if (msg->target == PCRDR_MSG_TARGET_COROUTINE) {
        if (strcmp(op, PCRDR_OPERATION_CALLMETHOD) == 0) {
            purc_log_warn("Not implemented operation: %s\n", op);
        }
        else {
            purc_log_warn("Unknown operation: %s\n", op);
        }
    }

    if (response->type == PCRDR_MSG_TYPE_VOID) {
        /* must be a bad request */
        response->type = PCRDR_MSG_TYPE_RESPONSE;
        response->requestId = purc_variant_ref(msg->requestId);
        response->sourceURI = purc_variant_make_string(
                purc_get_endpoint(NULL), false);
        response->retCode = PCRDR_SC_BAD_REQUEST;
        response->resultValue = 0;
        response->dataType = PCRDR_MSG_DATA_TYPE_VOID;
        response->data = PURC_VARIANT_INVALID;
    }

    const char *request_id;
    request_id = purc_variant_get_string_const(msg->requestId);
    if (strcmp(request_id, PCRDR_REQUESTID_NORETURN)) {
        purc_inst_move_message(requester, response);
    }

    pcrdr_release_message(response);
}

static int
inst_event_handler(purc_coroutine_t cor, purc_event_t event, void *data)
{
    UNUSED_PARAM(cor);
    UNUSED_PARAM(data);

    if (event == PURC_EVENT_NOCOR) {
        struct pcrun_inst_info *info;

        purc_get_local_data(PCRUN_LOCAL_DATA, (uintptr_t *)&info, NULL);
        assert(info);

        return info->request_to_shutdown ? -1 : 0;
    }

    return 0;
}

static void* general_instance_entry(void* arg)
{
    struct inst_arg *inst_arg = (struct inst_arg *)arg;

    int ret = purc_init_ex(PURC_MODULE_HVML,
            inst_arg->app, inst_arg->run, inst_arg->extra_info);

    if (ret != PURC_ERROR_OK) {
        inst_arg->atom = 0;
        sem_post(inst_arg->sync);
        return NULL;
    }

    purc_enable_log(false, false);

    inst_arg->atom =
        purc_inst_create_move_buffer(PCINST_MOVE_BUFFER_BROADCAST, 16);
    sem_post(inst_arg->sync);
    if (inst_arg->atom == 0)
        return NULL;

    struct pcrdr_conn *conn = purc_get_conn_to_renderer();
    assert(conn);

    pcrdr_conn_set_extra_message_source(conn, inst_extra_message_source,
            NULL, NULL);

    /* TODO: It is better that the runloop installs a request handler against
       the renderer connection, then dispatches the request messages which
       were sent to instance or coroutine to a request handler against
       the instance. */
    pcrdr_conn_set_request_handler(conn, inst_request_handler);

    struct pcrun_inst_info info = {};
    purc_set_local_data(PCRUN_LOCAL_DATA, (uintptr_t)(&info), NULL);

    purc_run(inst_event_handler);

    size_t n = purc_inst_destroy_move_buffer();
    purc_log_debug("Move buffer destroyed, %u messages discarded\n",
            (unsigned)n);

    purc_cleanup();
    return NULL;
}

static purc_atom_t
start_instance(const char *app, const char *run,
        const purc_instance_extra_info* extra_info)
{
    pthread_t th;
    struct inst_arg arg;

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    char sem_name[PURC_LEN_UNIQUE_ID + 1];
    purc_generate_unique_id(sem_name, "/SEM4SYNC");
    sem_unlink(sem_name);
    arg.sync = sem_open(sem_name, O_CREAT | O_EXCL, 0644, 0);
    if (arg.sync == SEM_FAILED) {
        purc_set_error(PURC_ERROR_BAD_SYSTEM_CALL);
        purc_log_error("failed to create semaphore: %s\n", strerror(errno));
        return 0;
    }
    arg.app = app;
    arg.run = run;
    arg.extra_info = extra_info;
    int ret = pthread_create(&th, &attr, general_instance_entry, &arg);
    if (ret) {
        sem_close(arg.sync);
        purc_log_error("failed to create thread for instance: %s/%s\n",
                app, run);
        return 0;
    }
    pthread_attr_destroy(&attr);

    sem_wait(arg.sync);
    sem_close(arg.sync);

    return arg.atom;
}

purc_atom_t
purc_inst_create_or_get(const char *app_name, const char *runner_name,
        const purc_instance_extra_info* extra_info)
{
    char endpoint_name[PURC_LEN_ENDPOINT_NAME + 1];

    if (!purc_is_valid_app_name(app_name) ||
            !purc_is_valid_runner_name(runner_name)) {
        purc_set_error(PURC_ERROR_INVALID_VALUE);
        return 0;
    }

    purc_assemble_endpoint_name_ex(PCRDR_LOCALHOST,
            app_name, runner_name,
            endpoint_name, sizeof(endpoint_name) - 1);

    purc_atom_t atom = purc_atom_try_string_ex(PURC_ATOM_BUCKET_USER,
            endpoint_name);
    if (atom == 0) {
        atom = start_instance(app_name, runner_name, extra_info);
    }

    return atom;
}

purc_atom_t
purc_inst_schedule_vdom(purc_atom_t inst, purc_vdom_t vdom,
        purc_atom_t curator, purc_variant_t request,
        pcrdr_page_type page_type, const char *target_workspace,
        const char *target_group, const char *page_name,
        purc_renderer_extra_info *extra_rdr_info,
        const char *body_id)
{
    const char *inst_endpoint = purc_atom_to_string(inst);
    if (inst_endpoint) {
        pcrdr_msg *request_msg = pcrdr_make_request_message(
                PCRDR_MSG_TARGET_INSTANCE, inst,
                PCRUN_OPERATION_createCoroutine, NULL,
                purc_get_endpoint(NULL),
                PCRDR_MSG_ELEMENT_TYPE_VOID, NULL,
                NULL,
                PCRDR_MSG_DATA_TYPE_VOID, NULL, 0);

        purc_variant_t data, tmp;
        data = purc_variant_make_object_0();

        tmp = purc_variant_make_ulongint((uint64_t)(uintptr_t)vdom);
        purc_variant_object_set_by_static_ckey(data, "vdom", tmp);
        purc_variant_unref(tmp);

        tmp = purc_variant_make_ulongint((uint64_t)curator);
        purc_variant_object_set_by_static_ckey(data, "curator", tmp);
        purc_variant_unref(tmp);

        purc_variant_object_set_by_static_ckey(data, "request", request);

        tmp = purc_variant_make_ulongint((uint64_t)page_type);
        purc_variant_object_set_by_static_ckey(data, "pageType", tmp);
        purc_variant_unref(tmp);

        if (target_workspace) {
            tmp = purc_variant_make_string(target_workspace, false);
            purc_variant_object_set_by_static_ckey(data, "targetWorkspace", tmp);
            purc_variant_unref(tmp);
        }

        if (target_group) {
            tmp = purc_variant_make_string(target_group, false);
            purc_variant_object_set_by_static_ckey(data, "targetGroup", tmp);
            purc_variant_unref(tmp);
        }

        if (page_name) {
            tmp = purc_variant_make_string(page_name, false);
            purc_variant_object_set_by_static_ckey(data, "pageName", tmp);
            purc_variant_unref(tmp);
        }

        if (extra_rdr_info) {
            if (extra_rdr_info->klass) {
                tmp = purc_variant_make_string(extra_rdr_info->klass, false);
                purc_variant_object_set_by_static_ckey(data, "class", tmp);
                purc_variant_unref(tmp);
            }

            if (extra_rdr_info->title) {
                tmp = purc_variant_make_string(extra_rdr_info->title, false);
                purc_variant_object_set_by_static_ckey(data, "title", tmp);
                purc_variant_unref(tmp);
            }

            if (extra_rdr_info->layout_style) {
                tmp = purc_variant_make_string(extra_rdr_info->layout_style,
                        false);
                purc_variant_object_set_by_static_ckey(data, "layoutStyle",
                        tmp);
                purc_variant_unref(tmp);
            }

            if (extra_rdr_info->toolkit_style) {
                purc_variant_object_set_by_static_ckey(data, "toolkitStyle",
                        extra_rdr_info->toolkit_style);
            }
        }

        if (body_id) {
            tmp = purc_variant_make_string(body_id, false);
            purc_variant_object_set_by_static_ckey(data, "bodyId", tmp);
            purc_variant_unref(tmp);
        }

        request_msg->dataType = PCRDR_MSG_DATA_TYPE_JSON;
        request_msg->data = data;
        purc_inst_move_message(inst, request_msg);

        struct pcrdr_conn *conn = purc_get_conn_to_renderer();
        assert(conn);

        pcrdr_msg *response;
        int ret = pcrdr_wait_response_for_specific_request(conn,
                request_msg->requestId, 1, &response);
        pcrdr_release_message(request_msg);

        if (ret || response->retCode != PCRDR_SC_OK) {
            purc_log_error("Failed to schedule vDOM in another instance\n");
        }
        else {
            return (purc_atom_t)response->resultValue;
        }
    }

    return 0;
}

#endif /* USE(PTHREADS) */
