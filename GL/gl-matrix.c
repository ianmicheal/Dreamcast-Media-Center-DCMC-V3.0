#include "gl.h"      // OpenGL API

/******************************************************************************/
/* A good bit of this has been adapted from KGL */

/* Viewport mapping */
GLfloat gl_viewport_scale[3];
GLfloat gl_viewport_offset[3];

/* Depth range */
GLclampf gl_depthrange_near;
GLclampf gl_depthrange_far;

/* Viewport size */
GLint gl_viewport_x1, gl_viewport_y1,
gl_viewport_width, gl_viewport_height;

/* Frustum attributes */
typedef struct {
	float left, right, bottom, top, znear, zfar;
} gl_frustum_t;

static gl_frustum_t gl_frustum;

static GLsizei gl_screen_width, gl_screen_height;

/* This Matrix will contain the GL base stack */
static matrix4f Matrix[6] __attribute__((aligned(32)));
static GLsizei MatrixMode = 0;

/* This Matrix will contain the GL push/pop stack ( fixed size, 32 matrices )*/
static const GLsizei MAX_MATRICES = 32;
static matrix4f MatrixStack[32] __attribute__((aligned(32)));
static GLsizei  StackPos = 0;
static matrix_t FrustumMatrix __attribute__((aligned(32))) =
{
      { 0.0f, 0.0f, 0.0f, 0.0f },
      { 0.0f, 0.0f, 0.0f, 0.0f },
      { 0.0f, 0.0f, 0.0f, -1.0f },
      { 0.0f, 0.0f, 0.0f, 0.0f }
};

static matrix4f OrthoMatrix __attribute__((aligned(32))) =
{
      { 0.0f, 0.0f, 0.0f, 0.0f },
      { 0.0f, 0.0f, 0.0f, 0.0f },
      { 0.0f, 0.0f, 0.0f, 0.0f },
      { 0.0f, 0.0f, 0.0f, 1.0f }
};

void glMatrixMode( GLenum mode )
{
    if(mode>=GL_SCREENVIEW && mode<=GL_IDENTITY)
       MatrixMode = mode;
}

void glLoadIdentity()
{
    mat_load( Matrix + GL_IDENTITY );
    mat_store( Matrix + MatrixMode );
}

void glPushMatrix()
{
    if(StackPos<MAX_MATRICES-1)
        mat_store( MatrixStack + StackPos++ );
}

void glPopMatrix()
{
    if(StackPos)
    {
        mat_load( MatrixStack + --StackPos );
        mat_store( Matrix + MatrixMode );
    }
}

void glTranslatef( GLfloat x, GLfloat y, GLfloat z )
{
    mat_load( Matrix + MatrixMode );
    mat_translate( x, y, z );
    mat_store( Matrix + MatrixMode );
}

void glScalef( GLfloat x, GLfloat y, GLfloat z )
{
    mat_load( Matrix + MatrixMode );
    mat_scale( x, y, z );
    mat_store( Matrix + MatrixMode );
}

void glRotatef( GLfloat angle, GLfloat x, GLfloat  y, GLfloat z )
{
    float r = DEG2RAD*angle;
    mat_load( Matrix + MatrixMode );
    mat_rotate( r*x, r*y, r*z );
    mat_store( Matrix + MatrixMode );
}

/* Set the depth range */
void glDepthRange(GLclampf n, GLclampf f)
{
    /* clamp the values... */
    if (n < 0.0f) n = 0.0f;
    else if (n > 1.0f) n = 1.0f;
    if (f < 0.0f) f = 0.0f;
    else if (f > 1.0f) f = 1.0f;

    gl_depthrange_near = n;
    gl_depthrange_far = f;

    /* Adjust the viewport scale and offset for Z */
    gl_viewport_scale[2] = ((f - n) / 2.0f);
    gl_viewport_offset[2] = (n + f) / 2.0f;
}


/* Set the GL viewport */
void glViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
    gl_viewport_x1 = x;
    gl_viewport_y1 = y;
    gl_viewport_width = width;
    gl_viewport_height = height;

    /* Calculate the viewport scale and offset */
    gl_viewport_scale[0] = (GLfloat)width / 2.0f;
    gl_viewport_offset[0] = gl_viewport_scale[0] + (GLfloat)x;
    gl_viewport_scale[1] = (GLfloat)height / 2.0f;
    gl_viewport_offset[1] = gl_viewport_scale[1] + (GLfloat)y;
    gl_viewport_scale[2] = (gl_depthrange_far - gl_depthrange_near) / 2.0f;
    gl_viewport_offset[2] = (gl_depthrange_near + gl_depthrange_far) / 2.0f;

    gl_viewport_offset[2] += 0.0001f;
}

void glFrustum(GLfloat left, GLfloat right,
               GLfloat bottom, GLfloat top,
               GLfloat znear, GLfloat zfar)
{
    gl_frustum.left = left;
    gl_frustum.right = right;
    gl_frustum.bottom = bottom;
    gl_frustum.top = top;
    gl_frustum.znear = znear;
    gl_frustum.zfar = zfar;

    FrustumMatrix[0][0] = (2.0f * znear) / (right - left);
    FrustumMatrix[2][0] = (right + left) / (right - left);
    FrustumMatrix[1][1] = (2.0f * znear) / (top - bottom);
    FrustumMatrix[2][1] = (top + bottom) / (top - bottom);
    FrustumMatrix[2][2] = zfar/(zfar - znear);
    FrustumMatrix[3][2] = -(zfar * znear) / (zfar - znear);

    mat_load( Matrix + MatrixMode );
    mat_apply(&FrustumMatrix);
    mat_store( Matrix + MatrixMode );
}
        
void glOrtho(GLfloat left, GLfloat right,
             GLfloat bottom, GLfloat top,
             GLfloat znear, GLfloat zfar)
{
    gl_frustum.znear = 0.001;
    gl_frustum.zfar = 100;

    OrthoMatrix[0][0] = 2.0f / (right - left);
    OrthoMatrix[1][1] = 2.0f / (top - bottom);
    OrthoMatrix[2][2] = -2.0f / (zfar - znear);
    OrthoMatrix[3][0] = -(right + left) / (right - left);;
    OrthoMatrix[3][1] = -(top + bottom) / (top - bottom);
    OrthoMatrix[3][2] = -(zfar + znear) / (zfar - znear);

    mat_load( Matrix + MatrixMode );
    mat_apply(&OrthoMatrix);
    mat_store( Matrix + MatrixMode );
}

void gluPerspective( GLfloat angle, GLfloat aspect,
                     GLfloat znear, GLfloat zfar )
{

    GLfloat xmin, xmax, ymin, ymax;

    ymax = znear * ftan(angle * F_PI / 360.0f);
    ymin = -ymax;
    xmin = ymin * aspect;
    xmax = ymax * aspect;

    glFrustum(xmin, xmax, ymin, ymax, znear, zfar);
}

/* XXX - these should be in glu */
void normalize(float v[3])
{
    float r;

    r = fsqrt( v[0]*v[0] + v[1]*v[1] + v[2]*v[2] );
    if (r == 0.0) return;

    v[0] /= r;
    v[1] /= r;
    v[2] /= r;
}

void cross(float v1[3], float v2[3], float result[3])
{
    result[0] = v1[1]*v2[2] - v1[2]*v2[1];
    result[1] = v1[2]*v2[0] - v1[0]*v2[2];
    result[2] = v1[0]*v2[1] - v1[1]*v2[0];
}
static matrix_t ml __attribute__((aligned(32))) = {
            { 1.0f, 0.0f, 0.0f, 0.0f },
            { 0.0f, 1.0f, 0.0f, 0.0f },
            { 0.0f, 0.0f, 1.0f, 0.0f },
            { 0.0f, 0.0f, 0.0f, 1.0f }
        };

/* XXX - should move to glu */
void gluLookAt(GLfloat eyex, GLfloat eyey, GLfloat eyez, GLfloat centerx,
               GLfloat centery, GLfloat centerz, GLfloat upx, GLfloat upy,
               GLfloat upz)
{
    float forward[3], side[3], up[3];

    forward[0] = centerx - eyex;
    forward[1] = centery - eyey;
    forward[2] = centerz - eyez;

    up[0] = upx;
    up[1] = upy;
    up[2] = upz;

    normalize(forward);

    /* Side = forward x up */
    cross(forward, up, side);
    normalize(side);

    /* Recompute up as: up = side x forward */
    cross(side, forward, up);

    ml[0][0] = side[0];
    ml[1][0] = side[1];
    ml[2][0] = side[2];

    ml[0][1] = up[0];
    ml[1][1] = up[1];
    ml[2][1] = up[2];

    ml[0][2] = -forward[0];
    ml[1][2] = -forward[1];
    ml[2][2] = -forward[2];

    mat_load( Matrix + MatrixMode );
    mat_apply(&ml);
    mat_store( Matrix + MatrixMode );

    glTranslatef(-eyex, -eyey, -eyez);
}

/* Use KOS Matrix Function mat_lookat() 
void glulookat()
{
mat_lookat( vector3f, vector3f, vector3f );
}
*/

void glKosMatrixApplyRender()
{
    mat_load( Matrix + GL_IDENTITY );
    mat_apply( Matrix + GL_SCREENVIEW );
    mat_apply( Matrix + GL_PROJECTION );  
    mat_apply( Matrix + GL_MODELVIEW );  
}

extern void glInitRender();

void glKosInit()
{
    glInitRender();
    
    mat_identity();
    mat_store( Matrix + GL_SCREENVIEW );
    mat_store( Matrix + GL_PROJECTION );
    mat_store( Matrix + GL_MODELVIEW );
    mat_store( Matrix + GL_TEXTURE );
    mat_store( Matrix + GL_IDENTITY );

    gl_screen_width = (GLsizei)vid_mode->width;
    gl_screen_height = (GLsizei)vid_mode->height;

    glDepthRange(0.0f, 1.0f);
    glViewport(0, 0, gl_screen_width, gl_screen_height);
       
    Matrix[GL_SCREENVIEW][0][0] = gl_viewport_scale[0];
    Matrix[GL_SCREENVIEW][1][1] = -gl_viewport_scale[1];
    Matrix[GL_SCREENVIEW][3][0] = gl_viewport_offset[0];
    Matrix[GL_SCREENVIEW][3][1] = gl_screen_height - gl_viewport_offset[1];
}

/******************************************************************************/
