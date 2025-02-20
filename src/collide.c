#include "collide.h"
#include <float.h>
#include <math.h>
#include "render.h"

struct cbox_info
{
    struct vec3 axis_x;
    struct vec3 axis_y;
    struct vec3 axis_z;
    struct vec3 pos;
    struct vec3 points[8];
};

struct cbox_prj
{
    float start;
    float end;
};

struct bbox
{
    float x1, x2;
    float y1, y2;
    float z1, z2;
};

static struct bbox get_bbox(const struct actor *a)
{
    struct bbox res;

    struct vec3 offset = vec3_vmul(a->cbox.offset, a->transform.scale);
    struct vec3 center = vec3_add(a->transform.pos, offset);
    struct vec3 delta = vec3_vmul(a->cbox.bounds, a->transform.scale);

    float delta_max = fmax(fmax(delta.x, delta.y), delta.z);

    res.x1 = center.x - delta_max;
    res.x2 = center.x + delta_max;
    res.y1 = center.y - delta_max;
    res.y2 = center.y + delta_max;
    res.z1 = center.z - delta_max;
    res.z2 = center.z + delta_max;

    return res;
}

static bool bbox_overlapping(struct bbox a, struct bbox b)
{
    return
        a.x1 <= b.x2 &&
        a.x2 >= b.x1 &&
        a.y1 <= b.y2 &&
        a.y2 >= b.y1 &&
        a.z1 <= b.z2 &&
        a.z2 >= b.z1;
}

static bool check_broad_collide(const struct actor *a, const struct actor *b)
{
    struct bbox bbox_a = get_bbox(a);
    struct bbox bbox_b = get_bbox(b);

    return bbox_overlapping(bbox_a, bbox_b);
}

static void get_cbox_info(struct cbox_info *info, const struct actor *ac)
{
    info->axis_x = transform_right(&ac->transform);
    info->axis_y = transform_up(&ac->transform);
    info->axis_z = transform_forward(&ac->transform);
    struct vec3 p = mat4_v3mul(transform_matrix(&ac->transform),
            ac->cbox.offset);
    struct vec3 dx = vec3_mul(info->axis_x,
            ac->transform.scale.x * ac->cbox.bounds.x);
    struct vec3 dy = vec3_mul(info->axis_y,
            ac->transform.scale.y * ac->cbox.bounds.y);
    struct vec3 dz = vec3_mul(info->axis_z,
            ac->transform.scale.z * ac->cbox.bounds.z);

    info->points[0] = vec3_add(vec3_add(vec3_sub(p, dx), dy), dz);
    info->points[1] = vec3_add(vec3_sub(vec3_sub(p, dx), dy), dz);
    info->points[2] = vec3_add(vec3_sub(vec3_add(p, dx), dy), dz);
    info->points[3] = vec3_add(vec3_add(vec3_add(p, dx), dy), dz);
    info->points[4] = vec3_sub(vec3_add(vec3_sub(p, dx), dy), dz);
    info->points[5] = vec3_sub(vec3_sub(vec3_sub(p, dx), dy), dz);
    info->points[6] = vec3_sub(vec3_sub(vec3_add(p, dx), dy), dz);
    info->points[7] = vec3_sub(vec3_add(vec3_add(p, dx), dy), dz);
}

static struct cbox_prj get_cbox_projection(const struct cbox_info *info,
        struct vec3 axis)
{
    struct cbox_prj res;
    res.start = FLT_MAX;
    res.end = -FLT_MAX;

    for (size_t i = 0; i < 8; i++)
    {
        float val = vec3_dot(info->points[i], axis);
        res.start = fminf(val, res.start);
        res.end = fmaxf(val, res.end);
    }

    return res;
}

bool check_collide(const struct actor *a, const struct actor *b)
{
    if (!check_broad_collide(a, b))
    {
        return false;
    }

    struct cbox_info ainfo;
    get_cbox_info(&ainfo, a);

    struct cbox_info binfo;
    get_cbox_info(&binfo, b);

    struct vec3 axes[15];
    axes[0] = ainfo.axis_x;
    axes[1] = ainfo.axis_y;
    axes[2] = ainfo.axis_z;
    axes[3] = binfo.axis_x;
    axes[4] = binfo.axis_y;
    axes[5] = binfo.axis_z;
    axes[6] = vec3_cross(ainfo.axis_x, binfo.axis_x);
    axes[7] = vec3_cross(ainfo.axis_y, binfo.axis_x);
    axes[8] = vec3_cross(ainfo.axis_z, binfo.axis_x);
    axes[9] = vec3_cross(ainfo.axis_x, binfo.axis_y);
    axes[10] = vec3_cross(ainfo.axis_y, binfo.axis_y);
    axes[11] = vec3_cross(ainfo.axis_z, binfo.axis_y);
    axes[12] = vec3_cross(ainfo.axis_x, binfo.axis_z);
    axes[13] = vec3_cross(ainfo.axis_y, binfo.axis_z);
    axes[14] = vec3_cross(ainfo.axis_z, binfo.axis_z);

    for (size_t i = 0; i < 15; i++)
    {
        struct vec3 axis = axes[i];
        struct cbox_prj prj1 = get_cbox_projection(&ainfo, axis);
        struct cbox_prj prj2 = get_cbox_projection(&binfo, axis);

        // No collision if projection does not overlap
        if (fminf(prj1.end, prj2.end) < fmaxf(prj1.start, prj2.start))
        {
            return false;
        }
    }

    return true;
}

void render_collider_outline(const struct actor *ac, float thickness,
        struct color col)
{
    struct cbox_info info;
    get_cbox_info(&info, ac);

    render_push_untextured_volume_outline(info.points[0], info.points[1],
            info.points[2], info.points[3], info.points[4], info.points[5],
            info.points[6], info.points[7], thickness, col);
}
