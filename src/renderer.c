#include "renderer.h"

#include "camera.h"
#include "check.h"
#include "data_loader.h"
#include "debug_draw.h"
#include "graphics_log.h"
#include "matrix.h"
#include "mesh.h"
#include "paths.h"
#include "renderpass.h"
#include "shader_init.h"
#include "shader.h"
#include "texture_loader.h"
#include "vector.h"
#include "window.h"

#include <GL/glew.h>
#include <GL/gl.h>

shader s_default;
static shader s_transition;
static renderpass transition_pass = NULL;
static gltex  t_transition;

extern camera c_main;
extern camera c_ui;

static const vec4 transition_color = { .r = 34.0f/256.0f, .g = 32.0f/256.0f, .b = 52.0f/256.0f, .a = 1.0f};

void init_renderer(void* window) {
    // ------------------------------------------------
    glewExperimental = 1;
    check_kill(glewInit() == GLEW_OK, "Failed to initialize GLEW");
    if(!check_error(glewIsExtensionSupported("GL_ARB_debug_output"), "Graphics debug logging is not available for your hardware. Graphical errors will not be reported")) {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(graphics_log, NULL);

        GLuint VAO; // Setup default VAO
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    // ------------------------------------------------

    shaders_init();
    s_default = shader_basic_tex_get();

    char* path = assets_path("transition.vert", NULL);
    char* vert = (char*)load_data_buffer(path, NULL);
    sfree(path);

    path = assets_path("transition.frag", NULL);
    char* frag = (char*)load_data_buffer(path, NULL);
    sfree(path);
    s_transition = shader_new_vf((const char**)&vert, (const char**)&frag);

    path = assets_path("Transition.png", NULL);
    t_transition = load_texture_gl(path);
    sfree(path);

    vec2 v = get_window_dims(window);
    transition_pass = renderpass_new(vec2_decomp(v));
}
void cleanup_renderer() {
    renderpass_free(transition_pass);
    glDeleteProgram(s_transition.id);
}

void begin_frame() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    renderpass_start(transition_pass);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
void end_frame(float transition_time) {
    glUseProgram(s_transition.id);
    shader_bind_uniform_name(s_transition, "u_time", transition_time * 2);
    shader_bind_uniform_name(s_transition, "u_transition_color", transition_color);
    shader_bind_uniform_texture_name(s_transition, "u_transition_texture", t_transition, GL_TEXTURE1);
    renderpass_next(NULL, s_transition);
}

void draw_texture(mat4 m, gltex t, vec4 c) {
    glUseProgram(s_default.id);
    vec2 dims = { .x = t.width * 2, .y = t.height * 2 };
    shader_bind_uniform_name(s_default, "u_color", c);
    shader_bind_uniform_name(s_default, "u_transform", mat4_mul(camera_get_vp(c_main), mat4_scale(m, dims)));
    shader_bind_uniform_texture_name(s_default, "u_texture", t, GL_TEXTURE0);
    mesh_render(s_default, mesh_quad(), GL_TRIANGLES, "i_pos", VT_POSITION, "i_uv", VT_TEXTURE);
}

void draw_texture_ui(mat4 m, gltex t, vec4 c) {
    glUseProgram(s_default.id);
    vec2 dims = { .x = t.width * 2, .y = t.height * 2 };
    shader_bind_uniform_name(s_default, "u_color", c);
    shader_bind_uniform_name(s_default, "u_transform", mat4_mul(camera_get_vp(c_ui), mat4_scale(m, dims)));
    shader_bind_uniform_texture_name(s_default, "u_texture", t, GL_TEXTURE0);
    mesh_render(s_default, mesh_quad(), GL_TRIANGLES, "i_pos", VT_POSITION, "i_uv", VT_TEXTURE);
}

void draw_text(vec3 p, text t, vec4 c) {
    glUseProgram(s_default.id);
    shader_bind_uniform_name(s_default, "u_color", c);
    text_draw(t, s_default, mat4_mul(camera_get_vp(c_ui), mat4_translate(mat4_ident, p)));
}

void draw_sprite(mat4 m, sprite spr, vec4 c) {
    glUseProgram(s_default.id);
    shader_bind_uniform_name(s_default, "u_color", c);
    vec2 dims = { .x = 2, .y = 2 };
    sprite_draw(spr, s_default, mat4_scale(m, dims), camera_get_vp(c_main));
}

void draw_debug_rect(aabb_2d rect, vec4 color) {
#ifdef BUILD_EDITOR
    vt_pc ul = (vt_pc){ .position = { .x = rect.position.x, .y = rect.position.y }, .color = color};
    vt_pc ur = (vt_pc){ .position = { .x = rect.position.x + rect.dimensions.x, .y = rect.position.y }, .color = color};
    vt_pc bl = (vt_pc){ .position = { .x = rect.position.x, .y = rect.position.y + rect.dimensions.y }, .color = color};
    vt_pc br = (vt_pc){ .position = { .x = rect.position.x + rect.dimensions.x, .y = rect.position.y + rect.dimensions.y }, .color = color};
    debug_draw_line(camera_get_vp(c_main), ul, ur, 2);
    debug_draw_line(camera_get_vp(c_main), ur, br, 2);
    debug_draw_line(camera_get_vp(c_main), bl, br, 2);
    debug_draw_line(camera_get_vp(c_main), ul, bl, 2);
#endif
}

mat4 get_camera_matrix() {
    return camera_get_vp(c_main);
}
