int volume_coords(int3 coords, int3 size) {
  return coords.x + coords.y * size.x + coords.z * size.x * size.y;
}

int3 volume_position(int index, int3 size) {
  return (int3)( index % size.x, (index / (size.x)) % size.y, index / (size.x * size.y) );
}

kernel void write_clear(global float* volume) {
  volume[get_global_id(0)] = 0.0f;
}

kernel void write_point(global float* volume, int x, int y, int z, int size_x, int size_y, int size_z, float value) {
  int pos = volume_coords((int3)(x, y, z), (int3)(size_x, size_y, size_z));
  volume[pos] = value;
}

kernel void write_metaball(global float* volume,
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



kernel void write_point_color_back(global float* volume, global float4* point_color) {
  float color = volume[get_global_id(0)];
  point_color[get_global_id(0)] = (float4)(color, 0, 0, 1);
}

void lock_release(global int* semaphor) {
  atom_xchg(semaphor, 0);
}

void lock_aquire(global int* semaphor) {
  while(atom_cmpxchg(semaphor, 0, 1));
}

float4 vertex_lerp(float threashold, float4 pos1, float4 pos2, float val1, float val2) {
   
  float mu = (threashold - val1) / (val2 - val1);
  
  float4 pos;
  pos.x = pos1.x + mu * (pos2.x - pos1.x);
  pos.y = pos1.y + mu * (pos2.y - pos1.y);
  pos.z = pos1.z + mu * (pos2.z - pos1.z);
  pos.w = 1.0;

  return pos;
}

#include "./kernels/lookup_table.cl"

kernel void construct_surface(global float* volume,
                              int3 volume_size,
                              global float4* vertex_buffer,
                              global int* vertex_index,
                              global int* vertex_lock) {
  
  int id = get_global_id(0);
  int3 pos = volume_position(id, volume_size);
  
  float v0 = volume[volume_coords(pos + (int3)(0,0,0), volume_size)];
  float v1 = volume[volume_coords(pos + (int3)(1,0,0), volume_size)];
  float v2 = volume[volume_coords(pos + (int3)(1,0,1), volume_size)];
  float v3 = volume[volume_coords(pos + (int3)(0,0,1), volume_size)];
  float v4 = volume[volume_coords(pos + (int3)(0,1,0), volume_size)];
  float v5 = volume[volume_coords(pos + (int3)(1,1,0), volume_size)];
  float v6 = volume[volume_coords(pos + (int3)(1,1,1), volume_size)];
  float v7 = volume[volume_coords(pos + (int3)(0,1,1), volume_size)];
  
  const float threashold = 0.1;
  
  unsigned char c0 = v0 > threashold;
  unsigned char c1 = v1 > threashold;
  unsigned char c2 = v2 > threashold;
  unsigned char c3 = v3 > threashold;
  unsigned char c4 = v4 > threashold;
  unsigned char c5 = v5 > threashold;
  unsigned char c6 = v6 > threashold;
  unsigned char c7 = v7 > threashold;
  
  unsigned char hash = c0 | (c1 << 1) | (c2 << 2) | (c3 << 3) | (c4 << 4) | (c5 << 5) | (c6 << 6) | (c7 << 7);
  
  if ((hash == 0) || (hash == 255)) {
    return;
  }
  
  float4 p0 = (float4)(pos.x, pos.y, pos.z, 1) + (float4)(0,0,0, 0);
  float4 p1 = (float4)(pos.x, pos.y, pos.z, 1) + (float4)(1,0,0, 0);
  float4 p2 = (float4)(pos.x, pos.y, pos.z, 1) + (float4)(1,0,1, 0);
  float4 p3 = (float4)(pos.x, pos.y, pos.z, 1) + (float4)(0,0,1, 0);
  float4 p4 = (float4)(pos.x, pos.y, pos.z, 1) + (float4)(0,1,0, 0);
  float4 p5 = (float4)(pos.x, pos.y, pos.z, 1) + (float4)(1,1,0, 0);
  float4 p6 = (float4)(pos.x, pos.y, pos.z, 1) + (float4)(1,1,1, 0);
  float4 p7 = (float4)(pos.x, pos.y, pos.z, 1) + (float4)(0,1,1, 0);
  
  float4 vert_list[12];
  
  int edge_mask = edge_table[hash];
  
  /* Find the vertices where the surface intersects the cube */
  
  if (edge_mask & 1) vert_list[0] = vertex_lerp(threashold, p0, p1, c0, c1);
  if (edge_mask & 2) vert_list[1] = vertex_lerp(threashold, p1, p2, c1, c2);
  if (edge_mask & 4) vert_list[2] = vertex_lerp(threashold, p2, p3, c2, c3);
  if (edge_mask & 8) vert_list[3] = vertex_lerp(threashold, p3, p0, c3, c0);
  if (edge_mask & 16) vert_list[4] = vertex_lerp(threashold, p4, p5, c4, c5);
  if (edge_mask & 32) vert_list[5] = vertex_lerp(threashold, p5, p6, c5, c6);
  if (edge_mask & 64) vert_list[6] = vertex_lerp(threashold, p6, p7, c6, c7);
  if (edge_mask & 128) vert_list[7] = vertex_lerp(threashold, p7, p4, c7, c4);
  if (edge_mask & 256) vert_list[8] = vertex_lerp(threashold, p0, p4, c0, c4);
  if (edge_mask & 512) vert_list[9] = vertex_lerp(threashold, p1, p5, c1, c5);
  if (edge_mask & 1024) vert_list[10] = vertex_lerp(threashold, p2, p6, c2, c6);
  if (edge_mask & 2048) vert_list[11] = vertex_lerp(threashold, p3, p7, c3, c7);
  
  /* Push appropriate verts to the back of the vertex buffer */
  
  lock_aquire(vertex_lock);
  for(int i = 0; triangle_table[hash][i] != -1; i+=3) {
    vertex_buffer[*vertex_index] = vert_list[triangle_table[hash][i  ]]; *vertex_index++;
    vertex_buffer[*vertex_index] = vert_list[triangle_table[hash][i+1]]; *vertex_index++;
    vertex_buffer[*vertex_index] = vert_list[triangle_table[hash][i+2]]; *vertex_index++;
  }
  lock_release(vertex_lock);
}
