/**
 * @file dvojbs.h
 * @author 
 * @date 2021/07/02
 * @brief The interface for dynamic variant objects.
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

#ifndef PURC_PRIVATE_DVOBJS_H
#define PURC_PRIVATE_DVOBJS_H

#include "config.h"
#include "purc-rwstream.h"
#include "purc-variant.h"

#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

// initialize dvobjs module (once)
void pcdvobjs_init_once(void) WTF_INTERNAL;

struct pcinst;

// initialize the dvobjs module for a PurC instance.
void pcdvobjs_init_instance(struct pcinst* inst) WTF_INTERNAL;

// clean up the dvobjs module for a PurC instance.
void pcdvobjs_cleanup_instance(struct pcinst* inst) WTF_INTERNAL;

purc_variant_t pcdvojbs_get_system(void);
purc_variant_t pcdvojbs_get_string(void);
purc_variant_t pcdvojbs_get_math(void);
purc_variant_t pcdvojbs_get_logical(void);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif  /* PURC_PRIVATE_DVOBJS_H */

