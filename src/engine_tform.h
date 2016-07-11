#ifndef ENGINE_TFORM_H_INCLUDED
#define ENGINE_TFORM_H_INCLUDED

extern stored_functions_engine tform_engine_vtbl;

struct SinCos
{
    float sin;
    float cos;
};

extern SinCos *SinCos_table;

#define MAT_FLAG_INV_SIN   1
#define MAT_FLAG_INV_COS   2

struct xyz
{
    float sx;
    float sy;
    float sz;
};

struct mat3x3
{
    float m00;
    float m01;
    float m02;
    float m10;
    float m11;
    float m12;
    float m20;
    float m21;
    float m22;
};

struct destFX
{
    char type_flag;
    int p1; // Model id. >= 0
    float p2;
    float p3;
    float p4;
};

void mat_mult(mat3x3 *mat1, mat3x3 *mat2, mat3x3 *dst);
void mat_rotate_y(mat3x3 *mat, float a2);

void mat_gen_axis_rotate(xyz *u, float angle, mat3x3 *out, int flags = 0);

#endif // ENGINE_TFORM_H_INCLUDED
