__kernel void vector_add_gpu(__global const float* src_a, __global const float* src_b, __global float* res, const int num) {

  const int idx = get_global_id(0);
   if (idx < num) {
    res[idx] = src_a[idx] + src_b[idx];
  }
   
}