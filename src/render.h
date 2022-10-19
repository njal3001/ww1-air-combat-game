#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdint.h>
#include <stdbool.h>
#include "spatial.h"
#include "transform.h"
#include "camera.h"

struct color
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
};

struct vertex
{
    struct vec3 pos;
    struct vec3 norm;
    struct color col;
    float uvx;
    float uvy;
};

struct texture
{
    GLuint id;
    GLsizei width, height;
};

struct mesh
{
    struct vertex *vertices;
    GLushort *indices;
    size_t vertex_count;
    size_t index_count;
    const struct texture *texture;
};

bool render_init(GLFWwindow *window);
void render_shutdown();

void render_begin();
void render_end();
void render_flush();

void set_texture(const struct texture *texture);

void render_tri(struct vec3 a, struct vec3 b, struct vec3 c,
        struct color col_a, struct color col_b, struct color col_c,
        float uvx_a, float uvy_a, float uvx_b, float uvy_b, float uvx_c, float uvy_c);
void render_quad(struct vec3 a, struct vec3 b, struct vec3 c, struct vec3 d, struct color col);

void render_mesh(const struct mesh *mesh, const struct transform *transform);

void set_light_pos(struct vec3 pos);

struct camera *get_camera();

struct color color_create(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
struct vec3 color_to_vec3(struct color col);

extern const struct color COLOR_WHITE;
extern const struct color COLOR_BLACK;
extern const struct color COLOR_RED;
extern const struct color COLOR_GREEN;
extern const struct color COLOR_BLUE;
