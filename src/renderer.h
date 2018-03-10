#ifndef RENDERER_H
#define RENDERER_H
#include "vector.h"
#include "matrix.h"
#include "sprite.h"
#include "text.h"
#include "texture.h"

void init_renderer(void* window);
void cleanup_renderer();

void begin_frame();
void end_frame(float transition_time);

void draw_texture(mat4 m, gltex t, vec4 c);
void draw_texture_ui(mat4 m, gltex t, vec4 c);
void draw_text(vec3 p, text t, vec4 c);
void draw_sprite(mat4 m, sprite spr, vec4 c);
void draw_debug_rect(aabb_2d rect, vec4 color);
mat4 get_camera_matrix();

#endif
