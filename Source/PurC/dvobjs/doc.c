/*
 * @file doc.c
 * @author Xu Xiaohong
 * @date 2021/11/17
 * @brief The implementation for DOC native variant
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

#include "purc-errors.h"

#include "private/debug.h"
#include "private/errors.h"
#include "private/document.h"
#include "private/avl.h"

#include "internal.h"

struct dynamic_args {
    const char              *name;
    purc_dvariant_method     getter;
    purc_dvariant_method     setter;
};

#if 0 // VW
static inline purc_variant_t
doctype_system(struct pcdom_document *doc)
{
    const char *s = "";

    pcdom_document_type_t *doc_type;
    doc_type = doc->doctype;
    if (doc_type) {
        const unsigned char *s_system;
        size_t len;
        s_system = pcdom_document_type_system_id(doc_type, &len);
        PC_ASSERT(s_system);
        PC_ASSERT(s_system[len]=='\0');
        s = (const char*)s_system;
    }

    // NOTE: we don't hold ownership
    return purc_variant_make_string_static(s, false);
}

static inline purc_variant_t
doctype_public(struct pcdom_document *doc)
{
    const char *s = "";

    pcdom_document_type_t *doc_type;
    doc_type = doc->doctype;
    if (doc_type) {
        const unsigned char *s_public;
        size_t len;
        s_public = pcdom_document_type_public_id(doc_type, &len);
        PC_ASSERT(s_public);
        PC_ASSERT(s_public[len]=='\0');
        s = (const char*)s_public;
    }

    // NOTE: we don't hold ownership
    return purc_variant_make_string_static(s, false);
}
#endif // VW

static inline purc_variant_t
doctype_getter(void *entity,
        size_t nr_args, purc_variant_t *argv, bool silently)
{
    UNUSED_PARAM(nr_args);
    UNUSED_PARAM(argv);
    UNUSED_PARAM(silently);

    PC_ASSERT(entity);
    purc_document_t doc = (purc_document_t)entity;

    const char *doctype = "";
    switch (doc->type) {
    case PCDOC_K_TYPE_VOID:
        doctype = PCDOC_TYPE_VOID;
        break;
    case PCDOC_K_TYPE_PLAIN:
        doctype = PCDOC_TYPE_PLAIN;
        break;
    case PCDOC_K_TYPE_HTML:
        doctype = PCDOC_TYPE_HTML;
        break;
    case PCDOC_K_TYPE_XML:
        doctype = PCDOC_TYPE_XML;
        break;
    case PCDOC_K_TYPE_XGML:
        doctype = PCDOC_TYPE_XGML;
        break;
    default:
        assert(0);
        break;
    }

    return purc_variant_make_string_static(doctype, false);

#if 0 // VW
    if (argv == NULL || argv[0] == PURC_VARIANT_INVALID) {
        pcinst_set_error(PURC_ERROR_ARGUMENT_MISSED);
        return PURC_VARIANT_INVALID;
    }
    purc_variant_t v = argv[0];
    if (!purc_variant_is_string(v)) {
        pcinst_set_error(PURC_ERROR_ARGUMENT_MISSED);
        return PURC_VARIANT_INVALID;
    }

    const char *s = purc_variant_get_string_const(v);
    PC_ASSERT(s);
    if (strcmp(s, "system") == 0) {
        return doctype_system(doc);
    }
    if (strcmp(s, "public") == 0) {
        return doctype_public(doc);
    }

    pcinst_set_error(PURC_ERROR_NOT_EXISTS);
    return PURC_VARIANT_INVALID;
#endif // VW
}

static inline purc_variant_t
query(purc_document_t doc, const char *css)
{
    PC_ASSERT(doc);
    PC_ASSERT(css);

    pcdoc_element_t root = purc_document_root(doc);
    PC_ASSERT(root);

    return pcdvobjs_query_elements(doc, root, css);
}

static inline purc_variant_t
query_getter(void *entity,
        size_t nr_args, purc_variant_t *argv, bool silently)
{
    UNUSED_PARAM(silently);
    PC_ASSERT(entity);
    purc_document_t doc = (purc_document_t)entity;

    if (nr_args > 0) {
        if (argv == NULL || argv[0] == PURC_VARIANT_INVALID) {
            pcinst_set_error(PURC_ERROR_ARGUMENT_MISSED);
            return PURC_VARIANT_INVALID;
        }
        purc_variant_t v = argv[0];
        if (!purc_variant_is_string(v)) {
            pcinst_set_error(PURC_ERROR_ARGUMENT_MISSED);
            return PURC_VARIANT_INVALID;
        }
        const char *css = purc_variant_get_string_const(v);
        PC_ASSERT(css);
        return query(doc, css);
    }

    pcinst_set_error(PURC_ERROR_ARGUMENT_MISSED);
    return PURC_VARIANT_INVALID;
}

static struct native_property_cfg configs[] = {
    {"doctype", doctype_getter, NULL, NULL, NULL},
    {"query", query_getter, NULL, NULL, NULL},
};

static struct native_property_cfg*
property_cfg_by_name(const char *key_name)
{
    for (size_t i=0; i<PCA_TABLESIZE(configs); ++i) {
        struct native_property_cfg *cfg = configs + i;
        const char *property_name = cfg->property_name;
        PC_ASSERT(property_name);
        if (strcmp(property_name, key_name) == 0) {
            return cfg;
        }
    }
    return NULL;
}

// query the getter for a specific property.
static purc_nvariant_method
property_getter(const char* key_name)
{
    PC_ASSERT(key_name);
    struct native_property_cfg *cfg = property_cfg_by_name(key_name);
    if (cfg)
        return cfg->property_getter;
    return NULL;
}

// query the setter for a specific property.
static purc_nvariant_method
property_setter(const char* key_name)
{
    PC_ASSERT(key_name);
    struct native_property_cfg *cfg = property_cfg_by_name(key_name);
    if (cfg)
        return cfg->property_setter;
    return NULL;
}

// query the eraser for a specific property.
static purc_nvariant_method
property_eraser(const char* key_name)
{
    PC_ASSERT(key_name);
    struct native_property_cfg *cfg = property_cfg_by_name(key_name);
    if (cfg)
        return cfg->property_eraser;
    return NULL;
}

// query the cleaner for a specific property.
static purc_nvariant_method
property_cleaner(const char* key_name)
{
    PC_ASSERT(key_name);
    struct native_property_cfg *cfg = property_cfg_by_name(key_name);
    if (cfg)
        return cfg->property_cleaner;
    return NULL;
}

#if 0
// the updater to update the content represented by the native entity.
static purc_variant_t
updater(void* native_entity,
        size_t nr_args, purc_variant_t* argv, bool silently);

// the cleaner to clear the content represented by the native entity.
static purc_variant_t
cleaner(void* native_entity,
        size_t nr_args, purc_variant_t* argv, bool silently);

// the eraser to erase the content represented by the native entity.
static purc_variant_t
eraser(void* native_entity,
        size_t nr_args, purc_variant_t* argv, bool silently);
#endif

purc_variant_t
purc_dvobj_doc_new(purc_document_t doc)
{
    static struct purc_native_ops ops = {
        .property_getter            = property_getter,
        .property_setter            = property_setter,
        .property_eraser            = property_eraser,
        .property_cleaner           = property_cleaner,

        .updater                    = NULL,
        .cleaner                    = NULL,
        .eraser                     = NULL,

        .on_observe                = NULL,
        .on_release                = NULL,
    };

    PC_ASSERT(doc);

    return purc_variant_make_native(doc, &ops);
}

