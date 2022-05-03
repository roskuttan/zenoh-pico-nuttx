/*
 * Copyright (c) 2017, 2021 ADLINK Technology Inc.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0, or the Apache License, Version 2.0
 * which is available at https://www.apache.org/licenses/LICENSE-2.0.
 *
 * SPDX-License-Identifier: EPL-2.0 OR Apache-2.0
 *
 * Contributors:
 *     ADLINK zenoh team, <zenoh@adlink-labs.tech>
 */

#include <string.h>
#include "zenoh-pico/collections/string.h"

/*-------- string --------*/
_z_string_t z_string_make(const _z_str_t value)
{
    _z_string_t s;
    s.val = _z_str_clone(value);
    s.len = strlen(value);
    return s;
}

void _z_string_append(_z_string_t *dst, const _z_string_t *src)
{
    dst->len += src->len;
    dst->val = (char *)realloc(dst->val, dst->len);
    strcat(dst->val, src->val);
}

void _z_string_copy(_z_string_t *dst, const _z_string_t *src)
{
    if (src->val)
        dst->val = _z_str_clone(src->val);
    else
        dst->val = NULL;
    dst->len = src->len;
}

void _z_string_move(_z_string_t *dst, _z_string_t *src)
{
    dst->val = src->val;
    dst->len = src->len;

    src->val = NULL;
    src->len = 0;
}

void _z_string_move_str(_z_string_t *dst, _z_str_t src)
{
    dst->val = src;
    dst->len = strlen(src);
}

void _z_string_reset(_z_string_t *str)
{
    str->val = NULL;
    str->len = 0;
}

void _z_string_clear(_z_string_t *str)
{
    free((_z_str_t)str->val);
    _z_string_reset(str);
}

void _z_string_free(_z_string_t **str)
{
    _z_string_t *ptr = (_z_string_t *)*str;
    _z_string_clear(ptr);

    free(ptr);
    *str = NULL;
}

_z_string_t _z_string_from_bytes(_z_bytes_t *bs)
{
    _z_string_t s;
    s.len = 2 * bs->len;
    _z_str_t s_val = (_z_str_t)malloc(s.len * sizeof(char) + 1);

    char c[] = "0123456789ABCDEF";
    for (size_t i = 0; i < bs->len; i++)
    {
        s_val[2 * i] = c[(bs->start[i] & 0xF0) >> 4];
        s_val[2 * i + 1] = c[bs->start[i] & 0x0F];
    }
    s_val[s.len] = '\0';
    s.val = s_val;

    return s;
}

/*-------- str --------*/
size_t _z_str_size(const _z_str_t src)
{
    return strlen(src) + 1;
}

void _z_str_clear(_z_str_t *src)
{
    free(*src);
    src = NULL;
}

void _z_str_free(_z_str_t **src)
{
    _z_str_t *ptr = *src;
    _z_str_clear(ptr);
    *src = NULL;
}

void _z_str_copy(_z_str_t dst, const _z_str_t src)
{
    strcpy(dst, src);
}

_z_str_t _z_str_clone(const _z_str_t src)
{
    _z_str_t dst = (_z_str_t)malloc(_z_str_size(src));
    _z_str_copy(dst, src);
    return dst;
}

int _z_str_eq(const _z_str_t left, const _z_str_t right)
{
    return strcmp(left, right) == 0;
}
