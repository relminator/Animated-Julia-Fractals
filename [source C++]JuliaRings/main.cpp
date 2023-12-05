/**************************
 * 
 * Julia Rings
 *
 * Richard Eric M. Lope BSN RN
 * http://rel.phatcode.net
 * 
 * Compile against these libs
 * -lopengl32 -lglu32 -lglaux -lgdi32
 * 
 **************************/

#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include <gl/glext.h>
#include <math.h>
#include <stdlib.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

#define BUFFER_WIDTH 320
#define BUFFER_HEIGHT 240

#define TEXTURE_WIDTH 512
#define TEXTURE_HEIGHT 512

#define PI 3.141593

#define MAXITER  20
#define MAXSIZE  4


unsigned int buffer[TEXTURE_WIDTH * TEXTURE_HEIGHT];
unsigned int vpage[BUFFER_WIDTH * BUFFER_HEIGHT];

GLuint textureID;

static float lx[BUFFER_WIDTH];
static float ly[BUFFER_WIDTH];

int frame;

/**************************
 * Function Declarations
 *
 **************************/

LRESULT CALLBACK WndProc (HWND hWnd, UINT message,
WPARAM wParam, LPARAM lParam);
void EnableOpenGL (HWND hWnd, HDC *hDC, HGLRC *hRC);
void DisableOpenGL (HWND hWnd, HDC hDC, HGLRC hRC);

void glInit();
void glBegin2D();
void glEnd2D();


void initJulia()
{
	float xmin = -2.0;
    float xmax =  2.0;
    float ymin = -1.5;
    float ymax =  1.5;
    
	float deltax = (xmax - xmin) / (float)(BUFFER_WIDTH - 1);
    float deltay = (ymax - ymin) / (float)(BUFFER_HEIGHT - 1);
    
    for( int i = 0; i < BUFFER_WIDTH; i++)
        lx[i] = xmin + i * deltax;
    
    for( int i = 0; i < BUFFER_HEIGHT; i++)
        ly[i] = ymax - i * deltay;
    
}

void drawJulia()
{
	unsigned int *p_buffer = vpage;
	unsigned int *p_bufferl = &vpage[(BUFFER_WIDTH * BUFFER_HEIGHT) - 1];
	
	frame +=5;
	float theta = frame * PI / 180.0f;

	float p = cos(theta) * sin(theta * 0.7f);
	float q = sin(theta) + sin(theta);
	p = p * 0.8f;
	q = q * 0.6f;

	float cmag = sqrt(p *p + q* q);
	float cmagsq = (p *p + q* q);
	float drad = 0.04f;
	float drad_L = (cmag - drad);
	float drad_H = (cmag + drad);
	
	drad_L = drad_L * drad_L;
	drad_H = drad_H * drad_H;

	int red, grn, blu;
	
	for( int py = 0; py < (BUFFER_HEIGHT >> 1); py++ )
	{
		float ty = ly[py];
		for( int px = 0; px < BUFFER_WIDTH; px++ )
		{
			float x = lx[px];
			float y = ty;
			float xsquare = 0;
			float ysquare = 0;
			float ztot = 0;
			int i = 0;
			int i_last = 0;
			while (i < MAXITER && ( xsquare + ysquare ) < MAXSIZE)
			{
				xsquare = x * x;
				ysquare = y * y;
				float ytemp = x * y * 2;
				x = xsquare - ysquare + p;
				y = ytemp + q;
				float zmag = (x * x + y * y);
				if (zmag < drad_H && zmag > drad_L && i > 0)
				{
					ztot = ztot + ( 1 - (fabs(zmag - cmagsq) / drad));
					i_last = i;
				}

				i++;
				if (zmag > 4.0) break;
			}

			if (ztot > 0)
				i = (int)(sqrt(ztot) * 500);
			else
				i = 0;
								
			red = i;
			if(red > 255) red = 255;
			
			if (i < 512 && i > 255)
			grn = i - 256;
			else
			{
			if (i >= 512) 
			  grn = 255;
			else
			  grn = 0;                   
			}

			if (i <= 768 && i > 511)
			blu = i - 512;
			else
			{
			if (i >= 768) 
			  blu = 255;
			else
			  blu = 0;                   
			}

			int tmp = ((red+grn+blu)/3);
			tmp = tmp & 0xFF;
			red = ((red+grn+tmp)/3);
			red = red & 0xFF;
			grn = ((grn+blu+tmp)/3);
			grn = grn & 0xFF;
			blu = ((blu+red+tmp)/3);
			blu = blu & 0xFF;

			switch(i_last % 3)
			{
				case 1:
				  tmp = red;
				  red = grn;
				  grn = blu;
				  blu = tmp;
				  break;
				case 2:
				  tmp = red;
				  blu = grn;
				  red = blu;
				  grn = tmp;
				  break;
			}

			int pixel = red << 24 | grn << 16 | blu << 8 | 255 ;
			*p_buffer = pixel;
			*p_bufferl = pixel;
			p_buffer++;
			p_bufferl--;
		}
	}
	
	
	// copy to texture
	p_buffer = vpage;
	p_bufferl = buffer; 

	for( int py = 0; py < (BUFFER_HEIGHT); py++)
	{
		for( int px = 0; px < BUFFER_WIDTH; px++)
		{
			*p_bufferl++ = *p_buffer++;
		}
		p_bufferl += TEXTURE_WIDTH - BUFFER_WIDTH;
	}
	
}


/**************************
 * WinMain
 *
 **************************/

int WINAPI WinMain (HINSTANCE hInstance,
                    HINSTANCE hPrevInstance,
                    LPSTR lpCmdLine,
                    int iCmdShow)
{
    WNDCLASS wc;
    HWND hWnd;
    HDC hDC;
    HGLRC hRC;        
    MSG msg;
    BOOL bQuit = FALSE;
    float theta = 0.0f;

    /* register window class */
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon (NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor (NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH) GetStockObject (BLACK_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = "OpenGL Julia Fractal";
    RegisterClass (&wc);

	initJulia();
	

    /* create main window */
    hWnd = CreateWindow (
      "OpenGL Julia Fractal", "OpenGL Julia Fractal", 
      WS_CAPTION | WS_POPUPWINDOW | WS_VISIBLE,
      0, 0, SCREEN_WIDTH, SCREEN_HEIGHT,
      NULL, NULL, hInstance, NULL);

    /* enable OpenGL for the window */
    EnableOpenGL (hWnd, &hDC, &hRC);
	glInit();
	
    /* program main loop */
    while (!bQuit)
    {
        /* check for messages */
        if (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE))
        {
            /* handle or dispatch messages */
            if (msg.message == WM_QUIT)
            {
                bQuit = TRUE;
            }
            else
            {
                TranslateMessage (&msg);
                DispatchMessage (&msg);
            }
        }
        else
        {
            /* OpenGL animation code goes here */

			drawJulia();
			
            glClearColor (0.0f, 0.0f, 0.0f, 0.0f);
            glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
			
			glPolygonMode(GL_FRONT, GL_FILL);		// need to fill the quad
			glPolygonMode(GL_BACK, GL_FILL);		// Just to make sure even if you messed up your winding
			glDisable (GL_BLEND); 					// ditch blending
			glDisable (GL_DEPTH_TEST);				// as well as distance testing
			glEnable  (GL_TEXTURE_2D);				// enable texturing ot we don't see anything
			
			
			// Upload the 512 x 512 buffer to the GPU
			// You can also use glTexImage2D() 
			glBindTexture( GL_TEXTURE_2D, textureID );
			glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, 
						     TEXTURE_WIDTH, TEXTURE_HEIGHT, 
							 GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, 
							 buffer );
			
			glBegin2D();
			int x2 = SCREEN_WIDTH;
			int y2 = SCREEN_HEIGHT;
			
			float sx = 0;
			float sy = 0;
			float sw = BUFFER_WIDTH/(float)TEXTURE_WIDTH;
			float sh = BUFFER_HEIGHT/(float)TEXTURE_HEIGHT;
    
			glColor4f(1,1,1,1) ;
			
			glBegin( GL_QUADS );
				glTexCoord2f(sx, sy);   glVertex2i(0, y2 - 1);
				glTexCoord2f(sw, sy);   glVertex2i(x2 - 1, y2 - 1);
				glTexCoord2f(sw, sh);   glVertex2i(x2 - 1, 0); 
				glTexCoord2f(sx, sh);   glVertex2i(0,0);    
			glEnd();
			
			glEnd2D();
			
			SwapBuffers (hDC);

            theta += 1.0f;
            Sleep(14);
        }
    }

	glDeleteTextures( 1, &textureID );
	
    /* shutdown OpenGL */
    DisableOpenGL (hWnd, hDC, hRC);

    /* destroy the window explicitly */
    DestroyWindow (hWnd);

    return msg.wParam;
}


/********************
 * Window Procedure
 *
 ********************/

LRESULT CALLBACK WndProc (HWND hWnd, UINT message,
                          WPARAM wParam, LPARAM lParam)
{

    switch (message)
    {
    case WM_CREATE:
        return 0;
    case WM_CLOSE:
        PostQuitMessage (0);
        return 0;

    case WM_DESTROY:
        return 0;

    case WM_KEYDOWN:
        switch (wParam)
        {
        case VK_ESCAPE:
            PostQuitMessage(0);
            return 0;
        }
        return 0;

    default:
        return DefWindowProc (hWnd, message, wParam, lParam);
    }
}


/*******************
 * Enable OpenGL
 *
 *******************/

void EnableOpenGL (HWND hWnd, HDC *hDC, HGLRC *hRC)
{
    PIXELFORMATDESCRIPTOR pfd;
    int iFormat;

    /* get the device context (DC) */
    *hDC = GetDC (hWnd);

    /* set the pixel format for the DC */
    ZeroMemory (&pfd, sizeof (pfd));
    pfd.nSize = sizeof (pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | 
      PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 16;
    pfd.iLayerType = PFD_MAIN_PLANE;
    iFormat = ChoosePixelFormat (*hDC, &pfd);
    SetPixelFormat (*hDC, iFormat, &pfd);

    /* create and enable the render context (RC) */
    *hRC = wglCreateContext( *hDC );
    wglMakeCurrent( *hDC, *hRC );

}


/******************
 * Disable OpenGL
 *
 ******************/

void DisableOpenGL (HWND hWnd, HDC hDC, HGLRC hRC)
{
    wglMakeCurrent (NULL, NULL);
    wglDeleteContext (hRC);
    ReleaseDC (hWnd, hDC);
}



void ren2GL()
{
	
}

void glInit()
{
	
	double FOVy;
    double aspect;
    double znear;
    double zfar;

    // using screen info w and h as params
    glViewport( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT );
	
    // Set current Mode to projection(ie: 3d)
    glMatrixMode( GL_PROJECTION );

    // Load identity matrix to projection matrix
    glLoadIdentity();

    // Set gluPerspective params
    FOVy = 90 / 2.0f;                                     //45 deg fovy
    aspect = SCREEN_WIDTH / (double)SCREEN_HEIGHT;
    znear = 1;                                       //Near clip
    zfar = 500;                                      //far clip

    // use glu Perspective to set our 3d frustum dimension up
    gluPerspective( FOVy, aspect, znear, zfar );

    // Modelview mode
    // ie. Matrix that does things to anything we draw
    // as in lines, points, tris, etc.
    glMatrixMode( GL_MODELVIEW );

    // load identity(clean) matrix to modelview
    glLoadIdentity();

    glShadeModel( GL_SMOOTH );                 // set shading to smooth(try GL_FLAT)
    glClearColor( 0.0, 0.0, 0.0, 1.0 );        // set Clear color to BLACK
    glClearDepth( 1.0 );                       // Set Depth buffer to 1(z-Buffer)
    glDisable( GL_DEPTH_TEST );                // Disable Depth Testing so that our z-buffer works

    // compare each incoming pixel z value with the z value present in the depth buffer
    // LEQUAL means than pixel is drawn if the incoming z value is less than
    // or equal to the stored z value
    glDepthFunc( GL_LEQUAL );

    // have one or more material parameters track the current color
    // Material is your 3d model
    glEnable( GL_COLOR_MATERIAL );


    // Enable Texturing
    glEnable( GL_TEXTURE_2D );


    // Tell openGL that we want the best possible perspective transform
    glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );

    // Disable Backface culling
    glDisable ( GL_CULL_FACE );

    glPolygonMode( GL_FRONT, GL_FILL );

    // enable blending for transparency
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    glDisable( GL_DEPTH_TEST );

    glEnable( GL_ALPHA_TEST );
    glAlphaFunc( GL_GREATER, 0 );


    glDisable( GL_STENCIL_TEST );
    glDisable( GL_TEXTURE_1D );
    glDisable( GL_LIGHTING );
    glDisable( GL_LOGIC_OP );
    glDisable( GL_DITHER );
    glDisable( GL_FOG );

    glHint( GL_POINT_SMOOTH_HINT, GL_NICEST );
    glHint( GL_LINE_SMOOTH_HINT , GL_NICEST );

    glPointSize( 1.0 );
    glLineWidth( 1.0 );
	
	
    glGenTextures( 1, &textureID );
    glBindTexture( GL_TEXTURE_2D, textureID );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, buffer );
                 
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	 
}


void glBegin2D()
{

    glDisable( GL_DEPTH_TEST );
    glDisable( GL_CULL_FACE);

    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    glEnable( GL_ALPHA_TEST );
    glAlphaFunc( GL_GREATER, 0 );

    glPolygonMode( GL_FRONT, GL_FILL );

	glEnable( GL_COLOR_MATERIAL );
	glEnable( GL_TEXTURE_2D );
	
    glDisable( GL_STENCIL_TEST );
    glDisable( GL_TEXTURE_1D );
    glDisable( GL_LIGHTING );
    glDisable( GL_LOGIC_OP );
    glDisable( GL_DITHER );
    glDisable( GL_FOG );

    glPointSize( 1.0 );
    glLineWidth( 1.0 );
	
    glHint( GL_POINT_SMOOTH_HINT, GL_NICEST );
    glHint( GL_LINE_SMOOTH_HINT , GL_NICEST );

	int ViewPort[4];

    glGetIntegerv( GL_VIEWPORT, ViewPort );
	
    glMatrixMode( GL_PROJECTION );
    glPushMatrix();
    glLoadIdentity();
    glOrtho( 0, ViewPort[2], ViewPort[3], 0, -1, 1 );
    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    glLoadIdentity();
    glTranslatef( 0.375, 0.375, 0.0 );

	
}

void glEnd2D()
{

    glMatrixMode( GL_PROJECTION );
    glPopMatrix();
    glMatrixMode( GL_MODELVIEW );
    glPopMatrix();
    
    glColor4ub(255,255,255,255);
	
}
