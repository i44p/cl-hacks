__kernel void _vec_add_float(__global const float *A_d, __global const float *B_d, __global float *result_d)
{
    int gid = get_global_id(0);
    result_d[gid] = A_d[gid] + B_d[gid];
}
