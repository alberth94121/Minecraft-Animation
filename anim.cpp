////////////////////////////////////////////////////
// anim.cpp version 4.1
// Template code for drawing an articulated figure.
// CS 174A 
////////////////////////////////////////////////////

#ifdef _WIN32
#include <windows.h>
#include "GL/glew.h"
#include <GL/gl.h>
#include <GL/glu.h>
#else
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#ifdef _WIN32
#include "GL/freeglut.h"
#else
#include <GLUT/glut.h>
#endif

#include "Ball.h"
#include "FrameSaver.h"
#include "Timer.h"
#include "Shapes.h"
#include "tga.h"

#include "Angel/Angel.h"

#ifdef __APPLE__
#define glutInitContextVersion(a,b)
#define glutInitContextProfile(a)
#define glewExperimental int glewExperimentalAPPLE
#define glewInit()
#endif

FrameSaver FrSaver ;
Timer TM ;

BallData *Arcball = NULL ;
int Width = 800;
int Height = 800 ;
int Button = -1 ;
float Zoom = 1 ;
int PrevY = 0 ;

int Animate = 0 ;
int Recording = 0 ;

void resetArcball() ;
void save_image();
void instructions();
void set_colour(float r, float g, float b) ;

const int STRLEN = 100;
typedef char STR[STRLEN];

#define PI 3.1415926535897
#define X 0
#define Y 1
#define Z 2

//texture

GLuint texture_cube;
GLuint texture_earth;
GLuint texture_x;
GLuint texture_dia;

// Structs that hold the Vertex Array Object index and number of vertices of each shape.
ShapeData cubeData;
ShapeData sphereData;
ShapeData coneData;
ShapeData cylData;
ShapeData AxeData;

// Matrix stack that can be used to push and pop the modelview matrix.
class MatrixStack {
    int    _index;
    int    _size;
    mat4*  _matrices;

   public:
    MatrixStack( int numMatrices = 32 ):_index(0), _size(numMatrices)
        { _matrices = new mat4[numMatrices]; }

    ~MatrixStack()
	{ delete[]_matrices; }

    void push( const mat4& m ) {
        assert( _index + 1 < _size );
        _matrices[_index++] = m;
    }

    mat4& pop( void ) {
        assert( _index - 1 >= 0 );
        _index--;
        return _matrices[_index];
    }
};

MatrixStack  mvstack;
mat4         model_view;
GLint        uModelView, uProjection, uView;
GLint        uAmbient, uDiffuse, uSpecular, uLightPos, uShininess;
GLint        uTex, uEnableTex;

// The eye point and look-at point.
// Currently unused. Use to control a camera with LookAt().
Angel::vec4 eye(0, 10.0, 00.0,1.0);
Angel::vec4 ref(0.0, 0.0, 0.0,1.0);
Angel::vec4 up(0.0,1.0,0.0,1.0);

double TIME = 0.0 ;

/////////////////////////////////////////////////////
//    PROC: drawCylinder()
//    DOES: this function 
//          render a solid cylinder  oriented along the Z axis. Both bases are of radius 1. 
//          The bases of the cylinder are placed at Z = 0, and at Z = 1.
//
//          
// Don't change.
//////////////////////////////////////////////////////
void drawCylinder(void)
{
    glUniformMatrix4fv( uModelView, 1, GL_TRUE, model_view );
    glBindVertexArray( cylData.vao );
    glDrawArrays( GL_TRIANGLES, 0, cylData.numVertices );
}

//////////////////////////////////////////////////////
//    PROC: drawCone()
//    DOES: this function 
//          render a solid cone oriented along the Z axis with base radius 1. 
//          The base of the cone is placed at Z = 0, and the top at Z = 1. 
//         
// Don't change.
//////////////////////////////////////////////////////
void drawCone(void)
{
    glUniformMatrix4fv( uModelView, 1, GL_TRUE, model_view );
    glBindVertexArray( coneData.vao );
    glDrawArrays( GL_TRIANGLES, 0, coneData.numVertices );
}


//////////////////////////////////////////////////////
//    PROC: drawCube()
//    DOES: this function draws a cube with dimensions 1,1,1
//          centered around the origin.
// 
// Don't change.
//////////////////////////////////////////////////////
void drawCube(void)
{
    glBindTexture( GL_TEXTURE_2D, texture_x );
    glUniform1i( uEnableTex, 1 );
    glUniformMatrix4fv( uModelView, 1, GL_TRUE, model_view );
    glBindVertexArray( cubeData.vao );
    glDrawArrays( GL_TRIANGLES, 0, cubeData.numVertices );
    glUniform1i( uEnableTex, 0 );
}

void drawCreeper(void)//creepercube
{
    glBindTexture( GL_TEXTURE_2D, texture_cube );
    glUniform1i( uEnableTex, 1 );
    glUniformMatrix4fv( uModelView, 1, GL_TRUE, model_view );
    glBindVertexArray( cubeData.vao );
    glDrawArrays( GL_TRIANGLES, 0, cubeData.numVertices );
    glUniform1i( uEnableTex, 0 );
}

void drawDiamond(void) //diamondcube
{
    glBindTexture( GL_TEXTURE_2D, texture_dia);
    glUniform1i( uEnableTex, 1 );
    glUniformMatrix4fv( uModelView, 1, GL_TRUE, model_view );
    glBindVertexArray( cubeData.vao );
    glDrawArrays( GL_TRIANGLES, 0, cubeData.numVertices );
    glUniform1i( uEnableTex, 0 );
}
//////////////////////////////////////////////////////
//    PROC: drawSphere()
//    DOES: this function draws a sphere with radius 1
//          centered around the origin.
// 
// Don't change.
//////////////////////////////////////////////////////

void drawSphere(void)
{
    glBindTexture( GL_TEXTURE_2D, texture_earth);    glUniform1i( uEnableTex, 1 );
    glUniformMatrix4fv( uModelView, 1, GL_TRUE, model_view );
    glBindVertexArray( sphereData.vao );
    glDrawArrays( GL_TRIANGLES, 0, sphereData.numVertices );    glUniform1i( uEnableTex, 0 );
}

//////////////////////////////////////////
//Draw pickaxe head
//////////////////////////////////////////////
void drawAxe(void)
{
    glUniformMatrix4fv( uModelView, 1, GL_TRUE, model_view );
    glBindVertexArray( AxeData.vao );
    glDrawArrays( GL_TRIANGLES, 0, AxeData.numVertices );
}

void resetArcball()
{
    Ball_Init(Arcball);
    Ball_Place(Arcball,qOne,0.75);
}

//////////////////////////////////////////////////////
//    PROC: myKey()
//    DOES: this function gets caled for any keypresses
// 
//////////////////////////////////////////////////////

void myKey(unsigned char key, int x, int y)
{
    float time ;
    switch (key) {
        case 'q':
        case 27:
            exit(0); 
        case 's':
            FrSaver.DumpPPM(Width,Height) ;
            break;
        case 'r':
            resetArcball() ;
            glutPostRedisplay() ;
            break ;
        case 'a': // togle animation
            Animate = 1 - Animate ;
            // reset the timer to point to the current time		
            time = TM.GetElapsedTime() ;
            TM.Reset() ;
            // printf("Elapsed time %f\n", time) ;
            break ;
        case '0':
            //reset your object
            break ;
        case 'm':
            if( Recording == 1 )
            {
                printf("Frame recording disabled.\n") ;
                Recording = 0 ;
            }
            else
            {
                printf("Frame recording enabled.\n") ;
                Recording = 1  ;
            }
            FrSaver.Toggle(Width);
            break ;
        case 'h':
        case '?':
            instructions();
            break;
    }
    glutPostRedisplay() ;

}

/*********************************************************
    PROC: myinit()
    DOES: performs most of the OpenGL intialization
     -- change these with care, if you must.

**********************************************************/
void myinit(void)
{
    // Load shaders and use the resulting shader program
    GLuint program = InitShader( "vshader.glsl", "fshader.glsl" );
    glUseProgram(program);

    // Generate vertex arrays for geometric shapes
    generateCube(program, &cubeData);	
    generateSphere(program, &sphereData);
    generateCone(program, &coneData);
    generateCylinder(program, &cylData);
	generateAxe(program, &AxeData);

    uModelView  = glGetUniformLocation( program, "ModelView"  );
    uProjection = glGetUniformLocation( program, "Projection" );
    uView       = glGetUniformLocation( program, "View"       );

    glClearColor( 0.1, 0.1, 0.2, 1.0 ); // dark blue background

    uAmbient   = glGetUniformLocation( program, "AmbientProduct"  );
    uDiffuse   = glGetUniformLocation( program, "DiffuseProduct"  );
    uSpecular  = glGetUniformLocation( program, "SpecularProduct" );
    uLightPos  = glGetUniformLocation( program, "LightPosition"   );
    uShininess = glGetUniformLocation( program, "Shininess"       );
    uTex       = glGetUniformLocation( program, "Tex"             );
    uEnableTex = glGetUniformLocation( program, "EnableTex"       );

    glUniform4f(uAmbient,    0.2f,  0.2f,  0.2f, 1.0f);
    glUniform4f(uDiffuse,    0.6f,  0.6f,  0.6f, 1.0f);
    glUniform4f(uSpecular,   0.2f,  0.2f,  0.2f, 1.0f);
    glUniform4f(uLightPos,  15.0f, 15.0f, 30.0f, 0.0f);
    glUniform1f(uShininess, 100.0f);

    glEnable(GL_DEPTH_TEST);

    TgaImage coolImage; //creeper
    if (!coolImage.loadTGA("creeper.tga"))
    {
        printf("Error loading image file\n");
        exit(1);
    }
    
    TgaImage earthImage; //fire
    if (!earthImage.loadTGA("earth.tga"))
    {
        printf("Error loading image file\n");
        exit(1);
    }
	
	TgaImage diaImage; //diamond cube
    if (!diaImage.loadTGA("diamond.tga"))
    {
        printf("Error loading image file\n");
        exit(1);
    }
    
    glGenTextures( 1, &texture_cube );
    glBindTexture( GL_TEXTURE_2D, texture_cube );
    
    glTexImage2D(GL_TEXTURE_2D, 0, 4, coolImage.width, coolImage.height, 0,
                 (coolImage.byteCount == 3) ? GL_BGR : GL_BGRA,
                 GL_UNSIGNED_BYTE, coolImage.data );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    
    
   glGenTextures( 1, &texture_earth );
    glBindTexture( GL_TEXTURE_2D, texture_earth );
    
    glTexImage2D(GL_TEXTURE_2D, 0, 4, earthImage.width, earthImage.height, 0,
                 (earthImage.byteCount == 3) ? GL_BGR : GL_BGRA,
                 GL_UNSIGNED_BYTE, earthImage.data );
    
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	
	glGenTextures( 1, &texture_dia );
    glBindTexture( GL_TEXTURE_2D, texture_dia );
    
    glTexImage2D(GL_TEXTURE_2D, 0, 4, diaImage.width, diaImage.height, 0,
                 (diaImage.byteCount == 3) ? GL_BGR : GL_BGRA,
                 GL_UNSIGNED_BYTE, diaImage.data );
    
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    
    // Set texture sampler variable to texture unit 0
    // (set in glActiveTexture(GL_TEXTURE0))
    
    glUniform1i( uTex, 0);
    
    Arcball = new BallData;
    Ball_Init(Arcball);
    Ball_Place(Arcball,qOne,0.75);
}


/*********************************************************
    PROC: set_colour();
    DOES: sets all material properties to the given colour
    -- don't change
**********************************************************/

void set_colour(float r, float g, float b)
{
    float ambient  = 0.2f;
    float diffuse  = 0.6f;
    float specular = 0.2f;
    glUniform4f(uAmbient,  ambient*r,  ambient*g,  ambient*b,  1.0f);
    glUniform4f(uDiffuse,  diffuse*r,  diffuse*g,  diffuse*b,  1.0f);
    glUniform4f(uSpecular, specular*r, specular*g, specular*b, 1.0f);
}

/*********************************************************
**********************************************************
**********************************************************

    PROC: display()
    DOES: this gets called by the event handler to draw
          the scene, so this is where you need to build
          your ROBOT --  
      
        MAKE YOUR CHANGES AND ADDITIONS HERE

    Add other procedures if you like.

**********************************************************
**********************************************************
**********************************************************/
void drawGround(mat4 view_trans)
{
    mat4 model_trans(1.0f);
    set_colour(1.0f, 1.0f, 1.0f); 
    model_trans *= Translate(0, -1, 0);
    model_trans *= Scale(100, 1, 100);
    model_view = view_trans * model_trans;
    drawCube();
}


void drawPerson(mat4 view_trans)
{
    mat4 model_trans(1.0f);
    //if(TIME<10)
	{
      set_colour(2.0f, 1.0f, 1.0f); 
	  mvstack.push(model_trans);//push
	  if(TIME<=40)
          model_trans *= Translate(0, 2.5, 0);
	  else if(TIME<=45)
		 { model_trans *= Translate(0, 0, 0);
	  model_trans *= Translate(0, 2.5, TIME-40);}
	  else  model_trans *= Translate(0, 2.5, 7.0);

	      mvstack.push(model_trans);//push
             model_trans *= Scale(1.5, 2.3, 1);
             model_view = view_trans * model_trans;
             drawCube();
	     model_trans = mvstack.pop();//pop 

	///legs
	////////////////////////
	set_colour(2.0f, 2.0f, 1.0f);
	mvstack.push(model_trans);//push
	model_trans *= Translate(0.4, -2, 0);
	 model_trans *= Scale(0.75, 1.8, 1);
    model_view = view_trans * model_trans;
    drawCube();
	model_trans = mvstack.pop();//pop 

	mvstack.push(model_trans);//push
	 model_trans *= Translate(-0.4, -2, 0);
	 model_trans *= Scale(0.75, 1.8, 1);
    model_view = view_trans * model_trans;
    drawCube();
	model_trans = mvstack.pop();//pop 
	////////////////////////

	//////////////////////arms
	/////////////////////////////
	mvstack.push(model_trans);//push
	model_trans *= Translate(-1.1, 0.9, 0);
	 model_trans *= RotateX(90);
	 model_trans *= RotateX(80.0*sin(2*TIME));
	 model_trans *= Translate(0, 0.5, 0.1);
	 model_trans *= Scale(0.75, 1.8, 0.75);
    model_view = view_trans * model_trans;
    drawCube();

	 model_trans *= Scale(1/0.75,1/1.8, 1/0.75);
	 model_trans *= RotateX(-90);
	 if(TIME<15){
	 model_trans *= Translate(0, 1.0, 0.6);
	 model_trans *= Scale(0.2, 2.2, 0.2);
	 model_trans *= RotateX(90);
	    model_view = view_trans * model_trans;
		set_colour( 0.647059 , 0.264706 ,0.164706);
    drawCube();

	 model_trans *= Scale(1/0.3,1/1.8, 1/0.3);
	 model_trans *= RotateX(-90);
	  model_trans *= Translate(0, 0.12, 0.0);
	 model_trans *= Scale(.65, .10, 12.3);
	    model_view = view_trans * model_trans;
		set_colour(0.8f, 0.8f, 0.8f);
		drawAxe();}
	 else
	 { set_colour(1.4f, 1.4f, 1.4f);	
		 model_trans *= Translate(0, 0.5, 0.8);
	 model_trans *= Scale(0.7, 0.7, 0.7);
	 
	    model_view = view_trans * model_trans;

    drawDiamond();
	 }

	model_trans = mvstack.pop();//pop 


	set_colour(2.0f, 2.0f, 1.0f);
	mvstack.push(model_trans);//push
	model_trans *= Translate(1.1, 0.9, 0);
	 model_trans *= RotateX(180);
	 model_trans *= Translate(0, 0.5, 0.1);
	 model_trans *= Scale(0.75, 1.8, 0.75);
    model_view = view_trans * model_trans;
    drawCube();
	model_trans = mvstack.pop();//pop 
	//////////////////////////////////


	/////head
	mvstack.push(model_trans);//push
	set_colour(2.0f, 2.0f, 1.0f);
	model_trans *= Translate(0, 1.6, 0);
	mvstack.push(model_trans);//push
	 model_trans *= Scale(1.55, 1.45, 1.45);
    model_view = view_trans * model_trans;
    drawCube();
	 model_trans = mvstack.pop();//pop

	 //eyes
	 /////////////////////////
	 mvstack.push(model_trans);//push
	 	 set_colour(1.0f, 1.0f, 1.0f);
	 model_trans *= Translate(0.4, 0.2, 0.8);

	 model_trans *= Scale(0.25, 0.25,0.25);
    model_view = view_trans * model_trans;
    drawCube();

	 model_trans = mvstack.pop();//pop
	 mvstack.push(model_trans);//push
	 	 set_colour(1.0f, 1.0f, 1.0f);
	 model_trans *= Translate(-0.4, 0.2, 0.8);

	 model_trans *= Scale(0.25, 0.25,0.25);
    model_view = view_trans * model_trans;
    drawCube();

	 model_trans = mvstack.pop();//pop

	 mvstack.push(model_trans);//push
	 	 set_colour(1.0f, 1.0f, 1.0f);
	 model_trans *= Translate(0.0, -0.2, 0.8);

	 model_trans *= Scale(0.4, 0.1,0.1);
    model_view = view_trans * model_trans;
    drawCube();

	 model_trans = mvstack.pop();//pop
	 /////////////////
     model_trans = mvstack.pop();//pop
     /////////////////////////////////////////
	 model_trans = mvstack.pop();//pop  
	 }
    
}

void drawCreeper(mat4 view_trans)
{
    mat4 model_trans(1.0f);
	model_trans *= RotateY(120);
		model_trans *= RotateY(-20*TIME);
	model_trans *= Translate(15.0f, (0.5+(0.7*sin(2.0*TIME))), 0.0f);
	if(TIME<10)
	{
      set_colour(0.0f, 1.0f, 0.0f); //green
	 mvstack.push(model_trans);//push
     model_trans *= Translate(0, 2.3, 0);
	 mvstack.push(model_trans);//push
     model_trans *= Scale(1.5, 5, 1);
     model_view = view_trans * model_trans;
     drawCreeper();
	 model_trans = mvstack.pop();//pop 

	///legs
	////////////////////////

	mvstack.push(model_trans);//push
	model_trans *= Translate(0, -2, 0.75);
	 model_trans *= Scale(2.0, 1.4, 0.8);
    model_view = view_trans * model_trans;
    drawCreeper();
	model_trans = mvstack.pop();//pop 

	mvstack.push(model_trans);//push
	model_trans *= Translate(0, -2, -0.75);
	 model_trans *= Scale(2.0, 1.4, 0.8);
    model_view = view_trans * model_trans;
    drawCreeper();
	model_trans = mvstack.pop();//pop 

	////////////////////////

	/////head
	mvstack.push(model_trans);//push

	model_trans *= Translate(0, 1.8, 0);
	mvstack.push(model_trans);//push
	 model_trans *= Scale(1.55, 1.60, 1.60);
    model_view = view_trans * model_trans;
    drawCreeper();
	 model_trans = mvstack.pop();//pop

	 //eyes
	 /////////////////////////
	 mvstack.push(model_trans);//push
	 	 set_colour(0.0f, 0.0f, 0.0f);
	 model_trans *= Translate(0.4, 0.2, 0.8);

	 model_trans *= Scale(0.25, 0.25,0.25);
    model_view = view_trans * model_trans;
    drawCube();

	 model_trans = mvstack.pop();//pop
	 mvstack.push(model_trans);//push
	
	 model_trans *= Translate(-0.4, 0.2, 0.8);

	 model_trans *= Scale(0.25, 0.25,0.25);
    model_view = view_trans * model_trans;
    drawCube();

	 model_trans = mvstack.pop();//pop

	 mvstack.push(model_trans);//push
	 	
	 model_trans *= Translate(0.0, -0.2, 0.8);

	 model_trans *= Scale(0.4, 0.1,0.1);
    model_view = view_trans * model_trans;
    drawCube();

	 model_trans = mvstack.pop();//pop
	 /////////////////
     model_trans = mvstack.pop();//pop
     /////////////////////////////////////////

	 model_trans = mvstack.pop();//pop  
	}
	 
	else if(TIME<11)
	{
		set_colour(1.0f, 0.5f, 0.0f); //green
		model_trans *= Scale(20.0, 20.0, 20.0);
	    model_view = view_trans * model_trans;
	    drawSphere();
	}
	else 
	{
	}
 
}

void drawCreeper2(mat4 view_trans)
{
    mat4 model_trans(1.0f);
	model_trans *= RotateY(-20*TIME);
	model_trans *= RotateY(240);
	model_trans *= Translate(15.0f, (0.5+(0.7*sin(2.0*TIME))), 0.0f);
	if (TIME <10)
	{  
      set_colour(0.0f, 1.0f, 0.0f); //green
	  mvstack.push(model_trans);//push
      model_trans *= Translate(0, 2.3, 0);
	  mvstack.push(model_trans);//push
      model_trans *= Scale(1.5, 5, 1);
      model_view = view_trans * model_trans;
      drawCreeper();
	  model_trans = mvstack.pop();//pop 

	///legs
	////////////////////////

	mvstack.push(model_trans);//push
	model_trans *= Translate(0, -2, 0.75);
	 model_trans *= Scale(2.0, 1.4, 0.8);
    model_view = view_trans * model_trans;
    drawCreeper();
	model_trans = mvstack.pop();//pop 

	mvstack.push(model_trans);//push
	model_trans *= Translate(0, -2, -0.75);
	 model_trans *= Scale(2.0, 1.4, 0.8);
    model_view = view_trans * model_trans;
    drawCreeper();
	model_trans = mvstack.pop();//pop 

	/////head
	mvstack.push(model_trans);//push

	model_trans *= Translate(0, 1.8, 0);
	mvstack.push(model_trans);//push
	 model_trans *= Scale(1.55, 1.60, 1.60);
    model_view = view_trans * model_trans;
    drawCreeper();
	 model_trans = mvstack.pop();//pop

	 //eyes
	 /////////////////////////
	 mvstack.push(model_trans);//push
	 	 set_colour(0.0f, 0.0f, 0.0f);
	 model_trans *= Translate(0.4, 0.2, 0.8);

	 model_trans *= Scale(0.25, 0.25,0.25);
    model_view = view_trans * model_trans;
    drawCube();

	 model_trans = mvstack.pop();//pop
	 mvstack.push(model_trans);//push
	
	 model_trans *= Translate(-0.4, 0.2, 0.8);

	 model_trans *= Scale(0.25, 0.25,0.25);
    model_view = view_trans * model_trans;
    drawCube();

	 model_trans = mvstack.pop();//pop

	 mvstack.push(model_trans);//push
	 	
	 model_trans *= Translate(0.0, -0.2, 0.8);

	 model_trans *= Scale(0.4, 0.1,0.1);
    model_view = view_trans * model_trans;
    drawCube();

	 model_trans = mvstack.pop();//pop
	 /////////////////
     model_trans = mvstack.pop();//pop
     /////////////////////////////////////////
	 model_trans = mvstack.pop();//pop  
	}
	else if(TIME<11)
	{
			 set_colour(1.0f, 0.5f, 0.0f); //green
			model_trans *= Scale(20.0, 20.0, 20.0);
	        model_view = view_trans * model_trans;
	       drawSphere();
	}
	else 
	{
	}
    
}

void drawCreeper3(mat4 view_trans)
{
    mat4 model_trans(1.0f);
	model_trans *= RotateY(-20*TIME);
	model_trans *= RotateY(360);
	model_trans *= Translate(15.0f, (0.5+(0.7*sin(2.0*TIME))), 0.0f);
	if(TIME <10)
	{  
      set_colour(0.0f, 1.0f, 0.0f); //green
	  mvstack.push(model_trans);//push
      model_trans *= Translate(0, 2.3, 0);
	  mvstack.push(model_trans);//push
      model_trans *= Scale(1.5, 5, 1);
      model_view = view_trans * model_trans;
      drawCreeper();
	  model_trans = mvstack.pop();//pop 

	///legs
	////////////////////////

	mvstack.push(model_trans);//push
	model_trans *= Translate(0, -2, 0.75);
	 model_trans *= Scale(2.0, 1.4, 0.8);
    model_view = view_trans * model_trans;
    drawCreeper();
	model_trans = mvstack.pop();//pop 

	mvstack.push(model_trans);//push
	model_trans *= Translate(0, -2, -0.75);
	 model_trans *= Scale(2.0, 1.4, 0.8);
    model_view = view_trans * model_trans;
    drawCreeper();
	model_trans = mvstack.pop();//pop 

	/////head
	mvstack.push(model_trans);//push

	model_trans *= Translate(0, 1.8, 0);
	mvstack.push(model_trans);//push
	 model_trans *= Scale(1.55, 1.60, 1.60);
    model_view = view_trans * model_trans;
    drawCreeper();
	 model_trans = mvstack.pop();//pop

	 //eyes
	 /////////////////////////
	 mvstack.push(model_trans);//push
	 	 set_colour(0.0f, 0.0f, 0.0f);
	 model_trans *= Translate(0.4, 0.2, 0.8);

	 model_trans *= Scale(0.25, 0.25,0.25);
    model_view = view_trans * model_trans;
    drawCube();

	 model_trans = mvstack.pop();//pop
	 mvstack.push(model_trans);//push
	
	 model_trans *= Translate(-0.4, 0.2, 0.8);

	 model_trans *= Scale(0.25, 0.25,0.25);
    model_view = view_trans * model_trans;
    drawCube();

	 model_trans = mvstack.pop();//pop

	 mvstack.push(model_trans);//push
	 	
	 model_trans *= Translate(0.0, -0.2, 0.8);

	 model_trans *= Scale(0.4, 0.1,0.1);
    model_view = view_trans * model_trans;
    drawCube();

	 model_trans = mvstack.pop();//pop
	 /////////////////
     model_trans = mvstack.pop();//pop
     /////////////////////////////////////////
	 model_trans = mvstack.pop();//pop  
	}
	else if(TIME<11)
	{
			 set_colour(1.0f, 0.5f, 0.0f); //green
			model_trans *= Scale(20.0, 20.0, 20.0);
	        model_view = view_trans * model_trans;
	       drawSphere();
	}
	else 
	{
	}
    
}

void drawDiamonds(mat4 view_trans)
{
	 mat4 model_trans(1.0f);
     set_colour(1.4f, 1.4f, 1.4f);	
	 if (TIME<10)
	 {	mvstack.push(model_trans);//push

		 	 model_trans *= Translate(0.0f, 1.0f, 4.0f);
	   model_trans *= Scale(3.0f, 3.0f,3.0f);
       model_view = view_trans * model_trans;
	   drawDiamond();
	model_trans = mvstack.pop();//pop 
		mvstack.push(model_trans);//push
		 
	 model_trans *= Translate(3.0f, 1.0f, 7.0f);
	 model_trans *= Scale(3.0f, 3.0f,3.0f);
    model_view = view_trans * model_trans;
	  drawDiamond();
	    model_trans = mvstack.pop();//pop 
		mvstack.push(model_trans);//push
		
	 model_trans *= Translate(0.0f, 1.0f, 7.0f);
	 model_trans *= Scale(3.0f, 3.0f,3.0f);
    model_view = view_trans * model_trans;
	  drawDiamond();
	    model_trans = mvstack.pop();//pop 
		mvstack.push(model_trans);//push
		
	 model_trans *= Translate(-3.0f, 1.0f, 7.0f);
	 model_trans *= Scale(3.0f, 3.0f,3.0f);
    model_view = view_trans * model_trans;
	  drawDiamond();
	    model_trans = mvstack.pop();//pop 
		mvstack.push(model_trans);//push
		 
	 model_trans *= Translate(0.0f, 1.0f, 10.0f);
	 model_trans *= Scale(3.0f, 3.0f,3.0f);
    model_view = view_trans * model_trans;
	  drawDiamond();
	    model_trans = mvstack.pop();//pop 
		mvstack.push(model_trans);//push	
	 model_trans *= Translate(0.0f, 4.0f, 7.0f);
	 model_trans *= Scale(3.0f, 3.0f,3.0f);
    model_view = view_trans * model_trans;
	  drawDiamond();
	    model_trans = mvstack.pop();//pop 
	 }
	 else if(TIME<15)
	 {
		 	mvstack.push(model_trans);//push
			model_trans *= Translate(0.0f, 1.0f, 4.0f);
        model_trans *= Scale(0.5, 0.5, 0.5);
	    model_trans *= RotateY(40*TIME);
       model_view = view_trans * model_trans;
	   drawDiamond();
	   model_trans = mvstack.pop();

	  mvstack.push(model_trans);//push
	  model_trans *= Translate(0.0f, 1.0f, 4.0f);
	   model_trans *= Translate(1.0f,0.0f, 0.0f);
    model_trans *= Scale(0.5, 0.5, 0.5);
	model_trans *= RotateY(40*TIME);
    model_view = view_trans * model_trans;
	drawDiamond();model_trans = mvstack.pop();
	  model_trans *= Translate(0.0f,0.0f, 1.0f);
	  mvstack.push(model_trans);//push
	  model_trans *= Translate(0.0f, 1.0f, 4.0f);
    model_trans *= Scale(0.5, 0.5, 0.5);
	model_trans *= RotateY(40*TIME);
    model_view = view_trans * model_trans;
	drawDiamond();model_trans = mvstack.pop();

	 }
	 else
	 {
		 if(TIME>15)
		 {
			
		mvstack.push(model_trans);//push	 
	 model_trans *= Translate(3.0f, 1.0f, 4.0f);
	 model_trans *= Scale(3.0f, 3.0f,3.0f);
    model_view = view_trans * model_trans;
	  drawDiamond();
	    model_trans = mvstack.pop();//pop 
		 }
		 if(TIME>16)
		 {
			
		mvstack.push(model_trans);//push	 
	 model_trans *= Translate(3.0f, 1.0f, 7.0f);
	 model_trans *= Scale(3.0f, 3.0f,3.0f);
    model_view = view_trans * model_trans;
	  drawDiamond();
	    model_trans = mvstack.pop();//pop 
		 }
		 if(TIME>17)
		 {
			
		mvstack.push(model_trans);//push	 
	 model_trans *= Translate(3.0f, 1.0f, 10.0f);
	 model_trans *= Scale(3.0f, 3.0f,3.0f);
    model_view = view_trans * model_trans;
	  drawDiamond();
	    model_trans = mvstack.pop();//pop 
		 }
		  if(TIME>18)
		 {
			
		mvstack.push(model_trans);//push	 
	 model_trans *= Translate(0.0f, 1.0f, 10.0f);
	 model_trans *= Scale(3.0f, 3.0f,3.0f);
    model_view = view_trans * model_trans;
	  drawDiamond();
	    model_trans = mvstack.pop();//pop 
		 }
		   if(TIME>19)
		 {
			
		mvstack.push(model_trans);//push	 
	 model_trans *= Translate(-3.0f, 1.0f, 10.0f);
	 model_trans *= Scale(3.0f, 3.0f,3.0f);
    model_view = view_trans * model_trans;
	  drawDiamond();
	    model_trans = mvstack.pop();//pop 
		 }
		    if(TIME>20)
		 {
			
		mvstack.push(model_trans);//push	 
	 model_trans *= Translate(-3.0f, 1.0f, 7.0f);
	 model_trans *= Scale(3.0f, 3.0f,3.0f);
    model_view = view_trans * model_trans;
	  drawDiamond();
	    model_trans = mvstack.pop();//pop 
		 }
			   if(TIME>21)
		 {
			
		mvstack.push(model_trans);//push	 
	 model_trans *= Translate(-3.0f, 1.0f,4.0f);
	 model_trans *= Scale(3.0f, 3.0f,3.0f);
    model_view = view_trans * model_trans;
	  drawDiamond();
	    model_trans = mvstack.pop();//pop 
		 }
			///////////////////////LAYEr2
			   if(TIME>22)
		 {
			
		mvstack.push(model_trans);//push	 
	 model_trans *= Translate(3.0f, 4.0f, 4.0f);
	 model_trans *= Scale(3.0f, 3.0f,3.0f);
    model_view = view_trans * model_trans;
	  drawDiamond();
	    model_trans = mvstack.pop();//pop 
		 }
		 if(TIME>23)
		 {
			
		mvstack.push(model_trans);//push	 
	 model_trans *= Translate(3.0f, 4.0f, 7.0f);
	 model_trans *= Scale(3.0f, 3.0f,3.0f);
    model_view = view_trans * model_trans;
	  drawDiamond();
	    model_trans = mvstack.pop();//pop 
		 }
		 if(TIME>24)
		 {
			
		mvstack.push(model_trans);//push	 
	 model_trans *= Translate(3.0f, 4.0f, 10.0f);
	 model_trans *= Scale(3.0f, 3.0f,3.0f);
    model_view = view_trans * model_trans;
	  drawDiamond();
	    model_trans = mvstack.pop();//pop 
		 }
		  if(TIME>25)
		 {
			
		mvstack.push(model_trans);//push	 
	 model_trans *= Translate(0.0f, 4.0f, 10.0f);
	 model_trans *= Scale(3.0f, 3.0f,3.0f);
    model_view = view_trans * model_trans;
	  drawDiamond();
	    model_trans = mvstack.pop();//pop 
		 }
		   if(TIME>26)
		 {
			
		mvstack.push(model_trans);//push	 
	 model_trans *= Translate(-3.0f, 4.0f, 10.0f);
	 model_trans *= Scale(3.0f, 3.0f,3.0f);
    model_view = view_trans * model_trans;
	  drawDiamond();
	    model_trans = mvstack.pop();//pop 
		 }
		    if(TIME>27)
		 {
			
		mvstack.push(model_trans);//push	 
	 model_trans *= Translate(-3.0f, 4.0f, 7.0f);
	 model_trans *= Scale(3.0f, 3.0f,3.0f);
    model_view = view_trans * model_trans;
	  drawDiamond();
	    model_trans = mvstack.pop();//pop 
		 }
			   if(TIME>28)
		 {
			
		mvstack.push(model_trans);//push	 
	 model_trans *= Translate(-3.0f, 4.0f,4.0f);
	 model_trans *= Scale(3.0f, 3.0f,3.0f);
    model_view = view_trans * model_trans;
	  drawDiamond();
	    model_trans = mvstack.pop();//pop 
		 }
			   ///////////////////////LAYEr3
			   if(TIME>29)
		 {
			
		mvstack.push(model_trans);//push	 
	 model_trans *= Translate(3.0f, 7.0f, 4.0f);
	 model_trans *= Scale(3.0f, 3.0f,3.0f);
    model_view = view_trans * model_trans;
	  drawDiamond();
	    model_trans = mvstack.pop();//pop 
		 }
		 if(TIME>30)
		 {
			
		mvstack.push(model_trans);//push	 
	 model_trans *= Translate(3.0f, 7.0f, 7.0f);
	 model_trans *= Scale(3.0f, 3.0f,3.0f);
    model_view = view_trans * model_trans;
	  drawDiamond();
	    model_trans = mvstack.pop();//pop 
		 }
		 if(TIME>31)
		 {
			
		mvstack.push(model_trans);//push	 
	 model_trans *= Translate(3.0f, 7.0f, 10.0f);
	 model_trans *= Scale(3.0f, 3.0f,3.0f);
    model_view = view_trans * model_trans;
	  drawDiamond();
	    model_trans = mvstack.pop();//pop 
		 }
		  if(TIME>32)
		 {
			
		mvstack.push(model_trans);//push	 
	 model_trans *= Translate(0.0f, 7.0f, 10.0f);
	 model_trans *= Scale(3.0f, 3.0f,3.0f);
    model_view = view_trans * model_trans;
	  drawDiamond();
	    model_trans = mvstack.pop();//pop 
		 }
		   if(TIME>33)
		 {
			
		mvstack.push(model_trans);//push	 
	 model_trans *= Translate(-3.0f, 7.0f, 10.0f);
	 model_trans *= Scale(3.0f, 3.0f,3.0f);
    model_view = view_trans * model_trans;
	  drawDiamond();
	    model_trans = mvstack.pop();//pop 
		 }
		    if(TIME>34)
		 {
			
		mvstack.push(model_trans);//push	 
	 model_trans *= Translate(-3.0f, 7.0f, 7.0f);
	 model_trans *= Scale(3.0f, 3.0f,3.0f);
    model_view = view_trans * model_trans;
	  drawDiamond();
	    model_trans = mvstack.pop();//pop 
		 }
			   if(TIME>35)
		 {
			
		mvstack.push(model_trans);//push	 
	 model_trans *= Translate(-3.0f, 7.0f,4.0f);
	 model_trans *= Scale(3.0f, 3.0f,3.0f);
    model_view = view_trans * model_trans;
	  drawDiamond();
	    model_trans = mvstack.pop();//pop 
		 }
			      if(TIME>36)
		 {
			
		mvstack.push(model_trans);//push	 
	 model_trans *= Translate(0.0f, 7.0f,4.0f);
	 model_trans *= Scale(3.0f, 3.0f,3.0f);
    model_view = view_trans * model_trans;
	  drawDiamond();
	    model_trans = mvstack.pop();//pop 
		 }
				    if(TIME>37)
		 {
			
		mvstack.push(model_trans);//push	 
	 model_trans *= Translate(0.0f, 7.0f,7.0f);
	 model_trans *= Scale(3.0f, 3.0f,3.0f);
    model_view = view_trans * model_trans;
	  drawDiamond();
	    model_trans = mvstack.pop();//pop 
		 }
	 }	 

}

void drawSlender(mat4 view_trans)
{
	
	      set_colour(0.0f, 0.0f, 0.0f);
    mat4 model_trans(1.0f);

		model_trans *= RotateY(-20*TIME);
	
	model_trans *= Translate(25.0f, 0.0f, 0.0f);
    model_trans *= Translate(0, 14.5, 0);
	
 mvstack.push(model_trans);//push

    model_trans *= Scale(1.5, 10, 1);
    model_view = view_trans * model_trans;
    drawCube();
	model_trans = mvstack.pop();//pop

	mvstack.push(model_trans);//push
	model_trans *= Translate(0, 6, 0);
	mvstack.push(model_trans);//push
	model_trans *= Scale(2, 3, 2);
	model_view = view_trans * model_trans;
    drawCube();
	model_trans = mvstack.pop();//pop

	set_colour(6.0f, 6.0f, 6.0f);
	
	mvstack.push(model_trans);//push
	model_trans *= Translate(0.5, 0.7, 1);
	model_trans *= Scale(0.5, 0.5, 0.5);
	model_view = view_trans * model_trans;
    drawCube();
	model_trans = mvstack.pop();//pop

	mvstack.push(model_trans);//push
	model_trans *= Translate(-0.5, 0.7, 1);
	model_trans *= Scale(0.5, 0.5, 0.5);
	model_view = view_trans * model_trans;
    drawCube();
	model_trans = mvstack.pop();//pop
	model_trans = mvstack.pop();//pop

	 set_colour(0.0f, 0.0f, 0.0f);
	//legs
	
	  mvstack.push(model_trans);//push
	   model_trans *= Translate(1.0, -3.5, 0);
	    mvstack.push(model_trans);//push
		 model_trans *= RotateX(-10);
	  model_trans *= RotateX(-30*sin(TIME));
	  model_trans *= Translate(0.0, -4, 0);
	 model_trans *= Scale(1.0, 7, 1);
    model_view = view_trans * model_trans;
    drawCube();

	model_trans *= Translate(0.0, -0.50, 0);
	 model_trans *= RotateX(5);
	  model_trans *= RotateX(-20*sin(TIME));
	  model_trans *= Translate(0.0, -0, 0);
	 model_trans *= Scale(1.0, 1, 1);
    model_view = view_trans * model_trans;
    drawCube();
	model_trans = mvstack.pop();//pop
	 model_trans = mvstack.pop();//pop

	  mvstack.push(model_trans);//push
	   model_trans *= Translate(-1.0, -3.5, 0);
	    mvstack.push(model_trans);//push
		 model_trans *= RotateX(10);
	  model_trans *= RotateX(30*sin(TIME));
	  model_trans *= Translate(0.0, -4, 0);
	 model_trans *= Scale(1.0, 7, 1);
    model_view = view_trans * model_trans;
    drawCube();

	model_trans *= Translate(0.0, -0.50, 0);
	 model_trans *= RotateX(-5);
	  model_trans *= RotateX(20*sin(TIME));
	  model_trans *= Translate(0.0, -0, 0);
	 model_trans *= Scale(1.0, 1, 1);
    model_view = view_trans * model_trans;
    drawCube();
	model_trans = mvstack.pop();//pop
	 model_trans = mvstack.pop();//pop

	 //arms
	mvstack.push(model_trans);//push
	   model_trans *= Translate(-1.0, 3.5, 0);
	    
		model_trans *= RotateZ(-15);
		
	  model_trans *= RotateX(30*sin(TIME));
	  model_trans *= Translate(0.0, -2, 0);
	 model_trans *= Scale(0.8, 6, 0.8);
    model_view = view_trans * model_trans;
    drawCube();
	
	model_trans *= Scale(1/0.8,1/6.0, 1/0.80);
	model_trans *= Translate(-00.0,-2.00, 0);
	model_trans *= RotateX(30*sin(TIME));
	model_trans *= Translate(-00.0,-3.57, 1.95);
	
	 model_trans *= RotateX(-35);
	   

	 model_trans *= Scale(0.8,6.8,0.8);
    model_view = view_trans * model_trans;
    drawCube();
	
	 model_trans = mvstack.pop();//pop

	 mvstack.push(model_trans);//push
	   model_trans *= Translate(1.0, 3.5, 0);
	    
		model_trans *= RotateZ(15);
		
	  model_trans *= RotateX(-30*sin(TIME));
	  model_trans *= Translate(0.0, -2, 0);
	 model_trans *= Scale(0.8, 6, 0.8);
    model_view = view_trans * model_trans;
    drawCube();
	
	model_trans *= Scale(1/0.8,1/6.0, 1/0.80);
	model_trans *= Translate(-00.0,-2.00, 0);
	model_trans *= RotateX(-30*sin(TIME));
	model_trans *= Translate(-00.0,-3.57, 1.95);
	
	 model_trans *= RotateX(-35);
	   

	 model_trans *= Scale(0.8,6.8,0.8);
    model_view = view_trans * model_trans;
    drawCube();
	
	 model_trans = mvstack.pop();//pop
}


void cameraHelper()
{

	if (TIME <= 7.0)
	{
		eye.x = 0;
		eye.y = 10;
		eye.z = 10;
		ref.x = 0;
		ref.y = 0;
		ref.z = 0;
	}
	else if(TIME <=15)
		{
	
		eye.x = 90;
		eye.y = 90;
		eye.z = 0;
		ref.x = 1;
		ref.y = 1;
		ref.z = 1;
	}
	else if (TIME <=40)
	{
		eye.x = 30;
		eye.y = 30;
		eye.z = 0;
		ref.x = 1;
		ref.y = 1;
		ref.z = 1;
	}
	else 
		{
	
		eye.x = 90;
		eye.y = 90;
		eye.z = 0;
		ref.x = 1;
		ref.y = 1;
		ref.z = 1;
	}
}
 	

void display(void)
{
    // /Clear the screen with the background colour (set in myinit)
     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

     mat4 model_trans(1.0f);
     mat4 view_trans(1.0f);
     
     view_trans *= Translate(0.0f, 0.0f, -15.0f);

     HMatrix r;
     Ball_Value(Arcball,r);
     
     mat4 mat_arcball_rot(
     r[0][0], r[0][1], r[0][2], r[0][3],
     r[1][0], r[1][1], r[1][2], r[1][3],
     r[2][0], r[2][1], r[2][2], r[2][3],
     r[3][0], r[3][1], r[3][2], r[3][3]);
     view_trans *= mat_arcball_rot;
     view_trans *= Scale(Zoom);
     
     glUniformMatrix4fv( uView, 1, GL_TRUE, model_view );
	
	 	
	////////////////////////////    
	
	 //view_trans *= Scale(Zoom);
	view_trans = LookAt(eye, ref, up); 
	mvstack.push(model_trans);//push
      drawGround(view_trans);
     model_trans = mvstack.pop();//pop  

	 mvstack.push(model_trans);//push
	  drawPerson(view_trans); 
   model_trans = mvstack.pop();//pop 

   //mvstack.push(model_trans);//push
		drawDiamonds(view_trans);
	
     mvstack.push(model_trans);//push
	  drawCreeper(view_trans);
	    model_trans = mvstack.pop();//pop 

		 mvstack.push(model_trans);//push
	  drawCreeper3(view_trans);
	    model_trans = mvstack.pop();//pop 
	
		 mvstack.push(model_trans);//push
	  drawCreeper2(view_trans);

	   mvstack.push(model_trans);//push
	  drawSlender(view_trans);
	    model_trans = mvstack.pop();//pop 

		  model_trans = mvstack.pop();//pop 
		  
			cameraHelper();
		  view_trans = LookAt(eye,ref,up);

    glutSwapBuffers();
    if(Recording == 1)
        FrSaver.DumpPPM(Width, Height) ;
}

/**********************************************
    PROC: myReshape()
    DOES: handles the window being resized 
    
      -- don't change
**********************************************************/

void myReshape(int w, int h)
{
    Width = w;
    Height = h;

    glViewport(0, 0, w, h);

    mat4 projection = Perspective(50.0f, (float)w/(float)h, 1.0f, 1000.0f);
    glUniformMatrix4fv( uProjection, 1, GL_TRUE, projection );
}

void instructions() 
{
    printf("Press:\n");
    printf("  s to save the image\n");
    printf("  r to restore the original view.\n") ;
    printf("  0 to set it to the zero state.\n") ;
    printf("  a to toggle the animation.\n") ;
    printf("  m to toggle frame dumping.\n") ;
    printf("  q to quit.\n");
}

// start or end interaction
void myMouseCB(int button, int state, int x, int y)
{
    Button = button ;
    if( Button == GLUT_LEFT_BUTTON && state == GLUT_DOWN )
    {
        HVect arcball_coords;
        arcball_coords.x = 2.0*(float)x/(float)Width-1.0;
        arcball_coords.y = -2.0*(float)y/(float)Height+1.0;
        Ball_Mouse(Arcball, arcball_coords) ;
        Ball_Update(Arcball);
        Ball_BeginDrag(Arcball);

    }
    if( Button == GLUT_LEFT_BUTTON && state == GLUT_UP )
    {
        Ball_EndDrag(Arcball);
        Button = -1 ;
    }
    if( Button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN )
    {
        PrevY = y ;
    }


    // Tell the system to redraw the window
    glutPostRedisplay() ;
}

// interaction (mouse motion)
void myMotionCB(int x, int y)
{
    if( Button == GLUT_LEFT_BUTTON )
    {
        HVect arcball_coords;
        arcball_coords.x = 2.0*(float)x/(float)Width - 1.0 ;
        arcball_coords.y = -2.0*(float)y/(float)Height + 1.0 ;
        Ball_Mouse(Arcball,arcball_coords);
        Ball_Update(Arcball);
        glutPostRedisplay() ;
    }
    else if( Button == GLUT_RIGHT_BUTTON )
    {
        if( y - PrevY > 0 )
            Zoom  = Zoom * 1.03 ;
        else 
            Zoom  = Zoom * 0.97 ;
        PrevY = y ;
        glutPostRedisplay() ;
    }
}

int frame = 0;
double prev = 0;
int fps = 0;

void idleCB(void)
{
    if( Animate == 1 )
    {
		
        // TM.Reset() ; // commenting out this will make the time run from 0
        // leaving 'Time' counts the time interval between successive calls to idleCB
        if( Recording == 0 )
		{	TIME = TM.GetElapsedTime();}
        else
           { TIME += 0.033 ; // save at 30 frames per second.
		;
		}
		if(TIME<=7)
		{
       eye.x = 20*sin(TIME);
	   eye.z = 20*cos(TIME);
		}
		else if(TIME>=15 && TIME <=40)
		{
       eye.x = 20*sin(TIME);
	   eye.z = 20*cos(TIME);
		}

	
        
       frame++;

	double cur = TIME;
	double timeInterval = cur - prev;

		fps = frame / timeInterval;
		prev = cur;
		frame = 0;
		printf("TIME %f \n", TIME) ;
		;printf("FPS = %i\n", fps); 

      glutPostRedisplay() ; 
    }
}
/*********************************************************
     PROC: main()
     DOES: calls initialization, then hands over control
           to the event handler, which calls 
           display() whenever the screen needs to be redrawn
**********************************************************/

int main(int argc, char** argv) 
{
    glutInit(&argc, argv);
    // If your code fails to run, uncommenting these lines may help.
    //glutInitContextVersion(3, 2);
    //glutInitContextProfile(GLUT_CORE_PROFILE);
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowPosition (0, 0);
    glutInitWindowSize(Width,Height);
    glutCreateWindow(argv[0]);
    printf("GL version %s\n", glGetString(GL_VERSION));
    glewExperimental = GL_TRUE;
    glewInit();
	
    myinit();

    glutIdleFunc(idleCB) ;
    glutReshapeFunc (myReshape);
    glutKeyboardFunc( myKey );
    glutMouseFunc(myMouseCB) ;
    glutMotionFunc(myMotionCB) ;
    instructions();

    glutDisplayFunc(display);
    glutMainLoop();

    TM.Reset() ;
    return 0;         // never reached
}



