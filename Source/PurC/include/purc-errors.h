/**
 * @file purc-errors.h
 * @author Vincent Wei (https://github.com/VincentWei)
 * @date 2021/07/02
 * @brief The error codes of PurC.
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

#ifndef PURC_PURC_ERRORS_H
#define PURC_PURC_ERRORS_H

#include <stdbool.h>
#include <stddef.h>

#include "purc-macros.h"
#include "purc-utils.h"
#include "purc-variant.h"

#define PURC_EXCEPT_BAD_ENCODING         "BadEncoding"
#define PURC_EXCEPT_BAD_HVML_TAG         "BadHVMLTag"
#define PURC_EXCEPT_BAD_HVML_ATTR_NAME   "BadHVMLAttrName"
#define PURC_EXCEPT_BAD_HVML_ATTR_VALUE  "BadHVMLAttrValue"
#define PURC_EXCEPT_BAD_HVML_CONTENT     "BadHVMLContent"
#define PURC_EXCEPT_BAD_TARGET_HTML      "BadTargetHTML"
#define PURC_EXCEPT_BAD_TARGET_XGML      "BadTargetXGML"
#define PURC_EXCEPT_BAD_TARGET_XML       "BadTargetXML"
#define PURC_EXCEPT_BAD_EXPRESSION       "BadExpression"
#define PURC_EXCEPT_BAD_EXECUTOR         "BadExecutor"
#define PURC_EXCEPT_BAD_NAME             "BadName"
#define PURC_EXCEPT_NO_DATA              "NoData"
#define PURC_EXCEPT_NOT_ITERABLE         "NotIterable"
#define PURC_EXCEPT_BAD_INDEX            "BadIndex"
#define PURC_EXCEPT_NO_SUCH_KEY          "NoSuchKey"
#define PURC_EXCEPT_DUPLICATE_KEY        "DuplicateKey"
#define PURC_EXCEPT_ARGUMENT_MISSED      "ArgumentMissed"
#define PURC_EXCEPT_WRONG_DATA_TYPE      "WrongDataType"
#define PURC_EXCEPT_INVALID_VALUE        "InvalidValue"
#define PURC_EXCEPT_MAX_ITERATION_COUNT  "MaxIterationCount"
#define PURC_EXCEPT_MAX_RECURSION_DEPTH  "MaxRecursionDepth"
#define PURC_EXCEPT_UNAUTHORIZED         "Unauthorized"
#define PURC_EXCEPT_TIMEOUT              "Timeout"
#define PURC_EXCEPT_E_DOM_FAILURE        "eDOMFailure"
#define PURC_EXCEPT_LOST_RENDERER        "LostRenderer"
#define PURC_EXCEPT_MEMORY_FAILURE       "MemoryFailure"
#define PURC_EXCEPT_INTERNAL_FAILURE     "InternalFailure"
#define PURC_EXCEPT_ZERO_DIVISION        "ZeroDivision"
#define PURC_EXCEPT_OVERFLOW             "Overflow"
#define PURC_EXCEPT_UNDERFLOW            "Underflow"
#define PURC_EXCEPT_INVALID_FLOAT        "InvalidFloat"
#define PURC_EXCEPT_ACCESS_DENIED        "AccessDenied"
#define PURC_EXCEPT_IO_FAILURE           "IOFailure"
#define PURC_EXCEPT_TOO_SMALL            "TooSmall"
#define PURC_EXCEPT_TOO_MANY             "TooMany"
#define PURC_EXCEPT_TOO_LONG             "TooLong"
#define PURC_EXCEPT_TOO_LARGE            "TooLarge"
#define PURC_EXCEPT_NOT_DESIRED_ENTITY   "NotDesiredEntity"
#define PURC_EXCEPT_ENTITY_NOT_FOUND     "EntityNotFound"
#define PURC_EXCEPT_ENTITY_EXISTS        "EntityExists"
#define PURC_EXCEPT_NO_STORAGE_SPACE     "NoStorageSpace"
#define PURC_EXCEPT_BROKEN_PIPE          "BrokenPipe"
#define PURC_EXCEPT_CONNECTION_ABORTED   "ConnectionAborted"
#define PURC_EXCEPT_CONNECTION_REFUSED   "ConnectionRefused"
#define PURC_EXCEPT_CONNECTION_RESET     "ConnectionReset"
#define PURC_EXCEPT_OS_FAILURE           "OSFailure"
#define PURC_EXCEPT_NOT_IMPLEMENTED      "NotImplemented"
#define PURC_EXCEPT_NOT_READY            "NotReady"

#define PURC_EXCEPT_FLAGS_NONE          0x0000
#define PURC_EXCEPT_FLAGS_REQUIRED      0x0001

// the first error codes for modules:
#define PURC_ERROR_FIRST_GENERIC        0
#define PURC_ERROR_FIRST_VARIANT        100
#define PURC_ERROR_FIRST_RWSTREAM       200

#define PURC_ERROR_FIRST_EJSON          1100
#define PURC_ERROR_FIRST_HVML           1200
#define PURC_ERROR_FIRST_HTML           1300
#define PURC_ERROR_FIRST_XGML           1400
#define PURC_ERROR_FIRST_XML            1500

#define PURC_ERROR_FIRST_VDOM           2100
#define PURC_ERROR_FIRST_EDOM           2200
#define PURC_ERROR_FIRST_VCM            2300

#define PURC_ERROR_FIRST_EXECUTOR       2400

enum {
    PURC_ERROR_FIRST = PURC_ERROR_FIRST_GENERIC,

    PURC_ERROR_OK = PURC_ERROR_FIRST,
    PURC_ERROR_BAD_SYSTEM_CALL,
    PURC_ERROR_BAD_STDC_CALL,
    PURC_ERROR_OUT_OF_MEMORY,
    PURC_ERROR_INVALID_VALUE,
    PURC_ERROR_DUPLICATED,
    PURC_ERROR_NOT_IMPLEMENTED,
    PURC_ERROR_NO_INSTANCE,
    PURC_ERROR_TOO_LARGE_ENTITY,
    PURC_ERROR_BAD_ENCODING,
    PURC_ERROR_NOT_SUPPORTED,
    PURC_ERROR_OUTPUT,
    PURC_ERROR_TOO_SMALL_BUFF,
    PURC_ERROR_TOO_SMALL_SIZE,
    PURC_ERROR_NULL_OBJECT,
    PURC_ERROR_INCOMPLETE_OBJECT,
    PURC_ERROR_NO_FREE_SLOT,
    PURC_ERROR_NOT_EXISTS,
    PURC_ERROR_ARGUMENT_MISSED,
    PURC_ERROR_WRONG_DATA_TYPE,
    PURC_ERROR_WRONG_STAGE,
    PURC_ERROR_OVERFLOW,
    PURC_ERROR_UNDERFLOW,
    PURC_ERROR_DIVBYZERO,
    PURC_ERROR_INVALID_FLOAT,

    /* XXX: change this when you append a new error code */
    PURC_ERROR_LAST = PURC_ERROR_INVALID_FLOAT,
};

#define PURC_ERROR_NR       (PURC_ERROR_LAST - PURC_ERROR_FIRST + 1)

// error codes for variant go here
enum {
    PCVARIANT_SUCCESS               = PURC_ERROR_OK,
    PCVARIANT_ERROR_FIRST           = PURC_ERROR_FIRST_VARIANT,
    PCVARIANT_ERROR_NOT_SUPPORTED   = PURC_ERROR_NOT_SUPPORTED,
    PCVARIANT_ERROR_OUT_OF_MEMORY   = PURC_ERROR_OUT_OF_MEMORY,
    PCVARIANT_ERROR_WRONG_ARGS      = PURC_ERROR_ARGUMENT_MISSED,
    PCVARIANT_ERROR_DUPLICATED      = PURC_ERROR_DUPLICATED,

    PCVARIANT_ERROR_INVALID_TYPE    = PCVARIANT_ERROR_FIRST,
    PCVARIANT_ERROR_OUT_OF_BOUNDS,
    PCVARIANT_ERROR_NOT_FOUND,

    /* XXX: change this when you append a new error code */
    PCVARIANT_ERROR_LAST            = PCVARIANT_ERROR_NOT_FOUND,
};

#define PCVARIANT_ERROR_NR \
    (PCVARIANT_ERROR_LAST - PCVARIANT_ERROR_FIRST + 1)

// error codes for rwstream go here
enum {
    PCRWSTREAM_SUCCESS              = PURC_ERROR_OK,
    PCRWSTREAM_ERROR_FIRST          = PURC_ERROR_FIRST_RWSTREAM,

    PCRWSTREAM_ERROR_FAILED         = PCRWSTREAM_ERROR_FIRST,
    PCRWSTREAM_ERROR_FILE_TOO_BIG,
    PCRWSTREAM_ERROR_IO,
    PCRWSTREAM_ERROR_IS_DIR,
    PCRWSTREAM_ERROR_NO_SPACE,
    PCRWSTREAM_ERROR_NO_DEVICE_OR_ADDRESS,
    PCRWSTREAM_ERROR_OVERFLOW,
    PCRWSTREAM_ERROR_PIPE,

    /* XXX: change this when you append a new error code */
    PCRWSTREAM_ERROR_LAST           = PCRWSTREAM_ERROR_PIPE,
};

#define PCRWSTREAM_ERROR_NR \
    (PCRWSTREAM_ERROR_LAST - PCRWSTREAM_ERROR_FIRST + 1)

// error codes for ejson go here
enum {
    PCEJSON_SUCCESS                 = PURC_ERROR_OK,
    PCEJSON_ERROR_FIRST             = PURC_ERROR_FIRST_EJSON,

    PCEJSON_ERROR_UNEXPECTED_CHARACTER = PCEJSON_ERROR_FIRST,
    PCEJSON_ERROR_UNEXPECTED_NULL_CHARACTER,
    PCEJSON_ERROR_UNEXPECTED_JSON_NUMBER_EXPONENT,
    PCEJSON_ERROR_UNEXPECTED_JSON_NUMBER_FRACTION,
    PCEJSON_ERROR_UNEXPECTED_JSON_NUMBER_INTEGER,
    PCEJSON_ERROR_UNEXPECTED_JSON_NUMBER,
    PCEJSON_ERROR_UNEXPECTED_RIGHT_BRACE,
    PCEJSON_ERROR_UNEXPECTED_RIGHT_BRACKET,
    PCEJSON_ERROR_UNEXPECTED_JSON_KEY_NAME,
    PCEJSON_ERROR_UNEXPECTED_COMMA,
    PCEJSON_ERROR_UNEXPECTED_JSON_KEYWORD,
    PCEJSON_ERROR_UNEXPECTED_BASE64,
    PCEJSON_ERROR_UNEXPECTED_EOF,
    PCEJSON_ERROR_BAD_JSON_NUMBER,
    PCEJSON_ERROR_BAD_JSON_STRING_ESCAPE_ENTITY,
    PCEJSON_ERROR_BAD_JSON,
    PCEJSON_ERROR_MAX_DEPTH_EXCEEDED,

    /* XXX: change this when you append a new error code */
    PCEJSON_ERROR_LAST              = PCEJSON_ERROR_MAX_DEPTH_EXCEEDED,
};

#define PCEJSON_ERROR_NR \
    (PCEJSON_ERROR_LAST - PCEJSON_ERROR_FIRST + 1)

// error codes for hvml go here
enum {
    PCHVML_SUCCESS          = PURC_ERROR_OK,
    PCHVML_ERROR_FIRST      = PURC_ERROR_FIRST_HVML,

    PCHVML_ERROR_UNEXPECTED_NULL_CHARACTER = PCHVML_ERROR_FIRST,
    PCHVML_ERROR_UNEXPECTED_QUESTION_MARK_INSTEAD_OF_TAG_NAME,
    PCHVML_ERROR_EOF_BEFORE_TAG_NAME,
    PCHVML_ERROR_MISSING_END_TAG_NAME,
    PCHVML_ERROR_INVALID_FIRST_CHARACTER_OF_TAG_NAME,
    PCHVML_ERROR_EOF_IN_TAG,
    PCHVML_ERROR_UNEXPECTED_EQUALS_SIGN_BEFORE_ATTRIBUTE_NAME,
    PCHVML_ERROR_UNEXPECTED_CHARACTER_IN_ATTRIBUTE_NAME,
    PCHVML_ERROR_UNEXPECTED_CHARACTER_IN_UNQUOTED_ATTRIBUTE_VALUE,
    PCHVML_ERROR_MISSING_WHITESPACE_BETWEEN_ATTRIBUTES,
    PCHVML_ERROR_UNEXPECTED_SOLIDUS_IN_TAG,
    PCHVML_ERROR_CDATA_IN_HTML_CONTENT,
    PCHVML_ERROR_INCORRECTLY_OPENED_COMMENT,
    PCHVML_ERROR_ABRUPT_CLOSING_OF_EMPTY_COMMENT,
    PCHVML_ERROR_EOF_IN_COMMENT,
    PCHVML_ERROR_EOF_IN_DOCTYPE,
    PCHVML_ERROR_MISSING_WHITESPACE_BEFORE_DOCTYPE_NAME,
    PCHVML_ERROR_MISSING_DOCTYPE_NAME,
    PCHVML_ERROR_INVALID_CHARACTER_SEQUENCE_AFTER_DOCTYPE_NAME,
    PCHVML_ERROR_MISSING_WHITESPACE_AFTER_DOCTYPE_PUBLIC_KEYWORD,
    PCHVML_ERROR_MISSING_DOCTYPE_PUBLIC_ID,
    PCHVML_ERROR_MISSING_QUOTE_BEFORE_DOCTYPE_PUBLIC_ID,
    PCHVML_ERROR_ABRUPT_DOCTYPE_PUBLIC_ID,
    PCHVML_ERROR_MISSING_WHITESPACE_BETWEEN_DOCTYPE_PUB_AND_SYS,
    PCHVML_ERROR_MISSING_WHITESPACE_AFTER_DOCTYPE_SYSTEM_KEYWORD,
    PCHVML_ERROR_MISSING_DOCTYPE_SYSTEM,
    PCHVML_ERROR_ABRUPT_DOCTYPE_SYSTEM,
    PCHVML_ERROR_UNEXPECTED_CHARACTER_AFTER_DOCTYPE_SYSTEM,
    PCHVML_ERROR_EOF_IN_CDATA,
    PCHVML_ERROR_UNKNOWN_NAMED_CHARACTER_REFERENCE,
    PCHVML_ERROR_ABSENCE_OF_DIGITS_IN_NUMERIC_CHARACTER_REFERENCE,
    PCHVML_ERROR_UNEXPECTED_CHARACTER,
    PCHVML_ERROR_UNEXPECTED_JSON_NUMBER_EXPONENT,
    PCHVML_ERROR_UNEXPECTED_JSON_NUMBER_FRACTION,
    PCHVML_ERROR_UNEXPECTED_JSON_NUMBER_INTEGER,
    PCHVML_ERROR_UNEXPECTED_JSON_NUMBER,
    PCHVML_ERROR_UNEXPECTED_RIGHT_BRACE,
    PCHVML_ERROR_UNEXPECTED_RIGHT_BRACKET,
    PCHVML_ERROR_UNEXPECTED_JSON_KEY_NAME,
    PCHVML_ERROR_UNEXPECTED_COMMA,
    PCHVML_ERROR_UNEXPECTED_JSON_KEYWORD,
    PCHVML_ERROR_UNEXPECTED_BASE64,
    PCHVML_ERROR_BAD_JSON_NUMBER,
    PCHVML_ERROR_BAD_JSON_STRING_ESCAPE_ENTITY,
    PCHVML_ERROR_BAD_JSONEE,
    PCHVML_ERROR_BAD_JSONEE_ESCAPE_ENTITY,
    PCHVML_ERROR_BAD_JSONEE_VARIABLE_NAME,
    PCHVML_ERROR_EMPTY_JSONEE_NAME,
    PCHVML_ERROR_BAD_JSONEE_NAME,
    PCHVML_ERROR_BAD_JSONEE_KEYWORD,
    PCHVML_ERROR_EMPTY_JSONEE_KEYWORD,
    PCHVML_ERROR_BAD_JSONEE_UNEXPECTED_COMMA,
    PCHVML_ERROR_BAD_JSONEE_UNEXPECTED_PARENTHESIS,
    PCHVML_ERROR_BAD_JSONEE_UNEXPECTED_LEFT_ANGLE_BRACKET,
    PCHVML_ERROR_MISSING_MISSING_ATTRIBUTE_VALUE,
    PCHVML_ERROR_NESTED_COMMENT,
    PCHVML_ERROR_INCORRECTLY_CLOSED_COMMENT,
    PCHVML_ERROR_MISSING_QUOTE_BEFORE_DOCTYPE_SYSTEM,
    PCHVML_ERROR_MISSING_SEMICOLON_AFTER_CHARACTER_REFERENCE,
    PCHVML_ERROR_CHARACTER_REFERENCE_OUTSIDE_UNICODE_RANGE,
    PCHVML_ERROR_SURROGATE_CHARACTER_REFERENCE,
    PCHVML_ERROR_NONCHARACTER_CHARACTER_REFERENCE,
    PCHVML_ERROR_NULL_CHARACTER_REFERENCE,
    PCHVML_ERROR_CONTROL_CHARACTER_REFERENCE,
    PCHVML_ERROR_INVALID_UTF8_CHARACTER,

    /* XXX: change this when you append a new error code */
    PCHVML_ERROR_LAST               = PCHVML_ERROR_INVALID_UTF8_CHARACTER,
};

#define PCHVML_ERROR_NR \
    (PCHVML_ERROR_LAST - PCHVML_ERROR_FIRST + 1)

enum pcexecutor_error
{
    PCEXECUTOR_SUCCESS               = PURC_ERROR_OK,
    PCEXECUTOR_ERROR_FIRST           = PURC_ERROR_FIRST_EXECUTOR,

    PCEXECUTOR_ERROR_OOM             = PURC_ERROR_OUT_OF_MEMORY,
    PCEXECUTOR_ERROR_BAD_ARG         = PURC_ERROR_INVALID_VALUE,
    PCEXECUTOR_ERROR_ALREAD_EXISTS   = PURC_ERROR_DUPLICATED,
    PCEXECUTOR_ERROR_NOT_EXISTS      = PURC_ERROR_NOT_EXISTS,
    PCEXECUTOR_ERROR_NOT_FOUND       = PCVARIANT_ERROR_NOT_FOUND,
    PCEXECUTOR_ERROR_NOT_IMPLEMENTED = PURC_ERROR_NOT_IMPLEMENTED,

    PCEXECUTOR_ERROR_NO_KEYS_SELECTED = PCEXECUTOR_ERROR_FIRST,
    PCEXECUTOR_ERROR_NOT_ALLOWED,
    PCEXECUTOR_ERROR_OUT_OF_RANGE,
    PCEXECUTOR_ERROR_BAD_SYNTAX,

    /* XXX: change this when you append a new error code */
    PCEXECUTOR_ERROR_LAST            = PCEXECUTOR_ERROR_BAD_SYNTAX,
};

#define PCEXECUTOR_ERROR_NR \
    (PCEXECUTOR_ERROR_LAST - PCEXECUTOR_ERROR_FIRST + 1)

PCA_EXTERN_C_BEGIN

/**
 * purc_get_last_error:
 *
 * Returns: The last error code.
 */
PCA_EXPORT int
purc_get_last_error (void);

/**
 * purc_set_error_ex:
 *
 * Returns: PURC_ERROR_OK or PURC_ERROR_NO_INSTANCE.
 */
PCA_EXPORT int
purc_set_error_ex(int err_code, purc_variant_t exinfo);

/**
 * purc_set_error:
 *
 * Returns: PURC_ERROR_OK or PURC_ERROR_NO_INSTANCE.
 */
static inline int
purc_set_error(int err_code)
{
    return purc_set_error_ex(err_code, PURC_VARIANT_INVALID);
}

/**
 * purc_get_error_message:
 *
 * @errcode: the error code.
 *
 * Returns: The message for the specified error code.
 */
PCA_EXPORT const char*
purc_get_error_message (int errcode);

/**
 * purc_get_error_exceptions:
 *
 * @errcode: the error code.
 *
 * Returns: The exception atom string for the specified error code.
 */
PCA_EXPORT purc_atom_t
purc_get_error_exception (int errcode);

PCA_EXTERN_C_END

#endif /* not defined PURC_PURC_ERRORS_H */

