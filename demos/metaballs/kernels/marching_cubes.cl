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


float smoothstepmap(float val) {
  return val*val*(3 - 2*val);
}

kernel void write_metaball(global float* volume,
                           int3 bottom, int3 top, int3 size,
                           float x, float y, float z) {
  
  const int METABALL_SIZE = 10;
  
  int id = get_global_id(0);
  
  int3 box_size = top - bottom;
  int3 pos = bottom + volume_position(id, box_size);
  
  /* So this distance function needs to be changed to a deterministic dropoff */
  float dist = distance((float3)(pos.x, pos.y, pos.z), (float3)(x, y, z)) / METABALL_SIZE;
  float amount = 1-smoothstepmap( clamp(dist, 0.0f, 1.0f) );
  
  int index = volume_coords(pos, size);
  
  volume[index] += amount;
}



kernel void write_point_color_back(global float* volume, global float4* point_color) {
  float color = volume[get_global_id(0)];
  point_color[get_global_id(0)] = (float4)(color, 0, 0, 1);
}

float4 vertex_lerp(float threashold, float4 pos1, float4 pos2, float val1, float val2) {
  float mu = (threashold - val1) / (val2 - val1);
  float4 ret = pos1 + mu * (pos2 - pos1);
  ret.w = 1;
  return ret;
}

#include "./kernels/lookup_table.cl"

kernel void construct_surface(global float* volume,
                              int3 volume_size,
                              global float4* vertex_buffer,
                              global int* vertex_index) {
  
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
  
  const float threashold = 0.5;
  
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
  
  /* Find the vertices where the surface intersects the cube */
  
  /*
  int edge_mask = edge_table[hash];
  if (edge_mask & 1) vert_list[0] = vertex_lerp(threashold, p0, p1, v0, v1);
  if (edge_mask & 2) vert_list[1] = vertex_lerp(threashold, p1, p2, v1, v2);
  if (edge_mask & 4) vert_list[2] = vertex_lerp(threashold, p2, p3, v2, v3);
  if (edge_mask & 8) vert_list[3] = vertex_lerp(threashold, p3, p0, v3, v0);
  if (edge_mask & 16) vert_list[4] = vertex_lerp(threashold, p4, p5, v4, v5);
  if (edge_mask & 32) vert_list[5] = vertex_lerp(threashold, p5, p6, v5, v6);
  if (edge_mask & 64) vert_list[6] = vertex_lerp(threashold, p6, p7, v6, v7);
  if (edge_mask & 128) vert_list[7] = vertex_lerp(threashold, p7, p4, v7, v4);
  if (edge_mask & 256) vert_list[8] = vertex_lerp(threashold, p0, p4, v0, v4);
  if (edge_mask & 512) vert_list[9] = vertex_lerp(threashold, p1, p5, v1, v5);
  if (edge_mask & 1024) vert_list[10] = vertex_lerp(threashold, p2, p6, v2, v6);
  if (edge_mask & 2048) vert_list[11] = vertex_lerp(threashold, p3, p7, v3, v7);
  */
  
  vert_list[0] = vertex_lerp(threashold, p0, p1, v0, v1);
  vert_list[1] = vertex_lerp(threashold, p1, p2, v1, v2);
  vert_list[2] = vertex_lerp(threashold, p2, p3, v2, v3);
  vert_list[3] = vertex_lerp(threashold, p3, p0, v3, v0);
  vert_list[4] = vertex_lerp(threashold, p4, p5, v4, v5);
  vert_list[5] = vertex_lerp(threashold, p5, p6, v5, v6);
  vert_list[6] = vertex_lerp(threashold, p6, p7, v6, v7);
  vert_list[7] = vertex_lerp(threashold, p7, p4, v7, v4);
  vert_list[8] = vertex_lerp(threashold, p0, p4, v0, v4);
  vert_list[9] = vertex_lerp(threashold, p1, p5, v1, v5);
  vert_list[10] = vertex_lerp(threashold, p2, p6, v2, v6);
  vert_list[11] = vertex_lerp(threashold, p3, p7, v3, v7);
  
  /* Push appropriate verts to the back of the vertex buffer */
  
  int num_verts = 0;
  while(triangle_table[hash][num_verts] != -1) {
    num_verts += 3;
  }
  
  int index = atomic_add(vertex_index, num_verts);
  
  for(int i = 0; i < num_verts; i++) {
    vertex_buffer[index + i] = vert_list[triangle_table[hash][i]];
  }
  
}

kernel void generate_flat_normals(global float4* vertex_positions, global float4* vertex_normals) {
  
  int id = get_global_id(0);
  
  float4 pos1 = vertex_positions[id * 3 + 0];
  float4 pos2 = vertex_positions[id * 3 + 1];
  float4 pos3 = vertex_positions[id * 3 + 2];
  
  float3 pos12 = pos2.xyz - pos1.xyz;
  float3 pos13 = pos3.xyz - pos1.xyz;

  float3 normal = cross(pos12, pos13);
  normal = normalize(normal);
  
  vertex_normals[id * 3 + 0] = (float4)(normal, 0);
  vertex_normals[id * 3 + 1] = (float4)(normal, 0);
  vertex_normals[id * 3 + 2] = (float4)(normal, 0);
}

kernel void generate_smooth_normals(global float4* vertex_positions, global float4* vertex_normals, global float4* metaball_positions, int num_metaballs) {
  
  const float METABALL_SIZE = 10;
  
  int id = get_global_id(0);
  
  float3 normal = (float3)(0,0,0);
  for(int i = 0; i < num_metaballs; i++) {
  
  float dist = distance(vertex_positions[id].xyz, metaball_positions[i].xyz) / METABALL_SIZE;
  float amount = 1-smoothstepmap( clamp(dist, 0.0f, 1.0f) );
  
    normal += (vertex_positions[id].xyz - metaball_positions[i].xyz) * amount;
  }
  
  normal = normalize(normal);
  vertex_normals[id] = (float4)(normal, 0);
}
