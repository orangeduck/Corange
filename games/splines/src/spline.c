#include "corange.h"

#include "spline.h"

void spline_print(spline* s) {
  printf("Spline: ");
  int i;
  for (i=0; i < s->num_points; i++) {
    printf("(%0.2f, %0.2f | %0.2f) ", s->x[i], s->y[i], s->yd[i]);
  }
  printf("\n");
}

void spline_update(spline* s) {
  
  int n = s->num_points;
  float* u = malloc(sizeof(float) * n);
  
  float yp0 = 1;
  float ypn = 1;
  
  if(yp0 > 999999) {
    s->yd[0] = 0.0;
    u[0] = 0.0;
  } else {
    s->yd[0] = -0.5;
    u[0] = (3.0 / (s->x[1] - s->x[0])) * 
           ((s->y[1] - s->y[0]) / 
            (s->x[1] - s->x[0]) - yp0);
  }
  
  int i;
  for(i = 1; i < n-1; i++) {
    float sig = (s->x[i] - s->x[i-1]) / (s->x[i+1] - s->x[i-1]);
    float p = sig * s->yd[i-1] + 2.0;
    s->yd[i] = (sig - 1.0) / p;
    u[i] = (s->y[i+1] - s->y[i]) / (s->x[i+1] - s->x[i]) - 
           (s->y[i] - s->y[i-1]) / (s->x[i] - s->x[i-1]);
    u[i] = (6.0 * u[i] / (s->x[i+1] - s->x[i-1]) - sig * u[i-1]) / p;
  }
  
  float qn, un;
  if (ypn > 9999999) {
    qn = 0.0;
    un = 0.0;
  } else {
    qn = 0.5;
    un = (3.0 / (s->x[n-1] - s->x[n-2])) * 
          (ypn - (s->y[n-1] - s->y[n-2]) / 
                 (s->x[n-1] - s->x[n-2]));
  }
  
  s->yd[n-1] = (un - qn*u[n-2]) / (qn * s->yd[n-2] + 1.0);
  int k;
  for(k = n-2; k >= 0; k--) {
    s->yd[k] = s->yd[k] * s->yd[k+1] + u[k];
  }
  
  free(u);
}

float spline_get_y(spline* s, float x) {
  
  int n = s->num_points;
  
  int klo = 0;
  int khi = n-1;
  while ( (x > s->x[klo]) && (x > s->x[klo+1]) ) {
    klo++;
  }
  while ( (x < s->x[khi]) && (x < s->x[khi-1]) ) {
    khi--;
  }
  
  float h = s->x[khi] - s->x[klo];
  if (h == 0.0) {
    printf("SOMETHING WENT WRONG!\n");
    exit(EXIT_FAILURE);
  }
  
  float a = (s->x[khi] - x)/h;
  float b = (x - s->x[klo])/h;
  
  float y = a * s->y[klo] + b * s->y[khi] + 
            ((a*a*a-a) * s->yd[klo] + (b*b*b-b) * s->yd[khi]) * (h*h)/6.0;
            
  return y;
} 

void write_lut_file( char* filename, char* output ) {
   
  unsigned char* contents = asset_file_contents(filename);
  
  int version = contents[0] | contents[1];
  int count = contents[2] | contents[3];
  
  //printf("version: %i curve count: %i\n", version, count);
  
  if (count != 5) {
    printf("Error: Can't load acv file %s. Doesn't contain 5 curves.\n", filename);
    exit(EXIT_FAILURE);
  }
  
  int i;
  int pos = 4;
  
  spline rgb_curve;
  spline r_curve;
  spline g_curve;
  spline b_curve;
  spline a_curve;
  
  rgb_curve.num_points = contents[pos] | contents[pos+1];
  pos += 2;
  for(i = 0; i < rgb_curve.num_points; i++) {
    int x = contents[pos] | contents[pos+1];
    int y = contents[pos+2] | contents[pos+3];
    rgb_curve.x[i] = (float)x / 255;
    rgb_curve.y[i] = (float)y / 255;
    pos += 4;
  }
  
  r_curve.num_points = contents[pos] | contents[pos+1];
  pos += 2;
  for(i = 0; i < r_curve.num_points; i++) {
    int x = contents[pos] | contents[pos+1];
    int y = contents[pos+2] | contents[pos+3];
    r_curve.x[i] = (float)x / 255;
    r_curve.y[i] = (float)y / 255;
    pos += 4;
  }
  
  g_curve.num_points = contents[pos] | contents[pos+1];
  pos += 2;
  for(i = 0; i < g_curve.num_points; i++) {
    int x = contents[pos] | contents[pos+1];
    int y = contents[pos+2] | contents[pos+3];
    g_curve.x[i] = (float)x / 255;
    g_curve.y[i] = (float)y / 255;
    pos += 4;
  }
  
  b_curve.num_points = contents[pos] | contents[pos+1];
  pos += 2;
  for(i = 0; i < b_curve.num_points; i++) {
    int x = contents[pos] | contents[pos+1];
    int y = contents[pos+2] | contents[pos+3];
    b_curve.x[i] = (float)x / 255;
    b_curve.y[i] = (float)y / 255;
    pos += 4;
  }
  
  a_curve.num_points = contents[pos] | contents[pos+1];
  pos += 2;
  for(i = 0; i < a_curve.num_points; i++) {
    int x = contents[pos] | contents[pos+1];
    int y = contents[pos+2] | contents[pos+3];
    a_curve.x[i] = (float)x / 255;
    a_curve.y[i] = (float)y / 255;
    pos += 4;
  }
  
  free(contents);
  
  spline_update(&rgb_curve);
  spline_update(&r_curve);
  spline_update(&g_curve);
  spline_update(&b_curve);
  spline_update(&a_curve);
  
  int lut_size = 128;
  
  unsigned char* lut_data = malloc(sizeof(char) * 3 * lut_size * lut_size * lut_size);
  
  int r, g, b;
  for(r = 0; r < lut_size; r++)
  for(g = 0; g < lut_size; g++)
  for(b = 0; b < lut_size; b++) {
    
    int i = (3 * r) + (3 * lut_size * g) + (3 * lut_size * lut_size * b);
    
    float red = (float)r / lut_size;
    float green = (float)g / lut_size;
    float blue = (float)b / lut_size;
    
    red = spline_get_y(&r_curve, red);
                       
    green = spline_get_y(&g_curve, green);
                         
    blue = spline_get_y(&b_curve, blue);
    
    lut_data[i] = (unsigned char) (red * 255);
    lut_data[i+1] = (unsigned char) (green * 255);
    lut_data[i+2] = (unsigned char) (blue * 255);
  
  }
  
  SDL_RWops* file = SDL_RWFromFile(output, "wb");
  SDL_RWwrite(file, "CORANGE-LUT", sizeof("CORANGE-LUT"), 1);
  SDL_RWwrite(file, &lut_size, sizeof(char) * 2, 1);
  SDL_RWwrite(file, lut_data, sizeof(char) * 3 * lut_size * lut_size * lut_size, 1);
  SDL_RWclose(file);
  
  free(lut_data);
  
}

void spline_render(spline* s) {

	glMatrixMode(GL_PROJECTION);
  glPushMatrix();
	glLoadIdentity();
	glOrtho(0, viewport_width(), viewport_height(), 0, -1, 1);
  
	glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
	glLoadIdentity();
  
  glColor4f(1,1,1,1);
  
  glBegin(GL_POINTS);
  

  float i;
  for( i = 0; i < 1; i += 0.001) {
    float position = spline_get_y(s, i);
    glVertex2f(position * 400, i * 400);
  }
  
  glEnd();
  
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  
	glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  
}