/**
 * Copyright (c) 2012, Nurahmadie <nurahmadie@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <string.h>
#include <stdlib.h>

#include "tlv.h"

TLV *(tlvTable[TLV_IDX_MAX][TLV_IDX_MAX]);

#define TLV_ENTRY(tag) (tlvTable[(byte)tag[0]][(byte)tag[1]])

#define TAG_EQ(tag, cmp) ((tag[0] == *(cmp++)) && (tag[1] == *(cmp++)))

#define BIN_CAT(buf, src, len) \
do { \
    memcpy(buf, src, len); \
    buf += len; \
} while(0);

void tlv_initTable()
{
    int i, j;
    for (i = 0; i < TLV_IDX_MAX; i++)
        for (j = 0; j < TLV_IDX_MAX; j++)
            tlvTable[i][j] = NULL;
}

void tlv_free()
{
    int i, j;
    for (i = 0; i < TLV_IDX_MAX; i++)
        for (j = 0; j < TLV_IDX_MAX; j++)
            if (tlvTable[i][j])
            {
                if (tlvTable[i][j]->value)
                    free(tlvTable[i][j]->value);
                free(tlvTable[i][j]);
            }
}

TLV *tlv_new(int len, byte **src)
{
    TLV *node = malloc(sizeof(TLV));
    if (!node)
        return NULL;
    node->length = len;
    node->value = malloc(len);
    if (!(node->value)) {
        free(node);
        return NULL;
    }
    memcpy(node->value, *src, len);
    *src += len;
    return node;
}

void tlv_build(byte *rawStr, byte *tagList[])
{
    int i = 0;
    byte *tag = NULL;
    byte *cursor = rawStr;
    
    tlv_initTable();
    
    while (tag = tagList[i++]) {
        if (TAG_EQ(tag, cursor)) {
            TLV_ENTRY(tag) = tlv_new((byte)*(cursor++), &cursor);
        }
    }
}


int tlv_getValue(byte *tag, byte **value)
{
    if (!TLV_ENTRY(tag))
        return -1;
    *value = TLV_ENTRY(tag)->value;
    return TLV_ENTRY(tag)->length;
}

int tlv_insert(byte *tag, TLV *tlv)
{
    TLV_ENTRY(tag) = tlv;
    return tlv->length;
}

int tlv_setValue(byte *tag, int length, byte *value)
{
    if (!TLV_ENTRY(tag))
        return tlv_insert(tag, tlv_new(length, &value));
    if (TLV_ENTRY(tag)->value)
        free(TLV_ENTRY(tag)->value);

    TLV_ENTRY(tag)->value = malloc(length);
    if (!(TLV_ENTRY(tag)->value))
        return -1;
    memcpy(TLV_ENTRY(tag)->value, value, length);

    TLV_ENTRY(tag)->length = length;
    return length;
}

int tlv_dump(byte *tlvStr, byte *tagList[])
{
    int i = 0, length = 0;
    byte *tag;
    byte *cursor = tlvStr;
    while (tag = tagList[i++]) {
        if (TLV_ENTRY(tag)) {
            BIN_CAT(cursor, tag, TLV_TAG_LEN);
            *(cursor++) = (byte) TLV_ENTRY(tag)->length;
            BIN_CAT(cursor, TLV_ENTRY(tag)->value, TLV_ENTRY(tag)->length);
            length += TLV_TAG_LEN + 1 + TLV_ENTRY(tag)->length;
        }
    }
    return length;
}

