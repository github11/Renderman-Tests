





/*
  main.c
  
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

#include <stdio.h>
#include <stdlib.h>

#include <math.h>

#include <ri.h>

#include "trimesh.h"


typedef struct camera_s {
    RtPoint location;
    RtPoint look_at;
    double roll;
} camera_t;

typedef struct scene_info_s {
    camera_t cam;
    char *fprefix;
} scene_info_t;


double randF(double min, double max);


const double PI = 3.141592654;

double randF(double min, double max) {
    return (max-min) + ((double)rand())/((double)RAND_MAX) + min;
}

/*
 * AimZ(): rotate the world so the direction vector points in
 *  positive z by rotating about the y axis, then x. The cosine
 *  of each rotation is given by components of the normalized
 *  direction vector. Before the y rotation the direction vector
 *  might be in negative z, but not afterward.
 */
void AimZ(RtPoint direction)
{
    double xzlen, yzlen, yrot, xrot;

    if (direction[0]==0 && direction[1]==0 && direction[2]==0)
        return;

    /*
     * The initial rotation about the y axis is given by the projection of
     * the direction vector onto the x,z plane: the x and z components
     * of the direction.
     */
    xzlen = sqrt(direction[0]*direction[0]+direction[2]*direction[2]);
    if (xzlen == 0)
        yrot = (direction[1] < 0) ? 180 : 0;
    else
        yrot = 180*acos(direction[2]/xzlen)/PI;

    /*
     * The second rotation, about the x axis, is given by the projection on
     * the y,z plane of the y-rotated direction vector: the original y
     * component, and the rotated x,z vector from above.
     */
    yzlen = sqrt(direction[1]*direction[1]+xzlen*xzlen);
    xrot = 180*acos(xzlen/yzlen)/PI; /* yzlen should never be 0 */

    if (direction[1] > 0)
        RiRotate(xrot, 1.0, 0.0, 0.0);
    else
        RiRotate(-xrot, 1.0, 0.0, 0.0);

    /* The last rotation declared gets performed first */
    if (direction[0] > 0)
        RiRotate(-yrot, 0.0, 1.0, 0.0);
    else
        RiRotate(yrot, 0.0, 1.0, 0.0);
}

void PlaceCamera(camera_t *cam)
{
    RiRotate(-cam->roll, 0.0, 0.0, 1.0);
    RtPoint direction;
    direction[0] = cam->look_at[0]-cam->location[0];
    direction[1] = cam->look_at[1]-cam->location[1];
    direction[2] = cam->look_at[2]-cam->location[2];
    AimZ(direction);
    RiTranslate(-cam->location[0], -cam->location[1], -cam->location[2]);
}

void doFrame(int fNum, scene_info_t *scene, tri_mesh_t *tmesh);

double x(double u, double v) {
    return u;
}
double y(double u, double v) {
    return v;
}
double z(double u, double v) {
    return 4*sin(u) * cos(v);
}

double r(double u, double v) {
    return 0.0;
}
double g(double u, double v) {
    return 0.8;
}
double b(double u, double v) {
    return 0.2;
}

void gen_terrain(tri_mesh_t *tmesh) {
    size_t NUM_I = tmesh->NUM_I;
    size_t NUM_J = tmesh->NUM_J;

    double umin = -12*PI;
    double umax = 12*PI;
    double vmin = -12*PI;
    double vmax = 12*PI;
    double du = (umax-umin)/(NUM_I-1);
    double dv = (vmax-vmin)/(NUM_J-1);

    
    double u = umin;
    for (size_t i=0; i< NUM_I; ++i) {

        double v = vmin;
        for (size_t j=0; j < NUM_J; ++j) {
            tmesh_set_pt(tmesh, i,j, x(u,v), z(u,v), y(u,v));
            
            if (i & j & 1) {
                tmesh_set_color(tmesh, i,j, 0,1,0);
            } else {
                tmesh_set_color(tmesh, i,j, 0,0,0);
            }
            v += dv;
        }
        u += du;
    }
}

void doFrame(int fNum, scene_info_t *scene, tri_mesh_t *tmesh) {
    RtInt on = 1;
    RiFrameBegin(fNum);

    char buffer[256];
    sprintf(buffer, "images/%s%05d.tif", scene->fprefix, fNum);
    RiDisplay(buffer,(char*)"file",(char*)"rgba",RI_NULL);
  
    RiFormat(800, 600,  1.25);


    RiProjection((char*)"perspective",RI_NULL);
    PlaceCamera(&scene->cam);

    /* RiAttribute("visibility", "int trace", &on, RI_NULL); */
    RiAttribute( "visibility",
                 "int camera", (RtPointer)&on,
                 "int transmission", (RtPointer)&on,
                 "int diffuse", (RtPointer)&on,
                 "int specular", (RtPointer)&on,
                 "int photon", (RtPointer)&on,
                 RI_NULL );
    RtString on_string = "on";
    RtInt samples = 2;
    RiAttribute( "light", (RtToken)"shadows", (RtPointer)&on_string, (RtToken)"samples", (RtPointer)&samples, RI_NULL );
    RtPoint lightPos = {40,80,40};
    RiAttribute((RtToken)"light", "string shadow", (RtPointer)"on", RI_NULL);
    RiLightSource("distantlight", (RtToken)"from", (RtPointer)lightPos, RI_NULL);
    
    RiWorldBegin();
  
    RiSurface((char*)"matte", RI_NULL);

    tmesh_render(tmesh);

    RiWorldEnd();
    RiFrameEnd();
}

int main(int argc, char *argv[]) {
    if (argc<2) {
        printf("No output file name prefixgiven.\n");
        printf("Use:\n\t%s output_prefix\n\n", argv[0]);
        exit(1);
    }

    const size_t NUM_FRAMES = 20;

    RiBegin(RI_NULL);
    RtInt md = 4;
    RiOption("trace", "maxdepth", &md, RI_NULL);
    RiSides(2);

    scene_info_t scene;

    scene.cam.location[0] = 20;
    scene.cam.location[1] = 20;
    scene.cam.location[2] = 20;

    scene.cam.look_at[0]= 0.0;
    scene.cam.look_at[1]= 0.0;
    scene.cam.look_at[2]= 0.0;
    scene.cam.roll = 0.0;
    
    scene.fprefix = argv[1];

    /* size_t cur_frame = 0; */

    double rad = 40.0;
    double t = 0.0;
    double dt = 2.0*PI/(NUM_FRAMES-1);
    tri_mesh_t tmesh;

    tmesh_alloc(&tmesh, 256,256);
    gen_terrain(&tmesh);
    
    for (size_t fnum = 0; fnum < NUM_FRAMES; ++fnum) {
        scene.cam.location[0] = rad * sin(t);
        scene.cam.location[2] = rad * cos(t);
        t += dt;
        printf("Rendering frame %lu\n", fnum);
        doFrame(fnum, &scene, &tmesh);
    }
    tmesh_free(&tmesh);
    RiEnd();

    return 0;
}