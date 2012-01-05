#include "corange.h"

#include "spline.h"

spline* spline_new() {
  spline* s = malloc(sizeof(spline));
  s->num_points = 0;
  s->y0d = 0;
  s->ynd = 0;
  s->x0d = 0;
  s->xnd = 0;
  return s;
}

void spline_delete(spline* s) {
  free(s);
}

void spline_add_point(spline* s, vector2 p) {
  if (s->num_points == 20) {
    warning("Spline already contains maximum 20 points");
    return;
  }
  
  s->x[s->num_points] = p.x;
  s->y[s->num_points] = p.y;
  s->num_points++;
}

vector2 spline_get_point(spline* s, int i) {
  return v2(s->x[i], s->y[i]);
}

void spline_set_point(spline* s, int i, vector2 p) {
  s->x[i] = p.x;
  s->y[i] = p.y;
}

void spline_print(spline* s) {
  printf("Spline: ");
  int i;
  for (i=0; i < s->num_points; i++) {
    printf("(%0.2f, %0.2f | %0.2f) ", s->x[i], s->y[i], s->yd[i]);
  }
  printf("\n");
}

static void spline_update_y(spline* s) {
  
  int n = s->num_points;
  float* u = malloc(sizeof(float) * n);
  
  float yp0 = s->y0d;
  float ypn = s->ynd;
  
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

static void spline_update_x(spline* s) {
  
  int n = s->num_points;
  float* u = malloc(sizeof(float) * n);
  
  float xp0 = s->x0d;
  float xpn = s->xnd;
  
  if(xp0 > 999999) {
    s->xd[0] = 0.0;
    u[0] = 0.0;
  } else {
    s->xd[0] = -0.5;
    u[0] = (3.0 / (s->y[1] - s->y[0])) * 
           ((s->x[1] - s->x[0]) / 
            (s->y[1] - s->y[0]) - xp0);
  }
  
  int i;
  for(i = 1; i < n-1; i++) {
    float sig = (s->y[i] - s->y[i-1]) / (s->y[i+1] - s->y[i-1]);
    float p = sig * s->xd[i-1] + 2.0;
    s->xd[i] = (sig - 1.0) / p;
    u[i] = (s->x[i+1] - s->x[i]) / (s->y[i+1] - s->y[i]) - 
           (s->x[i] - s->x[i-1]) / (s->y[i] - s->y[i-1]);
    u[i] = (6.0 * u[i] / (s->y[i+1] - s->y[i-1]) - sig * u[i-1]) / p;
  }
  
  float qn, un;
  if (xpn > 9999999) {
    qn = 0.0;
    un = 0.0;
  } else {
    qn = 0.5;
    un = (3.0 / (s->y[n-1] - s->y[n-2])) * 
          (xpn - (s->x[n-1] - s->x[n-2]) / 
                 (s->y[n-1] - s->y[n-2]));
  }
  
  s->xd[n-1] = (un - qn*u[n-2]) / (qn * s->xd[n-2] + 1.0);
  int k;
  for(k = n-2; k >= 0; k--) {
    s->xd[k] = s->xd[k] * s->xd[k+1] + u[k];
  }
  
  free(u);
}

void spline_update(spline* s) {
  spline_update_y(s);
  spline_update_x(s);
}

float spline_get_y(spline* s, float x) {
  
  int n = s->num_points;
  
  int k;
  int klo = 0;
  int khi = n-1;
  while(khi - klo > 1) {
    k = (khi+klo) >> 1;
    if (s->x[k] > x) khi =k;
    else klo = k;
  }
  
  float h = s->x[khi] - s->x[klo];
  if (h == 0.0) {
    error("Can't interpolate spline. Bad slope.");
  }
  
  float a = (s->x[khi] - x)/h;
  float b = (x - s->x[klo])/h;
  
  float y = a * s->y[klo] + b * s->y[khi] + 
            ((a*a*a-a) * s->yd[klo] + (b*b*b-b) * s->yd[khi]) * (h*h)/6.0;
            
  return y;
}

float spline_get_x(spline* s, float y) {
  
  int n = s->num_points;
  
  int k;
  int klo = 0;
  int khi = n-1;
  while(khi - klo > 1) {
    k = (khi+klo) >> 1;
    if (s->y[k] > y) khi =k;
    else klo = k;
  }
  
  float h = s->y[khi] - s->y[klo];
  if (h == 0.0) {
    error("Can't interpolate spline. Bad slope.");
  }
  
  float a = (s->y[khi] - y)/h;
  float b = (y - s->y[klo])/h;
  
  float x = a * s->x[klo] + b * s->x[khi] + 
            ((a*a*a-a) * s->xd[klo] + (b*b*b-b) * s->xd[khi]) * (h*h)/6.0;
            
  return x;
}

void spline_render(spline* s, vector2 position, vector2 size, int increments) {

	glMatrixMode(GL_PROJECTION);
  glPushMatrix();
	glLoadIdentity();
	glOrtho(0, viewport_width(), viewport_height(), 0, -1, 1);
  
	glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
	glLoadIdentity();
  
  glColor4f(1,1,1,1);
  glPointSize(4);
  glEnable(GL_POINT_SMOOTH);
  
  glBegin(GL_POINTS);
    
    int i;
    for(i = 0; i < s->num_points; i++) {
      vector2 loc = v2( s->x[i], s->y[i] );
      glVertex2f(position.x + loc.x * size.x, position.y + loc.y * size.y );
    }
  
  glEnd();
  
  glDisable(GL_POINT_SMOOTH);
  glPointSize(1);
  
  glBegin(GL_LINE_STRIP);
    
    float j;
    float step = 1.0 / increments;
    float loc;
    for( j = 0; j <= 1; j += step) {
      loc = spline_get_y(s, j);
      glVertex2f(position.x + j * size.x, position.y + loc * size.y);
    }
    float y = s->y[s->num_points-1];
    float x = s->x[s->num_points-1];
    glVertex2f(position.x + x * size.x, position.y + y * size.y);
  
  glEnd();
  
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  
	glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  
}

color_curves* acv_load_file(char* filename) {

  char* contents = asset_file_contents(filename);
  
  int version = contents[0] | contents[1];
  int count = contents[2] | contents[3];
  
  if (count != 5) {
    error("acv file %s. Doesn't contain 5 curves.", filename);
  }
  
  int i;
  int pos = 4;
  
  spline* rgb_curve = spline_new();
  spline* r_curve = spline_new();
  spline* g_curve = spline_new();
  spline* b_curve = spline_new();
  spline* a_curve = spline_new();
  
  rgb_curve->num_points = contents[pos] | contents[pos+1];
  rgb_curve->y0d = 1; rgb_curve->ynd = 1; rgb_curve->x0d = 1; rgb_curve->xnd = 1;
  pos += 2;
  for(i = 0; i < rgb_curve->num_points; i++) {
    int x = contents[pos] | contents[pos+1];
    int y = contents[pos+2] | contents[pos+3];
    rgb_curve->x[i] = (float)x / 255;
    rgb_curve->y[i] = (float)y / 255;
    pos += 4;
  }
  
  r_curve->num_points = contents[pos] | contents[pos+1];
  r_curve->y0d = 1; r_curve->ynd = 1; r_curve->x0d = 1; r_curve->xnd = 1;
  pos += 2;
  for(i = 0; i < r_curve->num_points; i++) {
    int x = contents[pos] | contents[pos+1];
    int y = contents[pos+2] | contents[pos+3];
    r_curve->x[i] = (float)x / 255;
    r_curve->y[i] = (float)y / 255;
    pos += 4;
  }
  
  g_curve->num_points = contents[pos] | contents[pos+1];
  g_curve->y0d = 1; g_curve->ynd = 1; g_curve->x0d = 1; g_curve->xnd = 1;
  pos += 2;
  for(i = 0; i < g_curve->num_points; i++) {
    int x = contents[pos] | contents[pos+1];
    int y = contents[pos+2] | contents[pos+3];
    g_curve->x[i] = (float)x / 255;
    g_curve->y[i] = (float)y / 255;
    pos += 4;
  }
  
  b_curve->num_points = contents[pos] | contents[pos+1];
  b_curve->y0d = 1; b_curve->ynd = 1; b_curve->x0d = 1; b_curve->xnd = 1;
  pos += 2;
  for(i = 0; i < b_curve->num_points; i++) {
    int x = contents[pos] | contents[pos+1];
    int y = contents[pos+2] | contents[pos+3];
    b_curve->x[i] = (float)x / 255;
    b_curve->y[i] = (float)y / 255;
    pos += 4;
  }
  
  a_curve->num_points = contents[pos] | contents[pos+1];
  a_curve->y0d = 1; a_curve->ynd = 1; a_curve->x0d = 1; a_curve->xnd = 1;
  pos += 2;
  for(i = 0; i < a_curve->num_points; i++) {
    int x = contents[pos] | contents[pos+1];
    int y = contents[pos+2] | contents[pos+3];
    a_curve->x[i] = (float)x / 255;
    a_curve->y[i] = (float)y / 255;
    pos += 4;
  }
  
  free(contents);
  
  spline_update(rgb_curve);
  spline_update(r_curve);
  spline_update(g_curve);
  spline_update(b_curve);
  spline_update(a_curve);

  color_curves* curves = malloc(sizeof(color_curves));
  curves->rgb_spline = rgb_curve;
  curves->r_spline = r_curve;
  curves->g_spline = g_curve;
  curves->b_spline = b_curve;
  curves->a_spline = a_curve;
  
  return curves;
  
}

void color_curves_delete(color_curves* cc) {
  
  spline_delete(cc->rgb_spline);
  spline_delete(cc->r_spline);
  spline_delete(cc->g_spline);
  spline_delete(cc->b_spline);
  spline_delete(cc->a_spline);
  
  free(cc);
}

void color_curves_write_lut(color_curves* cc, char* filename) {

  int lut_size = 64;
  
  unsigned char* lut_data = malloc(sizeof(char) * 3 * lut_size * lut_size * lut_size);
  
  int r, g, b;
  for(r = 0; r < lut_size; r++)
  for(g = 0; g < lut_size; g++)
  for(b = 0; b < lut_size; b++) {
    
    int i = (3 * r) + (3 * lut_size * g) + (3 * lut_size * lut_size * b);
    
    float red = (float)r / lut_size;
    float green = (float)g / lut_size;
    float blue = (float)b / lut_size;
    
    red = spline_get_x(cc->r_spline, red);
    green = spline_get_x(cc->g_spline, green);
    blue = spline_get_x(cc->b_spline, blue);
    
    red = spline_get_x(cc->rgb_spline, red);
    green = spline_get_x(cc->rgb_spline, green);
    blue = spline_get_x(cc->rgb_spline, blue);
    
    lut_data[i] = (unsigned char) (red * 255);
    lut_data[i+1] = (unsigned char) (green * 255);
    lut_data[i+2] = (unsigned char) (blue * 255);
  
  }
  
  SDL_RWops* file = SDL_RWFromFile(filename, "wb");
  SDL_RWwrite(file, "CORANGE-LUT", sizeof("CORANGE-LUT"), 1);
  SDL_RWwrite(file, &lut_size, sizeof(char) * 2, 1);
  SDL_RWwrite(file, lut_data, sizeof(char) * 3 * lut_size * lut_size * lut_size, 1);
  SDL_RWclose(file);
  
  free(lut_data);

}
