/**
 * @file unknown_element.h
 * @author 
 * @date 2021/07/02
 * @brief The hearder file for html unknown element.
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
 *
 * This implementation of HTML parser is derived from Lexbor <http://lexbor.com/>.
 *
 * Copyright (C) 2018-2020 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */


#ifndef PCHTML_PARSER_UNKNOWN_ELEMENT_H
#define PCHTML_PARSER_UNKNOWN_ELEMENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "config.h"
#include "html/interface.h"
#include "html/interfaces/element.h"


struct pchtml_html_unknown_element {
    pchtml_html_element_t element;
};


pchtml_html_unknown_element_t *
pchtml_html_unknown_element_interface_create(
            pchtml_html_document_t *document) WTF_INTERNAL;

pchtml_html_unknown_element_t *
pchtml_html_unknown_element_interface_destroy(
            pchtml_html_unknown_element_t *unknown_element) WTF_INTERNAL;


#ifdef __cplusplus
}       /* __cplusplus */
#endif

#endif  /* PCHTML_PARSER_UNKNOWN_ELEMENT_H */
