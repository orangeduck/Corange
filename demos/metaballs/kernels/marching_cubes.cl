int volume_coords(int3 coords, int3 size) {
  return coords.x + coords.y * size.x + coords.z * size.x * size.y;
}

__kernel void write_clear(global float* volume) {
  volume[get_global_id(0)] = 0.0f;
}

__kernel void write_point(global float* volume, int x, int y, int z, int size_x, int size_y, int size_z, float value) {
  int pos = volume_coords((int3)(x, y, z), (int3)(size_x, size_y, size_z));
  volume[pos] = value;
}

__kernel void write_metaball(global float* volume,
                             int bot_x, int bot_y, int bot_z,
                             int top_x, int top_y, int top_z,
                             int size_x, int size_y, int size_z,
                             float x, float y, float z, float size) {
  
  int id = get_global_id(0);
  
  int3 box_size = (int3)(top_x - bot_x, top_y - bot_y, top_z - bot_z);
  int3 box_offset = (int3)(bot_x, bot_y, bot_z);
  
  int3 pos = box_offset + (int3)( id % box_size.x, (id / (box_size.x)) % box_size.y, id / (box_size.x * box_size.y) );
  
  /* So this distance function needs to be changed to a deterministic dropoff */
  float amount = 1 / distance((float3)(pos.x, pos.y, pos.z), (float3)(x, y, z));
  
  int index = volume_coords(pos, (int3)(size_x, size_y, size_z));
  volume[index] += amount;
}



__kernel void write_point_color_back(global float* volume, __global float4* point_color) {
  float color = volume[get_global_id(0)];
  point_color[get_global_id(0)] = (float4)(color, 0, 0, 1);
}
