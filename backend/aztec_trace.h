/* aztec_trace.h - Trace routines for optimized AZTEC encodation algorithm */
/*
    libzint - the open source barcode library
    Copyright (C) 2026 Robin Stuart <rstuart114@gmail.com>

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

    1. Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
    3. Neither the name of the project nor the names of its contributors
       may be used to endorse or promote products derived from this software
       without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
    FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
    OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
    OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    SUCH DAMAGE.
 */
/* SPDX-License-Identifier: BSD-3-Clause */

#ifndef Z_AZTEC_TRACE_H
#define Z_AZTEC_TRACE_H

static void AZ_TRACE_EdgeToString(char *buf, const unsigned char *source, const int length, const char initial_mode,
			struct az_edge *edges, struct az_edge *edge) {
    struct az_edge *previous_edge = AZ_PREVIOUS(edges, edge);
    int previousMode = previous_edge ? previous_edge->mode : initial_mode;
	int previous = previous_edge ? (int) (previous_edge - edges) : 0;
	int current = (int) (edge - edges);
    (void)length;
    if (buf) {
        sprintf(buf, "%d_%c_%d %c(%d,%d) %d -> %d_%c_%d",
            edge->from, az_mode_char(previousMode), previous, az_mode_char(edge->mode), source[edge->from], edge->len,
			edge->size, edge->from + 1, az_mode_char(edge->mode), current);
    } else {
        printf("%d_%c_%d %c(%d,%d) %d -> %d_%c_%d",
            edge->from, az_mode_char(previousMode), previous, az_mode_char(edge->mode), source[edge->from], edge->len,
			edge->size, edge->from + 1, az_mode_char(edge->mode), current);
    }
}

static void AZ_TRACE_Path(const unsigned char *source, const int length, const char initial_mode,
			struct az_edge *edges, struct az_edge *edge, char *result, const int result_size) {
    struct az_edge *current;
    AZ_TRACE_EdgeToString(result, source, length, initial_mode, edges, edge);
    current = AZ_PREVIOUS(edges, edge);
    while (current) {
        char s[256];
        char *pos;
        int len;
        AZ_TRACE_EdgeToString(s, source, length, initial_mode, edges, current);
        pos = strrchr(s, ' ');
        assert(pos);
        len = strlen(result);
        if ((pos - s) + 1 + len + 1 >= result_size) {
            memcpy(result + result_size - 4, "...", 4); /* Include terminating NUL */
            break;
        }
        memmove(result + (pos - s) + 1, result, len + 1);
        memcpy(result, s, (pos - s) + 1);
        current = AZ_PREVIOUS(edges, current);
    }
    puts(result);
}

static void AZ_TRACE_Edges(const char *prefix, const unsigned char *source, const int length, const char initial_mode,
            struct az_edge *edges, const int vertexIndex) {
    int i, j, e_i;
    char result[1024 * 2];
    if (vertexIndex) {
        printf(prefix, vertexIndex);
    } else {
        fputs(prefix, stdout);
    }
    for (i = vertexIndex; i <= length; i++) {
        e_i = i * AZ_NUM_MODES;
        for (j = 0; j < AZ_NUM_MODES; j++) {
            if (edges[e_i + j].mode) {
                fputs(" **** ", stdout);
                AZ_TRACE_Path(source, length, initial_mode, edges, edges + e_i + j, result, (int) ARRAY_SIZE(result));
            }
        }
    }
}

static void AZ_TRACE_AddEdge(const unsigned char *source, const int length, struct az_edge *edges,
            struct az_edge *previous, const int vertexIndex, struct az_edge *edge) {
    const int v_ij = vertexIndex * AZ_NUM_MODES + AZ_MASK(edge->mode) - 1;

    (void)source; (void)length;

    printf("add mode %c_%d, previous %d, from %d, len %d, v_ij mode %d || v_ij.size %d >= edge->size %d\n",
            az_mode_char(edge->mode), v_ij, previous ? (int) (previous - edges) : 0, edge->from, edge->len,
            edges[v_ij].mode, edges[v_ij].size, edge->size);
}

static void AZ_TRACE_NotAddEdge(const unsigned char *source, const int length, struct az_edge *edges,
            struct az_edge *previous, const int vertexIndex, const int e_ij, struct az_edge *edge) {
    const int v_ij = vertexIndex * AZ_NUM_MODES + AZ_MASK(edge->mode) - 1;

    (void)source; (void)length; (void)e_ij;

    printf("NOT mode %c_%d, previous %d, from %d, len %d, v_ij.size %d < edge->size %d\n",
            az_mode_char(edge->mode), v_ij, previous ? (int) (previous - edges) : 0, edge->from, edge->len,
            edges[v_ij].size, edge->size);
}

/* vim: set ts=4 sw=4 et : */
#endif /* Z_AZTEC_TRACE_H */
