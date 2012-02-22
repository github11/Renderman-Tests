/*
  trimesh.c
  
  Copyright (c) 2012, Jeremiah LaRocco jeremiah.larocco@gmail.com

  Permission to use, copy, modify, and/or distribute this software for any
  purpose with or without fee is hereby granted, provided that the above
  copyright notice and this permission notice appear in all copies.

  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#include "trimesh.h"

#include <ri.h>

size_t idx(size_t i, size_t j, size_t NUM_I, size_t NUM_J) {
    return i*NUM_J + j;
}

void tmesh_alloc(tri_mesh_t *tmesh, size_t ni, size_t nj) {
    tmesh->NUM_I = ni;
    tmesh->NUM_J = nj;

    tmesh->pts = malloc(sizeof(RtPoint)*ni*nj);
    tmesh->colors = malloc(sizeof(RtColor)*ni*nj);

    for (size_t i=0; i< ni; ++i) {
        for (size_t j=0; j < nj; ++j) {
            tmesh_set_pt(tmesh, i,j, 0.0,0.0,0.0);
            tmesh_set_color(tmesh, i,j, 0.0,0.0,0.0);
        }
    }
    
    tmesh->npolys = 2*(nj+1)*(ni+1);

    tmesh->nvertices = malloc(sizeof(RtInt) * tmesh->npolys);
    for (size_t i=0; i<tmesh->npolys; ++i) {
        tmesh->nvertices[i] = 3;
    }
    
    tmesh->vertices = malloc(sizeof(RtInt)*3*tmesh->npolys);
    size_t curIdx = 0;
    for (size_t i = 0; i<(ni-1); ++i) {
        for (size_t j=0; j<(ni-1); ++j) {

            tmesh->vertices[curIdx  ] = idx(i,j, ni, nj);
            tmesh->vertices[curIdx+1] = idx(i,j+1, ni, nj);
            tmesh->vertices[curIdx+2] = idx(i+1,j, ni, nj);
            
            tmesh->vertices[curIdx+3] = idx(i,j+1, ni, nj);
            tmesh->vertices[curIdx+4] = idx(i+1,j+1, ni, nj);
            tmesh->vertices[curIdx+5] = idx(i+1,j, ni, nj);

            curIdx += 6;
        }
    }
}

void tmesh_free(tri_mesh_t *tmesh) {
    free(tmesh->vertices);
    free(tmesh->nvertices);
    free(tmesh->colors);
    free(tmesh->pts);

    tmesh->vertices = NULL;
    tmesh->nvertices = NULL;
    tmesh->colors = NULL;
    tmesh->pts = NULL;
    
    tmesh->NUM_I = 0;
    tmesh->NUM_J = 0;
    tmesh->npolys = 0;
}

void tmesh_render(tri_mesh_t *tmesh) {
    RiPointsPolygons(tmesh->npolys, tmesh->nvertices, tmesh->vertices, "P", tmesh->pts, "Cs", tmesh->colors, RI_NULL);
}
void tmesh_set_pt(tri_mesh_t *tmesh, size_t i, size_t j, double x, double y, double z) {
    tmesh->pts[idx(i,j, tmesh->NUM_I, tmesh->NUM_J)][0] = x;
    tmesh->pts[idx(i,j, tmesh->NUM_I, tmesh->NUM_J)][1] = y;
    tmesh->pts[idx(i,j, tmesh->NUM_I, tmesh->NUM_J)][2] = z;
}
void tmesh_set_color(tri_mesh_t *tmesh, size_t i, size_t j, double r, double g, double b) {
    tmesh->colors[idx(i,j, tmesh->NUM_I, tmesh->NUM_J)][0] = r;
    tmesh->colors[idx(i,j, tmesh->NUM_I, tmesh->NUM_J)][1] = g;
    tmesh->colors[idx(i,j, tmesh->NUM_I, tmesh->NUM_J)][2] = b;
}