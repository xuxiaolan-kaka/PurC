/*
 * @file tokenizer.c
 * @author Xue Shuming
 * @date 2022/02/08
 * @brief The implementation of hvml parser.
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
 */

#include "config.h"

#include "private/instance.h"
#include "private/errors.h"
#include "private/debug.h"
#include "private/utils.h"
#include "private/dom.h"
#include "private/hvml.h"

#include "hvml-buffer.h"
#include "hvml-rwswrap.h"
#include "hvml-token.h"
#include "hvml-sbst.h"
#include "hvml-attr.h"
#include "hvml-tag.h"

#include <math.h>

#if HAVE(GLIB)
#include <gmodule.h>
#else
#include <stdlib.h>
#endif

#include "hvml_err_msgs.inc"

#if HAVE(GLIB)
#define    PCHVML_ALLOC(sz)   g_slice_alloc0(sz)
#define    PCHVML_FREE(p)     g_slice_free1(sizeof(*p), (gpointer)p)
#else
#define    PCHVML_ALLOC(sz)   calloc(1, sz)
#define    PCHVML_FREE(p)     free(p)
#endif

#define HVML_STATE_DATA  \
        "DATA_STATE"
#define HVML_STATE_TAG_OPEN  \
        "TAG_OPEN_STATE"
#define HVML_STATE_END_TAG_OPEN  \
        "END_TAG_OPEN_STATE"
#define HVML_STATE_TAG_CONTENT \
        "TAG_CONTENT_STATE"
#define HVML_STATE_TAG_NAME  \
        "TAG_NAME_STATE"
#define HVML_STATE_BEFORE_ATTRIBUTE_NAME  \
        "BEFORE_ATTRIBUTE_NAME_STATE"
#define HVML_STATE_ATTRIBUTE_NAME  \
        "ATTRIBUTE_NAME_STATE"
#define HVML_STATE_AFTER_ATTRIBUTE_NAME  \
        "AFTER_ATTRIBUTE_NAME_STATE"
#define HVML_STATE_BEFORE_ATTRIBUTE_VALUE  \
        "BEFORE_ATTRIBUTE_VALUE_STATE"
#define HVML_STATE_ATTRIBUTE_VALUE_DOUBLE_QUOTED  \
        "ATTRIBUTE_VALUE_DOUBLE_QUOTED_STATE"
#define HVML_STATE_ATTRIBUTE_VALUE_SINGLE_QUOTED  \
        "ATTRIBUTE_VALUE_SINGLE_QUOTED_STATE"
#define HVML_STATE_ATTRIBUTE_VALUE_UNQUOTED  \
        "ATTRIBUTE_VALUE_UNQUOTED_STATE"
#define HVML_STATE_AFTER_ATTRIBUTE_VALUE_QUOTED  \
        "AFTER_ATTRIBUTE_VALUE_QUOTED_STATE"
#define HVML_STATE_SELF_CLOSING_START_TAG  \
        "SELF_CLOSING_START_TAG_STATE"
#define HVML_STATE_BOGUS_COMMENT  \
        "BOGUS_COMMENT_STATE"
#define HVML_STATE_MARKUP_DECLARATION_OPEN  \
        "MARKUP_DECLARATION_OPEN_STATE"
#define HVML_STATE_COMMENT_START  \
        "COMMENT_START_STATE"
#define HVML_STATE_COMMENT_START_DASH  \
        "COMMENT_START_DASH_STATE"
#define HVML_STATE_COMMENT  \
        "COMMENT_STATE"
#define HVML_STATE_COMMENT_LESS_THAN_SIGN  \
        "COMMENT_LESS_THAN_SIGN_STATE"
#define HVML_STATE_COMMENT_LESS_THAN_SIGN_BANG  \
        "COMMENT_LESS_THAN_SIGN_BANG_STATE"
#define HVML_STATE_COMMENT_LESS_THAN_SIGN_BANG_DASH  \
        "COMMENT_LESS_THAN_SIGN_BANG_DASH_STATE"
#define HVML_STATE_COMMENT_LESS_THAN_SIGN_BANG_DASH_DASH  \
        "COMMENT_LESS_THAN_SIGN_BANG_DASH_DASH_STATE"
#define HVML_STATE_COMMENT_END_DASH  \
        "COMMENT_END_DASH_STATE"
#define HVML_STATE_COMMENT_END  \
        "COMMENT_END_STATE"
#define HVML_STATE_COMMENT_END_BANG  \
        "COMMENT_END_BANG_STATE"
#define HVML_STATE_DOCTYPE  \
        "DOCTYPE_STATE"
#define HVML_STATE_BEFORE_DOCTYPE_NAME  \
        "BEFORE_DOCTYPE_NAME_STATE"
#define HVML_STATE_DOCTYPE_NAME  \
        "DOCTYPE_NAME_STATE"
#define HVML_STATE_AFTER_DOCTYPE_NAME  \
        "AFTER_DOCTYPE_NAME_STATE"
#define HVML_STATE_AFTER_DOCTYPE_PUBLIC_KEYWORD  \
        "AFTER_DOCTYPE_PUBLIC_KEYWORD_STATE"
#define HVML_STATE_BEFORE_DOCTYPE_PUBLIC_ID  \
        "BEFORE_DOCTYPE_PUBLIC_ID_STATE"
#define HVML_STATE_DOCTYPE_PUBLIC_ID_DOUBLE_QUOTED  \
        "DOCTYPE_PUBLIC_ID_DOUBLE_QUOTED_STATE"
#define HVML_STATE_DOCTYPE_PUBLIC_ID_SINGLE_QUOTED  \
        "DOCTYPE_PUBLIC_ID_SINGLE_QUOTED_STATE"
#define HVML_STATE_AFTER_DOCTYPE_PUBLIC_ID  \
        "AFTER_DOCTYPE_PUBLIC_ID_STATE"
#define HVML_STATE_BETWEEN_DOCTYPE_PUBLIC_ID_AND_SYSTEM_INFO  \
        "BETWEEN_DOCTYPE_PUBLIC_ID_AND_SYSTEM_INFO_STATE"
#define HVML_STATE_AFTER_DOCTYPE_SYSTEM_KEYWORD  \
        "AFTER_DOCTYPE_SYSTEM_KEYWORD_STATE"
#define HVML_STATE_BEFORE_DOCTYPE_SYSTEM  \
        "BEFORE_DOCTYPE_SYSTEM_STATE"
#define HVML_STATE_DOCTYPE_SYSTEM_DOUBLE_QUOTED  \
        "DOCTYPE_SYSTEM_DOUBLE_QUOTED_STATE"
#define HVML_STATE_DOCTYPE_SYSTEM_SINGLE_QUOTED  \
        "DOCTYPE_SYSTEM_SINGLE_QUOTED_STATE"
#define HVML_STATE_AFTER_DOCTYPE_SYSTEM  \
        "AFTER_DOCTYPE_SYSTEM_STATE"
#define HVML_STATE_BOGUS_DOCTYPE  \
        "BOGUS_DOCTYPE_STATE"
#define HVML_STATE_CDATA_SECTION  \
        "CDATA_SECTION_STATE"
#define HVML_STATE_CDATA_SECTION_BRACKET  \
        "CDATA_SECTION_BRACKET_STATE"
#define HVML_STATE_CDATA_SECTION_END  \
        "CDATA_SECTION_END_STATE"
#define HVML_STATE_CHARACTER_REFERENCE  \
        "CHARACTER_REFERENCE_STATE"
#define HVML_STATE_NAMED_CHARACTER_REFERENCE  \
        "NAMED_CHARACTER_REFERENCE_STATE"
#define HVML_STATE_AMBIGUOUS_AMPERSAND  \
        "AMBIGUOUS_AMPERSAND_STATE"
#define HVML_STATE_NUMERIC_CHARACTER_REFERENCE  \
        "NUMERIC_CHARACTER_REFERENCE_STATE"
#define HVML_STATE_HEXADECIMAL_CHARACTER_REFERENCE_START  \
        "HEXADECIMAL_CHARACTER_REFERENCE_START_STATE"
#define HVML_STATE_DECIMAL_CHARACTER_REFERENCE_START  \
        "DECIMAL_CHARACTER_REFERENCE_START_STATE"
#define HVML_STATE_HEXADECIMAL_CHARACTER_REFERENCE  \
        "HEXADECIMAL_CHARACTER_REFERENCE_STATE"
#define HVML_STATE_DECIMAL_CHARACTER_REFERENCE  \
        "DECIMAL_CHARACTER_REFERENCE_STATE"
#define HVML_STATE_NUMERIC_CHARACTER_REFERENCE_END  \
        "NUMERIC_CHARACTER_REFERENCE_END_STATE"
#define HVML_STATE_SPECIAL_ATTRIBUTE_OPERATOR_IN_ATTRIBUTE_NAME  \
        "SPECIAL_ATTRIBUTE_OPERATOR_IN_ATTRIBUTE_NAME_STATE"
#define HVML_STATE_SPECIAL_ATTRIBUTE_OPERATOR_AFTER_ATTRIBUTE_NAME  \
        "SPECIAL_ATTRIBUTE_OPERATOR_AFTER_ATTRIBUTE_NAME_STATE"
#define HVML_STATE_JSONTEXT_CONTENT  \
        "JSONTEXT_CONTENT_STATE"
#define HVML_STATE_TEXT_CONTENT  \
        "TEXT_CONTENT_STATE"
#define HVML_STATE_JSONEE_ATTRIBUTE_VALUE_DOUBLE_QUOTED  \
        "JSONEE_ATTRIBUTE_VALUE_DOUBLE_QUOTED_STATE"
#define HVML_STATE_JSONEE_ATTRIBUTE_VALUE_SINGLE_QUOTED  \
        "JSONEE_ATTRIBUTE_VALUE_SINGLE_QUOTED_STATE"
#define HVML_STATE_JSONEE_ATTRIBUTE_VALUE_UNQUOTED  \
        "JSONEE_ATTRIBUTE_VALUE_UNQUOTED_STATE"
#define HVML_STATE_EJSON_DATA  \
        "EJSON_DATA_STATE"
#define HVML_STATE_EJSON_FINISHED  \
        "EJSON_FINISHED_STATE"
#define HVML_STATE_EJSON_CONTROL  \
        "EJSON_CONTROL_STATE"
#define HVML_STATE_EJSON_LEFT_BRACE  \
        "EJSON_LEFT_BRACE_STATE"
#define HVML_STATE_EJSON_RIGHT_BRACE  \
        "EJSON_RIGHT_BRACE_STATE"
#define HVML_STATE_EJSON_LEFT_BRACKET  \
        "EJSON_LEFT_BRACKET_STATE"
#define HVML_STATE_EJSON_RIGHT_BRACKET  \
        "EJSON_RIGHT_BRACKET_STATE"
#define HVML_STATE_EJSON_LEFT_PARENTHESIS  \
        "EJSON_LEFT_PARENTHESIS_STATE"
#define HVML_STATE_EJSON_RIGHT_PARENTHESIS  \
        "EJSON_RIGHT_PARENTHESIS_STATE"
#define HVML_STATE_EJSON_DOLLAR  \
        "EJSON_DOLLAR_STATE"
#define HVML_STATE_EJSON_AFTER_VALUE  \
        "EJSON_AFTER_VALUE_STATE"
#define HVML_STATE_EJSON_BEFORE_NAME  \
        "EJSON_BEFORE_NAME_STATE"
#define HVML_STATE_EJSON_AFTER_NAME  \
        "EJSON_AFTER_NAME_STATE"
#define HVML_STATE_EJSON_NAME_UNQUOTED  \
        "EJSON_NAME_UNQUOTED_STATE"
#define HVML_STATE_EJSON_NAME_SINGLE_QUOTED  \
        "EJSON_NAME_SINGLE_QUOTED_STATE"
#define HVML_STATE_EJSON_NAME_DOUBLE_QUOTED  \
        "EJSON_NAME_DOUBLE_QUOTED_STATE"
#define HVML_STATE_EJSON_VALUE_SINGLE_QUOTED  \
        "EJSON_VALUE_SINGLE_QUOTED_STATE"
#define HVML_STATE_EJSON_VALUE_DOUBLE_QUOTED  \
        "EJSON_VALUE_DOUBLE_QUOTED_STATE"
#define HVML_STATE_EJSON_AFTER_VALUE_DOUBLE_QUOTED  \
        "EJSON_AFTER_VALUE_DOUBLE_QUOTED_STATE"
#define HVML_STATE_EJSON_VALUE_TWO_DOUBLE_QUOTED  \
        "EJSON_VALUE_TWO_DOUBLE_QUOTED_STATE"
#define HVML_STATE_EJSON_VALUE_THREE_DOUBLE_QUOTED  \
        "EJSON_VALUE_THREE_DOUBLE_QUOTED_STATE"
#define HVML_STATE_EJSON_KEYWORD  \
        "EJSON_KEYWORD_STATE"
#define HVML_STATE_EJSON_AFTER_KEYWORD  \
        "EJSON_AFTER_KEYWORD_STATE"
#define HVML_STATE_EJSON_BYTE_SEQUENCE  \
        "EJSON_BYTE_SEQUENCE_STATE"
#define HVML_STATE_EJSON_AFTER_BYTE_SEQUENCE  \
        "EJSON_AFTER_BYTE_SEQUENCE_STATE"
#define HVML_STATE_EJSON_HEX_BYTE_SEQUENCE  \
        "EJSON_HEX_BYTE_SEQUENCE_STATE"
#define HVML_STATE_EJSON_BINARY_BYTE_SEQUENCE  \
        "EJSON_BINARY_BYTE_SEQUENCE_STATE"
#define HVML_STATE_EJSON_BASE64_BYTE_SEQUENCE  \
        "EJSON_BASE64_BYTE_SEQUENCE_STATE"
#define HVML_STATE_EJSON_VALUE_NUMBER  \
        "EJSON_VALUE_NUMBER_STATE"
#define HVML_STATE_EJSON_AFTER_VALUE_NUMBER  \
        "EJSON_AFTER_VALUE_NUMBER_STATE"
#define HVML_STATE_EJSON_VALUE_NUMBER_INTEGER  \
        "EJSON_VALUE_NUMBER_INTEGER_STATE"
#define HVML_STATE_EJSON_VALUE_NUMBER_FRACTION  \
        "EJSON_VALUE_NUMBER_FRACTION_STATE"
#define HVML_STATE_EJSON_VALUE_NUMBER_EXPONENT  \
        "EJSON_VALUE_NUMBER_EXPONENT_STATE"
#define HVML_STATE_EJSON_VALUE_NUMBER_EXPONENT_INTEGER  \
        "EJSON_VALUE_NUMBER_EXPONENT_INTEGER_STATE"
#define HVML_STATE_EJSON_VALUE_NUMBER_SUFFIX_INTEGER  \
        "EJSON_VALUE_NUMBER_SUFFIX_INTEGER_STATE"
#define HVML_STATE_EJSON_VALUE_NUMBER_INFINITY  \
        "EJSON_VALUE_NUMBER_INFINITY_STATE"
#define HVML_STATE_EJSON_VALUE_NAN  \
        "EJSON_VALUE_NAN_STATE"
#define HVML_STATE_EJSON_STRING_ESCAPE  \
        "EJSON_STRING_ESCAPE_STATE"
#define HVML_STATE_EJSON_STRING_ESCAPE_FOUR_HEXADECIMAL_DIGITS  \
        "EJSON_STRING_ESCAPE_FOUR_HEXADECIMAL_DIGITS_STATE"
#define HVML_STATE_EJSON_JSONEE_VARIABLE  \
        "EJSON_JSONEE_VARIABLE_STATE"
#define HVML_STATE_EJSON_JSONEE_FULL_STOP_SIGN  \
        "EJSON_JSONEE_FULL_STOP_SIGN_STATE"
#define HVML_STATE_EJSON_JSONEE_KEYWORD  \
        "EJSON_JSONEE_KEYWORD_STATE"
#define HVML_STATE_EJSON_JSONEE_STRING  \
        "EJSON_JSONEE_STRING_STATE"
#define HVML_STATE_EJSON_AFTER_JSONEE_STRING  \
        "EJSON_AFTER_JSONEE_STRING_STATE"
#define HVML_STATE_EJSON_TEMPLATE_DATA  \
        "EJSON_TEMPLATE_DATA_STATE"
#define HVML_STATE_EJSON_TEMPLATE_DATA_LESS_THAN_SIGN  \
        "EJSON_TEMPLATE_DATA_LESS_THAN_SIGN_STATE"
#define HVML_STATE_EJSON_TEMPLATE_DATA_END_TAG_OPEN  \
        "EJSON_TEMPLATE_DATA_END_TAG_OPEN_STATE"
#define HVML_STATE_EJSON_TEMPLATE_DATA_END_TAG_NAME  \
        "EJSON_TEMPLATE_DATA_END_TAG_NAME_STATE"
#define HVML_STATE_EJSON_TEMPLATE_FINISHED  \
        "EJSON_TEMPLATE_FINISHED_STATE"

static const char *state_names[] = {
    HVML_STATE_DATA,
    HVML_STATE_TAG_OPEN,
    HVML_STATE_END_TAG_OPEN,
    HVML_STATE_TAG_CONTENT,
    HVML_STATE_TAG_NAME,
    HVML_STATE_BEFORE_ATTRIBUTE_NAME,
    HVML_STATE_ATTRIBUTE_NAME,
    HVML_STATE_AFTER_ATTRIBUTE_NAME,
    HVML_STATE_BEFORE_ATTRIBUTE_VALUE,
    HVML_STATE_ATTRIBUTE_VALUE_DOUBLE_QUOTED,
    HVML_STATE_ATTRIBUTE_VALUE_SINGLE_QUOTED,
    HVML_STATE_ATTRIBUTE_VALUE_UNQUOTED,
    HVML_STATE_AFTER_ATTRIBUTE_VALUE_QUOTED,
    HVML_STATE_SELF_CLOSING_START_TAG,
    HVML_STATE_BOGUS_COMMENT,
    HVML_STATE_MARKUP_DECLARATION_OPEN,
    HVML_STATE_COMMENT_START,
    HVML_STATE_COMMENT_START_DASH,
    HVML_STATE_COMMENT,
    HVML_STATE_COMMENT_LESS_THAN_SIGN,
    HVML_STATE_COMMENT_LESS_THAN_SIGN_BANG,
    HVML_STATE_COMMENT_LESS_THAN_SIGN_BANG_DASH,
    HVML_STATE_COMMENT_LESS_THAN_SIGN_BANG_DASH_DASH,
    HVML_STATE_COMMENT_END_DASH,
    HVML_STATE_COMMENT_END,
    HVML_STATE_COMMENT_END_BANG,
    HVML_STATE_DOCTYPE,
    HVML_STATE_BEFORE_DOCTYPE_NAME,
    HVML_STATE_DOCTYPE_NAME,
    HVML_STATE_AFTER_DOCTYPE_NAME,
    HVML_STATE_AFTER_DOCTYPE_PUBLIC_KEYWORD,
    HVML_STATE_BEFORE_DOCTYPE_PUBLIC_ID,
    HVML_STATE_DOCTYPE_PUBLIC_ID_DOUBLE_QUOTED,
    HVML_STATE_DOCTYPE_PUBLIC_ID_SINGLE_QUOTED,
    HVML_STATE_AFTER_DOCTYPE_PUBLIC_ID,
    HVML_STATE_BETWEEN_DOCTYPE_PUBLIC_ID_AND_SYSTEM_INFO,
    HVML_STATE_AFTER_DOCTYPE_SYSTEM_KEYWORD,
    HVML_STATE_BEFORE_DOCTYPE_SYSTEM,
    HVML_STATE_DOCTYPE_SYSTEM_DOUBLE_QUOTED,
    HVML_STATE_DOCTYPE_SYSTEM_SINGLE_QUOTED,
    HVML_STATE_AFTER_DOCTYPE_SYSTEM,
    HVML_STATE_BOGUS_DOCTYPE,
    HVML_STATE_CDATA_SECTION,
    HVML_STATE_CDATA_SECTION_BRACKET,
    HVML_STATE_CDATA_SECTION_END,
    HVML_STATE_CHARACTER_REFERENCE,
    HVML_STATE_NAMED_CHARACTER_REFERENCE,
    HVML_STATE_AMBIGUOUS_AMPERSAND,
    HVML_STATE_NUMERIC_CHARACTER_REFERENCE,
    HVML_STATE_HEXADECIMAL_CHARACTER_REFERENCE_START,
    HVML_STATE_DECIMAL_CHARACTER_REFERENCE_START,
    HVML_STATE_HEXADECIMAL_CHARACTER_REFERENCE,
    HVML_STATE_DECIMAL_CHARACTER_REFERENCE,
    HVML_STATE_NUMERIC_CHARACTER_REFERENCE_END,
    HVML_STATE_SPECIAL_ATTRIBUTE_OPERATOR_IN_ATTRIBUTE_NAME,
    HVML_STATE_SPECIAL_ATTRIBUTE_OPERATOR_AFTER_ATTRIBUTE_NAME,
    HVML_STATE_TEXT_CONTENT,
    HVML_STATE_JSONEE_ATTRIBUTE_VALUE_DOUBLE_QUOTED,
    HVML_STATE_JSONEE_ATTRIBUTE_VALUE_SINGLE_QUOTED,
    HVML_STATE_JSONEE_ATTRIBUTE_VALUE_UNQUOTED,
    HVML_STATE_JSONTEXT_CONTENT,
    HVML_STATE_EJSON_DATA,
    HVML_STATE_EJSON_FINISHED,
    HVML_STATE_EJSON_CONTROL,
    HVML_STATE_EJSON_LEFT_BRACE,
    HVML_STATE_EJSON_RIGHT_BRACE,
    HVML_STATE_EJSON_LEFT_BRACKET,
    HVML_STATE_EJSON_RIGHT_BRACKET,
    HVML_STATE_EJSON_LEFT_PARENTHESIS,
    HVML_STATE_EJSON_RIGHT_PARENTHESIS,
    HVML_STATE_EJSON_DOLLAR,
    HVML_STATE_EJSON_AFTER_VALUE,
    HVML_STATE_EJSON_BEFORE_NAME,
    HVML_STATE_EJSON_AFTER_NAME,
    HVML_STATE_EJSON_NAME_UNQUOTED,
    HVML_STATE_EJSON_NAME_SINGLE_QUOTED,
    HVML_STATE_EJSON_NAME_DOUBLE_QUOTED,
    HVML_STATE_EJSON_VALUE_SINGLE_QUOTED,
    HVML_STATE_EJSON_VALUE_DOUBLE_QUOTED,
    HVML_STATE_EJSON_AFTER_VALUE_DOUBLE_QUOTED,
    HVML_STATE_EJSON_VALUE_TWO_DOUBLE_QUOTED,
    HVML_STATE_EJSON_VALUE_THREE_DOUBLE_QUOTED,
    HVML_STATE_EJSON_KEYWORD,
    HVML_STATE_EJSON_AFTER_KEYWORD,
    HVML_STATE_EJSON_BYTE_SEQUENCE,
    HVML_STATE_EJSON_AFTER_BYTE_SEQUENCE,
    HVML_STATE_EJSON_HEX_BYTE_SEQUENCE,
    HVML_STATE_EJSON_BINARY_BYTE_SEQUENCE,
    HVML_STATE_EJSON_BASE64_BYTE_SEQUENCE,
    HVML_STATE_EJSON_VALUE_NUMBER,
    HVML_STATE_EJSON_AFTER_VALUE_NUMBER,
    HVML_STATE_EJSON_VALUE_NUMBER_INTEGER,
    HVML_STATE_EJSON_VALUE_NUMBER_FRACTION,
    HVML_STATE_EJSON_VALUE_NUMBER_EXPONENT,
    HVML_STATE_EJSON_VALUE_NUMBER_EXPONENT_INTEGER,
    HVML_STATE_EJSON_VALUE_NUMBER_SUFFIX_INTEGER,
    HVML_STATE_EJSON_VALUE_NUMBER_INFINITY,
    HVML_STATE_EJSON_VALUE_NAN,
    HVML_STATE_EJSON_STRING_ESCAPE,
    HVML_STATE_EJSON_STRING_ESCAPE_FOUR_HEXADECIMAL_DIGITS,
    HVML_STATE_EJSON_JSONEE_VARIABLE,
    HVML_STATE_EJSON_JSONEE_FULL_STOP_SIGN,
    HVML_STATE_EJSON_JSONEE_KEYWORD,
    HVML_STATE_EJSON_JSONEE_STRING,
    HVML_STATE_EJSON_AFTER_JSONEE_STRING,
    HVML_STATE_EJSON_TEMPLATE_DATA,
    HVML_STATE_EJSON_TEMPLATE_DATA_LESS_THAN_SIGN,
    HVML_STATE_EJSON_TEMPLATE_DATA_END_TAG_OPEN,
    HVML_STATE_EJSON_TEMPLATE_DATA_END_TAG_NAME,
    HVML_STATE_EJSON_TEMPLATE_FINISHED,
};

/* Make sure the number of error messages matches the number of error codes */
#define _COMPILE_TIME_ASSERT(name, x)               \
       typedef int _dummy_ ## name[(x) * 2 - 1]

_COMPILE_TIME_ASSERT(msgs,
        PCA_TABLESIZE(hvml_err_msgs) == PCHVML_ERROR_NR);

_COMPILE_TIME_ASSERT(states,
        PCA_TABLESIZE(state_names) == PCHVML_STATE_NR);

#undef _COMPILE_TIME_ASSERT

static struct err_msg_seg _hvml_err_msgs_seg = {
    { NULL, NULL },
    PURC_ERROR_FIRST_HVML,
    PURC_ERROR_FIRST_HVML + PCA_TABLESIZE(hvml_err_msgs) - 1,
    hvml_err_msgs
};

void pchvml_init_once(void)
{
    pcinst_register_error_message_segment(&_hvml_err_msgs_seg);
}

struct pchvml_parser* pchvml_create(uint32_t flags, size_t queue_size)
{
    UNUSED_PARAM(flags);
    UNUSED_PARAM(queue_size);

    struct pchvml_parser* parser = (struct pchvml_parser*) PCHVML_ALLOC(
            sizeof(struct pchvml_parser));
    parser->state = PCHVML_DATA_STATE;
    parser->rwswrap = pchvml_rwswrap_new ();
    parser->temp_buffer = pchvml_buffer_new ();
    parser->tag_name = pchvml_buffer_new ();
    parser->string_buffer = pchvml_buffer_new ();
    parser->quoted_buffer = pchvml_buffer_new ();
    parser->vcm_stack = pcvcm_stack_new();
    parser->ejson_stack = pcutils_stack_new(0);
    parser->tag_is_operation = false;
    return parser;
}

void pchvml_reset(struct pchvml_parser* parser, uint32_t flags,
        size_t queue_size)
{
    UNUSED_PARAM(flags);
    UNUSED_PARAM(queue_size);

    parser->state = PCHVML_DATA_STATE;
    pchvml_rwswrap_destroy (parser->rwswrap);
    parser->rwswrap = pchvml_rwswrap_new ();
    pchvml_buffer_reset (parser->temp_buffer);
    pchvml_buffer_reset (parser->tag_name);
    pchvml_buffer_reset (parser->string_buffer);
    pchvml_buffer_reset (parser->quoted_buffer);

    struct pcvcm_node* n = parser->vcm_node;
    parser->vcm_node = NULL;
    while (!pcvcm_stack_is_empty(parser->vcm_stack)) {
        struct pcvcm_node* node = pcvcm_stack_pop(parser->vcm_stack);
        pctree_node_append_child(
                (struct pctree_node*)node, (struct pctree_node*)n);
        n = node;
    }
    pcvcm_node_destroy(n);
    pcvcm_stack_destroy(parser->vcm_stack);
    parser->vcm_stack = pcvcm_stack_new();
    pcutils_stack_destroy(parser->ejson_stack);
    parser->ejson_stack = pcutils_stack_new(0);
    if (parser->token) {
        pchvml_token_destroy(parser->token);
        parser->token = NULL;
    }
}

void pchvml_destroy(struct pchvml_parser* parser)
{
    if (parser) {
        pchvml_rwswrap_destroy (parser->rwswrap);
        pchvml_buffer_destroy (parser->temp_buffer);
        pchvml_buffer_destroy (parser->tag_name);
        pchvml_buffer_destroy (parser->string_buffer);
        pchvml_buffer_destroy (parser->quoted_buffer);
        if (parser->sbst) {
            pchvml_sbst_destroy(parser->sbst);
        }
        struct pcvcm_node* n = parser->vcm_node;
        parser->vcm_node = NULL;
        while (!pcvcm_stack_is_empty(parser->vcm_stack)) {
            struct pcvcm_node* node = pcvcm_stack_pop(parser->vcm_stack);
            pctree_node_append_child(
                    (struct pctree_node*)node, (struct pctree_node*)n);
            n = node;
        }
        pcvcm_node_destroy(n);
        pcvcm_stack_destroy(parser->vcm_stack);
        pcutils_stack_destroy(parser->ejson_stack);
        if (parser->token) {
            pchvml_token_destroy(parser->token);
        }
        PCHVML_FREE(parser);
    }
}

const char* pchvml_get_state_name(enum pchvml_state state)
{
    PC_ASSERT(state >= 0 && state < PCHVML_STATE_NR);
    return state_names[state];
}

#define ERROR_NAME(state_name)                                              \
    case state_name:                                                        \
        return ""#state_name;                                               \

const char* pchvml_get_error_name(int err)
{
    switch (err) {
    ERROR_NAME(PCHVML_SUCCESS)
    ERROR_NAME(PCHVML_ERROR_UNEXPECTED_NULL_CHARACTER)
    ERROR_NAME(PCHVML_ERROR_UNEXPECTED_QUESTION_MARK_INSTEAD_OF_TAG_NAME)
    ERROR_NAME(PCHVML_ERROR_EOF_BEFORE_TAG_NAME)
    ERROR_NAME(PCHVML_ERROR_MISSING_END_TAG_NAME)
    ERROR_NAME(PCHVML_ERROR_INVALID_FIRST_CHARACTER_OF_TAG_NAME)
    ERROR_NAME(PCHVML_ERROR_EOF_IN_TAG)
    ERROR_NAME(PCHVML_ERROR_UNEXPECTED_EQUALS_SIGN_BEFORE_ATTRIBUTE_NAME)
    ERROR_NAME(PCHVML_ERROR_UNEXPECTED_CHARACTER_IN_ATTRIBUTE_NAME)
    ERROR_NAME(PCHVML_ERROR_UNEXPECTED_CHARACTER_IN_UNQUOTED_ATTRIBUTE_VALUE)
    ERROR_NAME(PCHVML_ERROR_MISSING_WHITESPACE_BETWEEN_ATTRIBUTES)
    ERROR_NAME(PCHVML_ERROR_UNEXPECTED_SOLIDUS_IN_TAG)
    ERROR_NAME(PCHVML_ERROR_CDATA_IN_HTML_CONTENT)
    ERROR_NAME(PCHVML_ERROR_INCORRECTLY_OPENED_COMMENT)
    ERROR_NAME(PCHVML_ERROR_ABRUPT_CLOSING_OF_EMPTY_COMMENT)
    ERROR_NAME(PCHVML_ERROR_EOF_IN_COMMENT)
    ERROR_NAME(PCHVML_ERROR_EOF_IN_DOCTYPE)
    ERROR_NAME(PCHVML_ERROR_MISSING_WHITESPACE_BEFORE_DOCTYPE_NAME)
    ERROR_NAME(PCHVML_ERROR_MISSING_DOCTYPE_NAME)
    ERROR_NAME(PCHVML_ERROR_INVALID_CHARACTER_SEQUENCE_AFTER_DOCTYPE_NAME)
    ERROR_NAME(PCHVML_ERROR_MISSING_WHITESPACE_AFTER_DOCTYPE_PUBLIC_KEYWORD)
    ERROR_NAME(PCHVML_ERROR_MISSING_DOCTYPE_PUBLIC_ID)
    ERROR_NAME(PCHVML_ERROR_MISSING_QUOTE_BEFORE_DOCTYPE_PUBLIC_ID)
    ERROR_NAME(PCHVML_ERROR_ABRUPT_DOCTYPE_PUBLIC_ID)
    ERROR_NAME(PCHVML_ERROR_MISSING_WHITESPACE_BETWEEN_DOCTYPE_PUB_AND_SYS)
    ERROR_NAME(PCHVML_ERROR_MISSING_WHITESPACE_AFTER_DOCTYPE_SYSTEM_KEYWORD)
    ERROR_NAME(PCHVML_ERROR_MISSING_DOCTYPE_SYSTEM)
    ERROR_NAME(PCHVML_ERROR_ABRUPT_DOCTYPE_SYSTEM)
    ERROR_NAME(PCHVML_ERROR_UNEXPECTED_CHARACTER_AFTER_DOCTYPE_SYSTEM)
    ERROR_NAME(PCHVML_ERROR_EOF_IN_CDATA)
    ERROR_NAME(PCHVML_ERROR_UNKNOWN_NAMED_CHARACTER_REFERENCE)
    ERROR_NAME(PCHVML_ERROR_ABSENCE_OF_DIGITS_IN_NUMERIC_CHARACTER_REFERENCE)
    ERROR_NAME(PCHVML_ERROR_UNEXPECTED_CHARACTER)
    ERROR_NAME(PCHVML_ERROR_UNEXPECTED_JSON_NUMBER_EXPONENT)
    ERROR_NAME(PCHVML_ERROR_UNEXPECTED_JSON_NUMBER_FRACTION)
    ERROR_NAME(PCHVML_ERROR_UNEXPECTED_JSON_NUMBER_INTEGER)
    ERROR_NAME(PCHVML_ERROR_UNEXPECTED_JSON_NUMBER)
    ERROR_NAME(PCHVML_ERROR_UNEXPECTED_RIGHT_BRACE)
    ERROR_NAME(PCHVML_ERROR_UNEXPECTED_RIGHT_BRACKET)
    ERROR_NAME(PCHVML_ERROR_UNEXPECTED_JSON_KEY_NAME)
    ERROR_NAME(PCHVML_ERROR_UNEXPECTED_COMMA)
    ERROR_NAME(PCHVML_ERROR_UNEXPECTED_JSON_KEYWORD)
    ERROR_NAME(PCHVML_ERROR_UNEXPECTED_BASE64)
    ERROR_NAME(PCHVML_ERROR_BAD_JSON_NUMBER)
    ERROR_NAME(PCHVML_ERROR_BAD_JSON_STRING_ESCAPE_ENTITY)
    ERROR_NAME(PCHVML_ERROR_BAD_JSONEE)
    ERROR_NAME(PCHVML_ERROR_BAD_JSONEE_ESCAPE_ENTITY)
    ERROR_NAME(PCHVML_ERROR_BAD_JSONEE_VARIABLE_NAME)
    ERROR_NAME(PCHVML_ERROR_EMPTY_JSONEE_NAME)
    ERROR_NAME(PCHVML_ERROR_BAD_JSONEE_NAME)
    ERROR_NAME(PCHVML_ERROR_BAD_JSONEE_KEYWORD)
    ERROR_NAME(PCHVML_ERROR_EMPTY_JSONEE_KEYWORD)
    ERROR_NAME(PCHVML_ERROR_BAD_JSONEE_UNEXPECTED_COMMA)
    ERROR_NAME(PCHVML_ERROR_BAD_JSONEE_UNEXPECTED_PARENTHESIS)
    ERROR_NAME(PCHVML_ERROR_BAD_JSONEE_UNEXPECTED_LEFT_ANGLE_BRACKET)
    ERROR_NAME(PCHVML_ERROR_MISSING_MISSING_ATTRIBUTE_VALUE)
    ERROR_NAME(PCHVML_ERROR_NESTED_COMMENT)
    ERROR_NAME(PCHVML_ERROR_INCORRECTLY_CLOSED_COMMENT)
    ERROR_NAME(PCHVML_ERROR_MISSING_QUOTE_BEFORE_DOCTYPE_SYSTEM)
    ERROR_NAME(PCHVML_ERROR_MISSING_SEMICOLON_AFTER_CHARACTER_REFERENCE)
    ERROR_NAME(PCHVML_ERROR_CHARACTER_REFERENCE_OUTSIDE_UNICODE_RANGE)
    ERROR_NAME(PCHVML_ERROR_SURROGATE_CHARACTER_REFERENCE)
    ERROR_NAME(PCHVML_ERROR_NONCHARACTER_CHARACTER_REFERENCE)
    ERROR_NAME(PCHVML_ERROR_NULL_CHARACTER_REFERENCE)
    ERROR_NAME(PCHVML_ERROR_CONTROL_CHARACTER_REFERENCE)
    ERROR_NAME(PCHVML_ERROR_INVALID_UTF8_CHARACTER)
    }
    return NULL;
}
