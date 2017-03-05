#include <set>
#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

struct VAO {
    GLuint VertexArrayID;
    GLuint VertexBuffer;
    GLuint ColorBuffer;

    GLenum PrimitiveMode;
    GLenum FillMode;
    int NumVertices;
};
typedef struct VAO VAO;
struct GLMatrices {
  glm::mat4 projection;
  glm::mat4 model;
  glm::mat4 view;
  GLuint MatrixID;
} Matrices;

GLuint programID;

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

  // Create the shaders
  GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
  GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

  // Read the Vertex Shader code from the file
  std::string VertexShaderCode;
  std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
  if(VertexShaderStream.is_open())
  {
    std::string Line = "";
    while(getline(VertexShaderStream, Line))
      VertexShaderCode += "\n" + Line;
    VertexShaderStream.close();
  }

  // Read the Fragment Shader code from the file
  std::string FragmentShaderCode;
  std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
  if(FragmentShaderStream.is_open()){
    std::string Line = "";
    while(getline(FragmentShaderStream, Line))
      FragmentShaderCode += "\n" + Line;
    FragmentShaderStream.close();
  }

  GLint Result = GL_FALSE;
  int InfoLogLength;

  // Compile Vertex Shader
  printf("Compiling shader : %s\n", vertex_file_path);
  char const * VertexSourcePointer = VertexShaderCode.c_str();
  glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
  glCompileShader(VertexShaderID);

  // Check Vertex Shader
  glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
  glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
  std::vector<char> VertexShaderErrorMessage(InfoLogLength);
  glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
  fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

  // Compile Fragment Shader
  printf("Compiling shader : %s\n", fragment_file_path);
  char const * FragmentSourcePointer = FragmentShaderCode.c_str();
  glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
  glCompileShader(FragmentShaderID);

  // Check Fragment Shader
  glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
  glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
  std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
  glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
  fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

  // Link the program
  fprintf(stdout, "Linking program\n");
  GLuint ProgramID = glCreateProgram();
  glAttachShader(ProgramID, VertexShaderID);
  glAttachShader(ProgramID, FragmentShaderID);
  glLinkProgram(ProgramID);

  // Check the program
  glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
  glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
  std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
  glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
  fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

  glDeleteShader(VertexShaderID);
  glDeleteShader(FragmentShaderID);

  return ProgramID;
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
    glfwDestroyWindow(window);
    glfwTerminate();
//    exit(EXIT_SUCCESS);
}


/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
    struct VAO* vao = new struct VAO;
    vao->PrimitiveMode = primitive_mode;
    vao->NumVertices = numVertices;
    vao->FillMode = fill_mode;

    // Create Vertex Array Object
    // Should be done after CreateWindow and before any other GL calls
    glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
    glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
    glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

    glBindVertexArray (vao->VertexArrayID); // Bind the VAO 
    glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
    glVertexAttribPointer(
                          0,                  // attribute 0. Vertices
                          3,                  // size (x,y,z)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
    glVertexAttribPointer(
                          1,                  // attribute 1. Color
                          3,                  // size (r,g,b)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
    GLfloat* color_buffer_data = new GLfloat [3*numVertices];
    for (int i=0; i<numVertices; i++) {
        color_buffer_data [3*i] = red;
        color_buffer_data [3*i + 1] = green;
        color_buffer_data [3*i + 2] = blue;
    }

    return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
    // Change the Fill Mode for this object
    glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

    // Bind the VAO to use
    glBindVertexArray (vao->VertexArrayID);

    // Enable Vertex Attribute 0 - 3d Vertices
    glEnableVertexAttribArray(0);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

    // Enable Vertex Attribute 1 - Color
    glEnableVertexAttribArray(1);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

    // Draw the geometry !
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

/**************************
 * Customizable functions *
 **************************/

float triangle_rot_dir = 1;
float rectangle_rot_dir = 1;
bool rectangle_rot_status = true;
bool triangle_rot_status = true;
bool pressed[10000];
/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
     // Function is called first on GLFW_PRESS.
    if (action == GLFW_RELEASE) {
            pressed[key] = false;
            if(key == GLFW_KEY_C) {
              rectangle_rot_status = !rectangle_rot_status;
            }

            if(key == GLFW_KEY_P) {
                triangle_rot_status = !triangle_rot_status;
            }
    }
    else if (action == GLFW_PRESS) {
          pressed[key] = true;
          if(key == GLFW_KEY_ESCAPE) {
            quit(window);
          }
    }
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
  switch (key) {
    case 'Q':
    case 'q':
            quit(window);
            break;
    default:
      break;
  }
}

void checkblock();
void checkcanon();
bool Clicked = false;
/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
    switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT:
            if (action == GLFW_RELEASE) {
                triangle_rot_dir *= -1;
                Clicked =false;
              }
            else if (action == GLFW_PRESS) {
               Clicked = true;
               checkblock();
               checkcanon();
            }
            break;  
        case GLFW_MOUSE_BUTTON_RIGHT:
            if (action == GLFW_RELEASE) {
                rectangle_rot_dir *= -1;
            }
            break;
        default:
            break;
    }
}


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
    int fbwidth=width, fbheight=height;
    /* With Retina display on Mac OS X, GLFW's FramebufferSize
     is different from WindowSize */
    glfwGetFramebufferSize(window, &fbwidth, &fbheight);

  GLfloat fov = 90.0f;

  // sets the viewport of openGL renderer
  glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

  // set the projection matrix as perspective
  /* glMatrixMode (GL_PROJECTION);
     glLoadIdentity ();
     gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
  // Store the projection matrix in a variable for future use
    // Perspective projection for 3D views
    // Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

    // Ortho projection for 2D views
    Matrices.projection = glm::ortho(-40.0f, 40.0f, -40.0f, 40.0f, 0.1f, 500.0f);
}

struct Piece {
  float x1, x2, y1, y2, trans;
  float color;
}; 

struct Lazer {
  float x1, y1, ang, x2, y2;
};

float Score = 0;
float zoom = 1, pan = 0;
float block_trans = 0.3;
vector<Lazer> L;
vector<VAO *> lazer;
VAO *battery, *battery_power, *battery_cell, *canonmid, *water, *Laz, *canonbase, *canonshooter, *baseline, *triangle, *rectangle, *basket1, *basket2, *block, *box[400], *mirror1, *mirror2, *mirror3;
Piece Block[400], current[400];
float b1 = 0, b2 = 0;
float c = 0;
float rot = 0;
bool Shoot = false;
float xi;
float yi;
float tempx, tempy;
float Pix = -36.5, Piy = 36.5;
float Pfy = 33.5, Pfx = Pix;
void createNewLazer(double x,double y,double ang,double x1=-800.0f,double y1=-800.0f) {

  float slope = tan(ang);
  if(x1==-800.0f && y1==-800.0f)
    {
      x1=1000.0000f;
      if(slope>0.00001f)
      y1=slope*1000.0000f+(y-slope*x);
      else
      {
        y1=-slope*1000.00000f+(y-slope*x);
        x1*=-1.0f;
      }
      tempx = x1;
      tempy = y1;
    }

  GLfloat vertex_buffer_data [] = {
    x, y, 0,
    x1, y1, 0
  };

  const GLfloat color_buffer_data [] = {
    0,0,1, // color 1
    0,0,1 // color 2
  };
  // create3DObject creates and returns a handle to a VAO that can be used later
  Laz= create3DObject(GL_LINES, 2, vertex_buffer_data, color_buffer_data, GL_LINE); 
}
void createBattery ()
{
  // GL3 accepts only Triangles. Quads are not supported 
  const GLfloat vertex_buffer_data1 [] = {
    -37, 37, 0,   // vertex 1
    -32, 37, 0,  // vertex 2
    -32, 33, 0,

    -32, 33, 0,
    -37, 33, 0, 
    -37, 37, 0 
   };

  const GLfloat color_buffer_data1 [] = {
    0,0,0,  //color 1
    0,0,0,  //color 2
    0,0,0,
    0,0,0,
    0,0,0,
    0,0,0
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  battery = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data1, color_buffer_data1, GL_LINE);


  // GL3 accepts only Triangles. Quads are not supported 
  const GLfloat vertex_buffer_data2 [] = {
    -32, 36, 0,   // vertex 1
    -30.1, 36, 0,  // vertex 2
    -30.1, 34, 0,

    -30.1, 34, 0,
    -32, 34, 0, 
    -32, 36, 0 
   };

  const GLfloat color_buffer_data2 [] = {
    0,0,0,  //color 1
    0,0,0,  //color 2
    0,0,0,
    0,0,0,
    0,0,0,
    0,0,0
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  battery_cell = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data2, color_buffer_data2, GL_FILL);

    // GL3 accepts only Triangles. Quads are not supported 
  const GLfloat vertex_buffer_data3 [] = {
    Pix, Piy, 0,
    Pfx, Piy, 0,
    Pfx, Pfy, 0,

    Pfx, Pfy, 0,
    Pix, Pfy, 0, 
    Pix, Piy, 0,
   };

  const GLfloat color_buffer_data3 [] = {
    0.3,1,0.1,  
    0.3,1,0.1,  
    0.3,1,0.1,
    0.3,1,0.1,
    0.3,1,0.1,
    0.3,1,0.1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  battery_power = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data3, color_buffer_data3, GL_FILL);

}

void createLazer ()
{
  // GL3 accepts only Triangles. Quads are not supported
  float x2 = 500;
  float x1 = -40;
  float y1 = c;
  float y2 = 540*tan(rot) + c;

  const GLfloat vertex_buffer_data1 [] = {
    x1, y1, 0,   // vertex 1
    x2, y2, 0  // vertex 2
   };

  const GLfloat color_buffer_data1 [] = {
    0,0,1,  //color 1
    0,0,1,  //color 2
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  Laz = create3DObject(GL_LINES, 2, vertex_buffer_data1, color_buffer_data1, GL_LINE);
}

struct Mirror { 
  float a, b, c;
  float x1, x2, y1, y2;
  float angle;
};

Mirror m[4];

void createMirrors ()
{
  // GL3 accepts only Triangles. Quads are not supported
  //PI/4 with the x-axis
  const GLfloat vertex_buffer_data1 [] = {
    -12,-10,0, // vertex 1
    -12+cos(M_PI/4),-cos(M_PI/4)-10,0, // vertex 2
    -12+9*cos(M_PI/4)+sin(M_PI/4),-10+9*cos(M_PI/4)-cos(M_PI/4),0, //vertex 3
   
    -12+9*cos(M_PI/4)+sin(M_PI/4),-10+9*cos(M_PI/4)-cos(M_PI/4),0, //vertex 3
    -12+9*cos(M_PI/4),-10+9*cos(M_PI/4),0,//vertex 4
    -12,-10,0 //vertex 1
  };

  float v1x = -12, v2x = -12+10*cos(M_PI/4), v1y = -10, v2y = -10+8*cos(M_PI/4);
  m[1].x1 = v1x, m[1].x2 = v2x, m[1].y1 = v1y, m[1].y2 = v2y;
  m[1].a = -(v2y - v1y);
  m[1].b = v2x - v1x;
  m[1].c = v1y*(v2x-v1x) - v1x*(v2y-v1y);
  m[1].angle = M_PI/4;
  const GLfloat color_buffer_data1 [] = {
    0,0,0,  //color 1
    0,0,0,  //color 2
    0,0,0,  //color 3

    0,0,0,  //color 3
    0,0,0,  //color 4
    0,0,0   //color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  mirror1 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data1, color_buffer_data1, GL_FILL);

  //2*PI/3 with the x-axis
  GLfloat vertex_buffer_data2 [] = {
    32,30,0, // vertex 1
    32+cos(M_PI/6),30+sin(M_PI/6),0, // vertex 2
    32+cos(M_PI/6)-9*cos(M_PI/3),30+sin(M_PI/6)+9*sin(M_PI/3),0, //vertex 3

    32+cos(M_PI/6)-9*cos(M_PI/3),30+sin(M_PI/6)+9*sin(M_PI/3),0, //vertex 3
    32-9*cos(M_PI/3),30+9*sin(M_PI/3),0,//vertex 4
    32,30,0 //vertex 1
  };

  GLfloat color_buffer_data2 [] = {
    0,0,0,  //color 1
    0,0,0,  //color 2
    0,0,0,  //color 3

    0,0,0,  //color 3
    0,0,0,  //color 4
    0,0,0   //color 1
  };

  v1x = 32, v2x = 32-9*cos(M_PI/3), v1y = 30, v2y = 30+9*sin(M_PI/3);
  m[2].x1 = v1x, m[2].x2 = v2x, m[2].y1 = v1y, m[2].y2 = v2y;
  m[2].a = -(v2y - v1y);
  m[2].b = v2x - v1x;
  m[2].c = v1y*(v2x-v1x) - v1x*(v2y-v1y);
  m[2].angle = (2*M_PI)/3;
  // create3DObject creates and returns a handle to a VAO that can be used later
  mirror2 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data2, color_buffer_data2, GL_FILL);

  //PI/3 with the x-axis
  GLfloat vertex_buffer_data3 [] = {
    25,-20,0, // vertex 1
    25+10*cos(M_PI/3),-20+10*sin(M_PI/3),0, // vertex 2
    25+10*cos(M_PI/3)+cos(M_PI/6),-20+10*sin(M_PI/3)-sin(M_PI/6),0,//vertex 3

    25+10*cos(M_PI/3)+cos(M_PI/6),-20+10*sin(M_PI/3)-sin(M_PI/6),0,//vertex 3
    25+cos(M_PI/6),-20-sin(M_PI/6),0,//vertex 4
    25,-20,0 //vertex 1
  };

  GLfloat color_buffer_data3 [] = {
    0,0,0,  //color 1
    0,0,0,  //color 2
    0,0,0,  //color 3

    0,0,0,  //color 3
    0,0,0,  //color 4
    0,0,0   //color 1
  };
  
  v1x = 25, v2x = 25+10*cos(M_PI/3), v1y = -20, v2y = -20+10*sin(M_PI/3);
  m[3].x1 = v1x, m[3].x2 = v2x, m[3].y1 = v1y, m[3].y2 = v2y;
  m[3].a = -(v2y - v1y);
  m[3].b = v2x - v1x;
  m[3].c = v1y*(v2x-v1x) - v1x*(v2y-v1y);
  m[3].angle = M_PI/3;
  // create3DObject creates and returns a handle to a VAO that can be used later
  mirror3 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data3, color_buffer_data3, GL_FILL);
}

void createRectangle ()
{
  // GL3 accepts only Triangles. Quads are not supported
  const GLfloat vertex_buffer_data [] = {
    -10,-40,0, // vertex 1
    -5,-40,0, // vertex 2
    -2.5, -30,0, // vertex 3

    -2.5, -30,0, // vertex 3
    -12.5, -30,0, // vertex 4
    -10,-40,0  // vertex 1
  };

  const GLfloat color_buffer_data [] = {
    1,0,0, // color 1
    0,0,1, // color 2
    0,1,0, // color 3

    0,1,0, // color 3
    0.3,0.3,0.3, // color 4
    1,0,0  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  rectangle = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

float gapx = 10, gapy = 10, maxy = -1; 
//create pieces
void createPieces (int i)
{
  // GL3 accepts only Triangles. Quads are not supported

  float val1 = (rand() % 50);
  val1 -= 30;
  Block[i].color = rand() % 3;
  Block[i].trans = 0;
  Block[i].x1 = val1 + gapx;
  Block[i].x2 = Block[i].x1 + 1;
  if(maxy == -1) { 
    Block[i].y1 = 40 + gapy; 
    Block[i].y2 = Block[i].y1 + 3;
    maxy = i;
  }
  else {
    Block[i].y1 = current[(int)maxy].y2 + gapy;
    Block[i].y2 = Block[i].y1 + 3;
    maxy = i;
  }
  float X1 = Block[i].x1;
  current[i].x1 = X1;
  float X2 = Block[i].x2;
  current[i].x2 = X2;
  float Y1 = Block[i].y1;
  current[i].y1 = Y1;
  float Y2 = Block[i].y2;
  current[i].y2 = Y2;
  current[i].color = Block[i].color;
  current[i].trans = Block[i].trans;
  
  const GLfloat vertex_buffer_data [] = {
    X1, Y1,0, // vertex 1
    X2, Y1,0, // vertex 2
    X2, Y2,0, // vertex 3

    X2, Y2,0, // vertex 3
    X1, Y2,0, // vertex 4
    X1, Y1,0  // vertex 1
  };

  //GLfloat color_buffer_data [20];

  if(current[i].color == 0) { 
      const GLfloat color_buffer_data [] = {
      1,0,0, // color 1
      1,0,0, // color 2
      1,0,0, // color 3

      1,0,0, // color 3
      1,0,0, // color 4
      1,0,0  // color 1
    };
  // create3DObject creates and returns a handle to a VAO that can be used later
  box[i] = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);

  }
  else if(current[i].color == 1) { 
      const GLfloat color_buffer_data [] = {
      0,1,0, // color 1
      0,1,0, // color 2
      0,1,0, // color 3

      0,1,0, // color 3
      0,1,0, // color 4
      0,1,0  // color 1
    };

    // create3DObject creates and returns a handle to a VAO that can be used later
    box[i] = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
  }

  else if(current[i].color == 2) { 
     const  GLfloat color_buffer_data [] = {
      0,0,0, // color 1
      0,0,0, // color 2
      0,0,0, // color 3

      0,0,0, // color 3
      0,0,0, // color 4
      0,0,0  // color 1
    };

  // create3DObject creates and returns a handle to a VAO that can be used later
  box[i] = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
  }
}
  
void createWater()
{
  // GL3 accepts only Triangles. Quads are not supported
  //basket1 specifications
  const GLfloat vertex_buffer_data1 [] = {
    -40, -40, 0,
    40, -40, 0,
    40, -36.7, 0,

    40, -36.7, 0,
    -40, -36.7, 0,
    -40, -40, 0
  };

  const GLfloat color_buffer_data1 [] = {
    0,1,1, // color 1
    0,1,1, // color 2
    0,1,1, // color 3

    0,1,1, // color 3
    0,1,1, // color 4
    0,1,1  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  water = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data1, color_buffer_data1, GL_FILL);
}

void createBaskets()
{
  // GL3 accepts only Triangles. Quads are not supported
  //basket1 specifications
  const GLfloat vertex_buffer_data1 [] = {
    -10,-40,0, // vertex 1
    -5,-40,0, // vertex 2
    -2.5, -35,0, // vertex 3

    -2.5, -35,0, // vertex 3
    -12.5, -35,0, // vertex 4
    -10,-40,0  // vertex 1
  };

  const GLfloat color_buffer_data1 [] = {
    1,0,0, // color 1
    1,0,0, // color 2
    1,0,0, // color 3

    1,0,0, // color 3
    1,0,0, // color 4
    1,0,0  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  basket1 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data1, color_buffer_data1, GL_FILL);

  //basket2 specifications 
  const GLfloat vertex_buffer_data2 [] = {
    10,-40,0, // vertex 1
    5,-40,0, // vertex 2
    2.5, -35,0, // vertex 3

    2.5, -35,0, // vertex 3
    12.5, -35,0, // vertex 4
    10,-40,0  // vertex 1
  };

  const GLfloat color_buffer_data2 [] = {
    0,1,0, // color 1
    0,1,0, // color 2
    0,1,0, // color 3

    0,1,0, // color 3
    0,1,0, // color 4
    0,1,0  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  basket2 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data2, color_buffer_data2, GL_FILL);
  
  return; 
}

void createCanon()
{
  // GL3 accepts only Triangles. Quads are not supported
  // canon base 
  const GLfloat vertex_buffer_data1 [] = {
    -40, -4, 0,
    -38.5, -4, 0,
    -38.5, 4, 0,
    
    -38.5, 4, 0,
    -40, 4, 0,
    -40, -4,0
  };

  const GLfloat color_buffer_data1 [] = {
    0,0,0, // color 1
    0,0,0, // color 2
    0,0,0, // color 3

    0,0,0, // color 3
    0,0,0, // color 4
    0,0,0  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  canonbase = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data1, color_buffer_data1, GL_FILL);

  //canon mid
  const GLfloat vertex_buffer_data3 [] = {
    -40, -3, 0,
    -36.5, -2, 0,
    -36.5, 2, 0,
    
    -36.5, 2, 0,
    -40, 3, 0,
    -40, -3,0
  };

  const GLfloat color_buffer_data3 [] = {
    0,0,0, // color 1
    0,0,0, // color 2
    0,0,0, // color 3

    0,0,0, // color 3
    0,0,0, // color 4
    0,0,0  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  canonmid = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data3, color_buffer_data3, GL_FILL);

  //shooter
  const GLfloat vertex_buffer_data2 [] = {
    -40, -1, 0,
    -33, -1, 0,
    -33, 1, 0,
    
    -33, 1, 0,
    -40, 1, 0,
    -40, -1, 0
  };

  const GLfloat color_buffer_data2 [] = {
    0,0,0, // color 1
    0,0,0, // color 2
    0,0,0, // color 3

    0,0,0, // color 3
    0,0,0, // color 4
    0,0,0  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  canonshooter = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data2, color_buffer_data2, GL_FILL);
  
  return; 
}

float camera_rotation_angle = 90;
float rectangle_rotation = 0;
float triangle_rotation = 0;

/* Render the scene with openGL */
/* Edit this function according to your assignment */
void draw ()
{
  // clear the color and depth n the frame buffer
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // use the loaded shader program
  // Don't change unless you know what you are doing
  glUseProgram (programID);

  // Eye - Location of camera. Don't change unless you are sure!!
  glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
  // Target - Where is the camera looking at.  Don't change unless you are sure!!
  glm::vec3 target (0, 0, 0);
  // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
  glm::vec3 up (0, 1, 0);

  // Compute Camera matrix (view)
  // Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
  //  Don't change unless you are sure!!
  Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

  Matrices.projection = glm::ortho(-40.0f/zoom + pan, 40.0f/zoom + pan, -40.0f/zoom, 40.0f/zoom, 0.1f/zoom, 500.0f/zoom);

  // Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
  //  Don't change unless you are sure!!
  glm::mat4 VP = Matrices.projection * Matrices.view;

  // Send our transformation to the currently bound shader, in the "MVP" uniform
  // For each model you render, since the MVP will be different (at least the M part)
  //  Don't change unless you are sure!!
  glm::mat4 MVP;  // MVP = Projection * View * Model
  glm::mat4 translatePiece, translate1, translate2; 
   glm::mat4 rotateCannons; 
  /* Water */
  Matrices.model = glm::mat4(1.0f);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(water);

   /* Canon */
  //create lazer
  if(Shoot) {
      for(int i = 0; i < (int)lazer.size(); i++ ) {
        Matrices.model = glm::mat4(1.0f); 
        MVP = VP * Matrices.model;
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        draw3DObject(lazer[i]);
      }
  }
  //draw canonbase
  Matrices.model = glm::mat4(1.0f); 
  translatePiece = glm::translate (glm::vec3(0, c, 0));
  translate1 = glm::translate (glm::vec3(40, 0, 0));
  translate2 = glm::translate (glm::vec3(-40, 0, 0));
  rotateCannons = glm::rotate((float)(rot), glm::vec3(0,0,1));
  Matrices.model *= translatePiece*translate2*rotateCannons*translate1; 
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(canonshooter);

  Matrices.model = glm::mat4(1.0f); 
  translatePiece = glm::translate (glm::vec3(0, c, 0));
  translate1 = glm::translate (glm::vec3(40, 0, 0));
  translate2 = glm::translate (glm::vec3(-40, 0, 0));
  rotateCannons = glm::rotate((float)(rot), glm::vec3(0,0,1));
  Matrices.model *= translatePiece*translate2*rotateCannons*translate1; 
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(canonmid);
  
  //draw canonshooter
  Matrices.model = glm::mat4(1.0f); 
  translatePiece = glm::translate (glm::vec3(0, c, 0));
  Matrices.model *= translatePiece; 
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(canonbase);

  //draw basket 1
  Matrices.model = glm::mat4(1.0f); 
  translatePiece = glm::translate (glm::vec3(b1, 0, 0));
  Matrices.model *= translatePiece; 
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(basket1);

  //draw basket 2
  Matrices.model = glm::mat4(1.0f);
  translatePiece = glm::translate (glm::vec3(b2, 0, 0));
  Matrices.model *= translatePiece; 
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(basket2);


  //mirror1
  Matrices.model = glm::mat4(1.0f);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(mirror1);

  //mirror2
  Matrices.model = glm::mat4(1.0f);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(mirror2);

  //mirror3
  Matrices.model = glm::mat4(1.0f);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(mirror3);


  for(int i = 1; i <= 20; i++) { 
    Matrices.model = glm::mat4(1.0f);  
    translatePiece = glm::translate (glm::vec3(0, current[i].trans, 0));
    Matrices.model *= translatePiece;
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(box[i]);
    current[i].y1 -= block_trans;
    current[i].y2 -= block_trans;
    //translating pieces 
    current[i].trans -= block_trans;
  }
  //battery
  Matrices.model = glm::mat4(1.0f);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(battery);


  //battery_cell
  Matrices.model = glm::mat4(1.0f);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(battery_cell);


  //battery_power
  Matrices.model = glm::mat4(1.0f);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(battery_power);

  // Increment angles
  float increments = 1;

  //camera_rotation_angle++; // Simulating camera rotation
  triangle_rotation = triangle_rotation + increments*triangle_rot_dir*triangle_rot_status;
  rectangle_rotation = rectangle_rotation + increments*rectangle_rot_dir*rectangle_rot_status;
}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
void scroll_callback(GLFWwindow*, double, double);

GLFWwindow* initGLFW (int width, int height)
{
    GLFWwindow* window; // window desciptor/handle

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
      //exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

    if (!window) {
        glfwTerminate();
        //exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval( 1 );
    glfwSetScrollCallback(window, scroll_callback);

    /* --- register callbacks with GLFW --- */
    /* Register function to handle window resizes */
    /* With Retina display on Mac OS X GLFW's FramebufferSize
     is different from WindowSize */
    glfwSetFramebufferSizeCallback(window, reshapeWindow);
    glfwSetWindowSizeCallback(window, reshapeWindow);

    /* Register function to handle window close */
    glfwSetWindowCloseCallback(window, quit);

    /* Register function to handle keyboard input */
    glfwSetKeyCallback(window, keyboard);      // general keyboard input
    glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

    /* Register function to handle mouse click */
    glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks

    return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
    /* Objects should be created before any other gl function and shaders */
  
  // Create the models
  //creating the pieces of the game 
  for(int i = 1; i <= 20; i++) { 
    createPieces(i);
  }
  createBattery();
  createWater();
  //creating the baskets at the bottom
  createBaskets(); 
  //creating the mirrors for reflection of the lazer
  createMirrors();
  //creating canon
  createCanon();
  //the lazer from the cannon
  // Create and compile our GLSL program from the shaders
  programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
  // Get a handle for our "MVP" uniform
  Matrices.MatrixID = glGetUniformLocation(programID, "MVP");

  
  reshapeWindow (window, width, height);

  // Background color of the scene
  glClearColor (1.0f, 1.0f, 1.0f, 1.0f); // R, G, B, A
  glClearDepth (1.0f);

  glEnable (GL_DEPTH_TEST);
  glDepthFunc (GL_LEQUAL);

    cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
    cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
    cout << "VERSION: " << glGetString(GL_VERSION) << endl;
    cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

void translate_() {
  if(pressed[GLFW_KEY_LEFT] && !pressed[GLFW_KEY_LEFT_CONTROL] && !pressed[GLFW_KEY_LEFT_ALT] && pan > -10)  pan -= 0.1;
  if(pressed[GLFW_KEY_RIGHT] && !pressed[GLFW_KEY_LEFT_CONTROL] && !pressed[GLFW_KEY_LEFT_ALT]&& pan < 10) pan += 0.1;
  if(pressed[GLFW_KEY_UP] && zoom < 1.5) zoom += 0.01;
  if(pressed[GLFW_KEY_DOWN] && zoom > 1) zoom -= 0.01;
  if(pressed[GLFW_KEY_N]) block_trans+=0.02;
  if(pressed[GLFW_KEY_M] && block_trans > 0.05) block_trans-=0.02;
  if(pressed[GLFW_KEY_LEFT_CONTROL] && pressed[GLFW_KEY_LEFT]) {
    if(b1 >= -32) {
      b1-= 0.3;
    }
  }
  else if(pressed[GLFW_KEY_LEFT_CONTROL] && pressed[GLFW_KEY_RIGHT]) {
    if(b1 <= 46) {
      b1+= 0.3;
    }
  }
  else if(pressed[GLFW_KEY_LEFT_ALT] && pressed[GLFW_KEY_LEFT]) {
    if(b2 >= -46) {
      b2-= 0.3;
    }
  }
  else if(pressed[GLFW_KEY_LEFT_ALT] && pressed[GLFW_KEY_RIGHT]) {
    if(b2 <= 32) {
      b2+= 0.3;
    }
  } 

  if(pressed[GLFW_KEY_S]) { 
    if(c <= 37) {
      c+=0.3;
    }
  }
  else if(pressed[GLFW_KEY_F]) { 
    if(c >= -37) {
      c-= 0.3;
    }
  }
}

void rotate_canon()  { 
  if(pressed[GLFW_KEY_A]) { 
    if(2*rot < M_PI) {
      rot+=0.03;
    }
  }
  else if(pressed[GLFW_KEY_D]) {
    if(2*rot > -M_PI) {
      rot-=0.03;
    }
  }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
  if(yoffset == 1 && zoom < 1.5) {
    zoom += 0.01;
  }
  else if(yoffset == -1 && zoom > 1) {
    zoom -= 0.01;
  }
}

int a = 0;
void checkhit(int j) {
  int i, k;

  // cout << "in checkhit - start" << endl;
  // for(k = 0; k < (int)L.size(); k++) {
  //   cout << L[k].x1 << ' ' << L[k].x2 << endl;
  // }
  // cout << "in checkhit - end" << endl;
  int r;
  //cout << j << endl;
  float x1 = L[j].x1, y1 = L[j].y1, ang = L[j].ang;
  float x2 = L[j].x2, y2 = L[j].y2;
  for(int i = 1; i <= 20; i++) {
      // float y = (current[i].x1 - x1)*tan(ang) + y1;
      // if((y >= current[i].y1) && (y <= (current[i].y1 + 3)) && (current[i].y1 <= 40) && (current[i].y1 >= -40)) { 
      //   if(current[i].x1 >= min(L[j].x1, L[j].x2) && current[i].x1 <= max(L[j].x1, L[j].x2)) {
          
          float y = (current[i].x1 - L[j].x1) * tan(ang) + L[j].y1;
          if(y >= min(current[i].y1, current[i].y2) && y <= max(current[i].y1, current[i].y2) && current[i].x1 >= min(L[j].x1, L[j].x2) && current[i].x1 <= max(L[j].x1, L[j].x2) ) {   
            //cout << x1 << ' ' << x2 << ' ' << endl;
            // cout << j << ' ' << L.size() << endl;
            // cout << x1 << ' ' << current[i].x1 << ' ' << x2 << endl;
            //cout << "hit" << endl;
            // cout << "info" << endl;
            // cout << L.size() << endl;
            // cout << a << endl;
            // cout << L[j].x1 << ' ' << L[j].x2 << " " << j << endl;
            //cout << "hit done" << endl;
            if( (L.size() == 1 && x1 == -40 && x2 == 500) || L.size() >= 2)
              { 
                if(current[i].color == 2) {
                  Score += 100;
                }
                else 
                {
                  Score -= 10;
                }
                createPieces(i);
              }
        }
      }
  return ;
}

#define FF pair<float, float> 

FF solve_lines(float a1, float b1, float c1, float a2, float b2, float c2) { 
  return {(c2*b1-b2*c1)/(a2*b1-a1*b2), (c1*a2-a1*c2)/(a2*b1-a1*b2)};
}

int k = 1;
void LazerWithMirror(set<int> s) {
  for(int i = 1; i <= 3; i++) {
      if(s.count(i) == 1) continue;
      float A = -tan(L[L.size()-1].ang);
      float B = 1;
      float C = L[L.size()-1].y1 + A*L[L.size()-1].x1;
      FF t = solve_lines(m[i].a, m[i].b, m[i].c, A, B, C); 
      float x = t.first;
      float y = t.second;
      if( x >= min(m[i].x1, m[i].x2) && x <= max(m[i].x2, m[i].x1) )  {
          float ang_reflected = 2*m[i].angle - L[L.size()-1].ang;
          if(2*m[i].angle > M_PI) {
            ang_reflected -= M_PI;          
          }
          float X = L[L.size()-1].x1, Y = L[L.size()-1].y1, ANG = L[L.size()-1].ang;
          lazer.pop_back();
          L.pop_back();
          createNewLazer(X, Y, ANG, x, y); //for the incident lazer
          lazer.push_back(Laz);
          L.push_back((Lazer){X, Y, ANG, x, y});
          checkhit(L.size()-1);
          if(2*m[i].angle > M_PI) {
            float yy = (tan(ang_reflected) * (-700-x)) + y; 
            createNewLazer(x, y, ang_reflected, -700.0f, yy); //for the reflected lazer
          }
          else if(ang_reflected  < 0) {
            createNewLazer(x, y, ang_reflected, 800.0f, (800.0f-x)*tan(ang_reflected) +y );
          } 
          else {
            createNewLazer(x, y, ang_reflected);
          }
          L.push_back((Lazer){x, y, ang_reflected, tempx, tempy});
          lazer.push_back(Laz);
          // cout << L.size() << endl;
          // cout << "start" << endl;
          // for(int i = 0; i < L.size(); i++) {
          //   cout << L[i].x1 << ' ' << L[i].x2 << endl;
          // }

          // cout << "end" << endl;
          //cout << L.size() << endl;
          a++;
          checkhit(L.size()-1);
          s.insert(i);
          LazerWithMirror(s);
          s.erase(i);
          break;
        }
    }
}

void shoot() { 
  if(pressed[GLFW_KEY_SPACE]) Shoot = true;
  else { 
    Shoot = false;
    lazer.clear();
    L.clear();
  }
  if(Shoot && (Pfx > Pix) ) {
    if(Pfx >= Pix) Pfx -= 0.07;
    if(Pfx <= Pix) return ;
    xi = -40;
    yi = c;
    // L.clear();
    // lazer.clear();
    createLazer();
    lazer.push_back(Laz);
    L.push_back((Lazer){-40, c, rot, 500, 540*tan(rot) + c});

    set<int> s;
    checkhit(0);
    LazerWithMirror(s);
  }
}

GLFWwindow *Window;
float prevx2= 6, prevx1 = -6;
bool block1 = false;
bool block2 = false;
bool canon = false;

void MouseControl_baskets() {
  double currx, curry;
  glfwGetCursorPos(Window, &currx, &curry);
  currx = (currx*2*40/600) - 40.0;
  curry = 40.0 - (curry*2*40/600);
  if(Clicked) {
    if(block1) {
      b1 += currx - prevx1;
      prevx1 = currx;
    }
    else if(block2) {
      b2 += currx - prevx2;
      prevx2 = currx;
    }
  }
  else {
    Clicked = false;
    block1 = block2 = false;
  }
}

void checkblock() { 
  double mousex, mousey;
  glfwGetCursorPos(Window, &mousex, &mousey); 
  mousex = (mousex*2*40/600) - 40.0;
  mousey = 40.0-(mousey*2*40/600);
  //check for basket1
  double currx = mousex, curry = mousey;
 
  if(currx <= b2 + 10 && currx >= b2 + 5 && curry >= -43 && curry <= -35)
    block2 = true;
  else if(currx >= b1-10 && currx <= b1-5 && curry >= -43 && curry <= -35)
    block1 = true;
}

double prevy = c;

void MouseControl_canon() {
  double currx, curry;
  glfwGetCursorPos(Window, &currx, &curry);
  currx = (currx*2*40/600) - 40.0;
  curry = 40.0 - (curry*2*40/600);
  if(Clicked) {
    if(canon) {
      c += curry - prevy;
      prevy = curry;
    }
  }
  else {
    Clicked = false;
    canon = false;
    prevy = c;
  }
}

void checkcanon() { 
  double mousex, mousey;
  glfwGetCursorPos(Window, &mousex, &mousey); 
  mousex = (mousex*2*40/600) - 40.0;
  mousey = 40.0-(mousey*2*40/600);
  //check for basket1
  double currx = mousex, curry = mousey;
 
  if(currx <= -35 && currx >= -40 && curry >= c - 5 && curry <= c + 5) {
    canon = true;
  }
}

void shoot_mouse() {
  double mousex, mousey;
  glfwGetCursorPos(Window, &mousex, &mousey); 
  mousex = (mousex*2*40/600) - 40.0;
  mousey = 40.0-(mousey*2*40/600);
  float slope = (mousey-c)/(mousex+40);
  if(Clicked && mousex >= -30 && mousey >= -34) {
    Shoot = true;
    rot = atan(slope);
    createNewLazer(c, -40, atan(slope), mousex, mousey);
    lazer.push_back(Laz);
    L.push_back((Lazer){-40, c, atan(slope), mousex, mousey});
  }
}
int main (int argc, char** argv)
{ 
  int width = 600;
  int height = 600;

  GLFWwindow* window = initGLFW(width, height);
  Window = window;

  initGL (window, width, height);

    double last_update_time = glfwGetTime(), current_time;

    /* Draw in loop */
    while (!glfwWindowShouldClose(window)) {
        if(Pfx <= -32.5) {
          Pfx+=0.03;
        }
        createBattery();
        // OpenGL Draw commands
        L.clear();
        lazer.clear();
        translate_();
        rotate_canon();
        shoot(); 
        draw();
        MouseControl_baskets();
        shoot_mouse();
        MouseControl_canon();
        // Swap Frame Buffer in double buffering
        glfwSwapBuffers(window);

        // Poll for Keyboard and mouse events
        glfwPollEvents();

        // Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
        current_time = glfwGetTime(); // Time in seconds
        if ((current_time - last_update_time) >= 0.5) { // atleast 0.5s elapsed since last frame
            // do something every 0.5 seconds ..
            last_update_time = current_time;
        }

        for(int i = 1; i <= 20; i++) { 
          if(current[i].y1 <= -37) {
              if(current[i].color == 2) {
                  quit(window);
                  cout << "Game Over!" << endl;
                  cout << "Your final Score is: " << Score << endl;
                  return 0;
              }
              else if(current[i].color == 0) {
                if(current[i].x1 <= b1 - 2.5 && current[i].x1 >= b1 - 12.5)
                  Score += 100;
              }
              else if(current[i].color == 1) {
                if(current[i].x1 >= b2 + 2.5 && current[i].x1 <= b2 + 12.5)
                  Score += 100; 
              }
              createPieces(i);
          }
          cout << "Current Score is: " << Score << endl;
        }

        if(Shoot) {
          for(int i = 0; i < L.size(); i++) {
            checkhit(i);
          }
        }
    }

    glfwTerminate();
//    exit(EXIT_SUCCESS);
}


//glfwGetCursorPos(window, &x, &y);