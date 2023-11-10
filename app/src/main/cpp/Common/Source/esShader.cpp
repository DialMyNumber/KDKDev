// The MIT License (MIT)
//
// Copyright (c) 2013 Dan Ginsburg, Budirijanto Purnomo
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

//
// Book:      OpenGL(R) ES 3.0 Programming Guide, 2nd Edition
// Authors:   Dan Ginsburg, Budirijanto Purnomo, Dave Shreiner, Aaftab Munshi
// ISBN-10:   0-321-93388-5
// ISBN-13:   978-0-321-93388-1
// Publisher: Addison-Wesley Professional
// URLs:      http://www.opengles-book.com
//            http://my.safaribooksonline.com/book/animation-and-3d/9780133440133
//
// ESShader.c
//
//    Utility functions for loading shaders and creating program objects.
//

///
//  Includes
//
#include <Common/Include/common.h>
#include <stdlib.h>
#include <stdio.h>
#ifdef ANDROID
#include <android_fopen.h>
#endif

//////////////////////////////////////////////////////////////////
//
//  Private Functions
//
//



//////////////////////////////////////////////////////////////////
//
//  Public Functions
//
//

//
///
/// \brief Load a shader, check for compile errors, print error messages to output log
/// \param type Type of shader (GL_VERTEX_SHADER or GL_FRAGMENT_SHADER)
/// \param shaderSrc Shader source string
/// \return A new shader object on success, 0 on failure
//
GLuint esLoadShader ( GLenum type, const char *shaderSrc )
{
   GLuint shader;
   GLint compiled;

   // Create the shader object
   shader = glCreateShader ( type );

   if ( shader == 0 )
   {
      return 0;
   }

   // Load the shader source
   glShaderSource ( shader, 1, &shaderSrc, NULL );

   // Compile the shader
   glCompileShader ( shader );

   // Check the compile status
   glGetShaderiv ( shader, GL_COMPILE_STATUS, &compiled );

   if ( !compiled )
   {
      GLint infoLen = 0;

      glGetShaderiv ( shader, GL_INFO_LOG_LENGTH, &infoLen );

      if ( infoLen > 1 )
      {
         char *infoLog = (char*) malloc ( sizeof ( char ) * infoLen );

         glGetShaderInfoLog ( shader, infoLen, NULL, infoLog );
         LogMessage ("POTT", "Error compiling shader:\n%s\n", infoLog );

         free ( infoLog );
      }

      glDeleteShader ( shader );
      return 0;
   }

   return shader;

}


//
///
/// \brief Load a vertex and fragment shader, create a program object, link program.
//         Errors output to log.
/// \param vertShaderSrc Vertex shader source code
/// \param fragShaderSrc Fragment shader source code
/// \return A new program object linked with the vertex/fragment shader pair, 0 on failure
//
GLuint esLoadProgram ( const char *vertShaderSrc, const char *fragShaderSrc )
{
   //GLuint vertexShader;
   //GLuint fragmentShader;
   GLuint programObject;
   GLint linked;

   // Load the vertex/fragment shaders
   GLuint vertexShader = esLoadShader ( GL_VERTEX_SHADER, vertShaderSrc );

   if ( vertexShader == 0 )
   {
      return 0;
   }

   GLuint fragmentShader = esLoadShader ( GL_FRAGMENT_SHADER, fragShaderSrc );

   if ( fragmentShader == 0 )
   {
      glDeleteShader ( vertexShader );
      return 0;
   }

   // Create the program object
   programObject = glCreateProgram ( );

   if ( programObject == 0 )
   {
      return 0;
   }

   glAttachShader ( programObject, vertexShader );
   glAttachShader ( programObject, fragmentShader );

   // Free up no longer needed shader resources
   glDeleteShader ( vertexShader );
   glDeleteShader ( fragmentShader );

   return programObject;
}

GLuint esLinkProgram(GLuint programObject)
{
    GLint linked;
   // Link the program
   glLinkProgram ( programObject );

   // Check the link status
   glGetProgramiv ( programObject, GL_LINK_STATUS, &linked );

   if ( !linked )
   {
      GLint infoLen = 0;

      glGetProgramiv ( programObject, GL_INFO_LOG_LENGTH, &infoLen );

      if ( infoLen > 1 )
      {
         char *infoLog = (char*) malloc ( sizeof ( char ) * infoLen );

         glGetProgramInfoLog ( programObject, infoLen, NULL, infoLog );
         LogMessage ("POTT", "Error linking program:\n%s\n", infoLog );

         free ( infoLog );
      }

      glDeleteProgram ( programObject );
      return 0;
   }
   return programObject;
}

char *esShaderFileRead(const char *filename)
{
    FILE *shaderFile;
    char *shaderSrc;
    long size;

    shaderFile = fopen( filename, "rb");

    if ( shaderFile == NULL)
        return 0;

    fseek( shaderFile, 0, SEEK_END);
    size = ftell(shaderFile);
    fseek( shaderFile, 0, SEEK_SET);
    shaderSrc = new char[size+1];
    fread( shaderSrc, size, 1, shaderFile);
    fclose( shaderFile);
    shaderSrc[size] = '\0';

    return shaderSrc;
}

void esShaderFileFree(char *shaderSrc)
{
    delete [] shaderSrc;
}

GLuint esLoadShaderFromFile ( GLenum type, const char *filename )
{
   char *shaderSrc;

   shaderSrc = esShaderFileRead(filename);
   GLuint shader = esLoadShader (type, shaderSrc);
   esShaderFileFree(shaderSrc);

   return shader;
}

GLuint esLoadProgramFromFile ( const char *vertShaderFile, const char *fragShaderSrc )
{
   GLuint vertexShader;
   GLuint fragmentShader;
   GLuint programObject;
   GLint linked;

   // Load the vertex/fragment shaders
   vertexShader = esLoadShaderFromFile ( GL_VERTEX_SHADER, vertShaderFile );

   if ( vertexShader == 0 )
   {
      return 0;
   }

   fragmentShader = esLoadShaderFromFile ( GL_FRAGMENT_SHADER, fragShaderSrc );

   if ( fragmentShader == 0 )
   {
      glDeleteShader ( vertexShader );
      return 0;
   }

   // Create the program object
   programObject = glCreateProgram ( );

   if ( programObject == 0 )
   {
      return 0;
   }

   glAttachShader ( programObject, vertexShader );
   glAttachShader ( programObject, fragmentShader );

   // Link the program
   glLinkProgram ( programObject );

   // Check the link status
   glGetProgramiv ( programObject, GL_LINK_STATUS, &linked );

   if ( !linked )
   {
      GLint infoLen = 0;

      glGetProgramiv ( programObject, GL_INFO_LOG_LENGTH, &infoLen );

      if ( infoLen > 1 )
      {
         char *infoLog = (char*) malloc ( sizeof ( char ) * infoLen );

         glGetProgramInfoLog ( programObject, infoLen, NULL, infoLog );
         LogMessage ("POTT", "Error linking program:\n%s\n", infoLog );

         free ( infoLog );
      }

      glDeleteProgram ( programObject );
      return 0;
   }

   // Free up no longer needed shader resources
   glDeleteShader ( vertexShader );
   glDeleteShader ( fragmentShader );

   return programObject;
}





// Added by Jae-Ho Nah //2015-03-09
//
///
/// \brief Load a vertex and fragment shader, create a program object, link program.
//         Errors output to log.
/// \param vertShaderSrc Vertex shader source code
/// \param fragShaderSrc Fragment shader source code
/// \return A new program object linked with the vertex/fragment shader pair, 0 on failure
//


GLuint esLoadProgramFromFileTessellation
		( const char *vertShaderFile,
		const char *tessControlShaderFile,
		const char *tessEvaluationShaderFile,
		const char *fragShaderFile )
{
   GLuint vertexShader;
   GLuint tessControlShader;
   GLuint tessEvaluationShader;
   GLuint fragmentShader;
   GLuint programObject;
   GLint linked;

   LogMessage ("POTT", "start to load shaders: %s", vertShaderFile);

   // Load the vertex/tessellation/fragment shaders
   vertexShader = esLoadShaderFromFile ( GL_VERTEX_SHADER, vertShaderFile );

   if ( vertexShader == 0 )
   {
	  LogMessage ("POTT", "failed to load vertex shader");
      return 0;
   }

   LogMessage ("POTT", "start to load shaders: %s", tessControlShaderFile);
   tessControlShader = esLoadShaderFromFile ( GL_TESS_CONTROL_SHADER_EXT, tessControlShaderFile);
   if ( tessControlShader == 0 )
   {
	   LogMessage ("POTT", "failed to load tess control shader");
	   glDeleteShader ( vertexShader );
       return 0;
   }

   LogMessage ("POTT", "start to load shaders: %s", tessEvaluationShaderFile);
   tessEvaluationShader = esLoadShaderFromFile ( GL_TESS_EVALUATION_SHADER_EXT, tessEvaluationShaderFile );
   if ( tessEvaluationShader == 0 )
   {
	   LogMessage ("POTT", "failed to load tess eval shader");
	   glDeleteShader ( vertexShader );
	   glDeleteShader ( tessControlShader );
      return 0;
   }


   LogMessage ("POTT", "start to load shaders: %s", fragShaderFile);
   fragmentShader = esLoadShaderFromFile ( GL_FRAGMENT_SHADER, fragShaderFile );

   if ( fragmentShader == 0 )
   {
	   LogMessage ("POTT", "failed to load frag shader");
	  glDeleteShader ( vertexShader );
	  glDeleteShader ( tessControlShader );
	  glDeleteShader ( tessEvaluationShader );
      return 0;
   }

   // Create the program object
   programObject = glCreateProgram ( );

   if ( programObject == 0 )
   {
      return 0;
   }

   glAttachShader ( programObject, vertexShader );
   glAttachShader ( programObject, tessControlShader );
   glAttachShader ( programObject, tessEvaluationShader );
   glAttachShader ( programObject, fragmentShader );

   // Link the program
   glLinkProgram ( programObject );

   // Check the link status
   glGetProgramiv ( programObject, GL_LINK_STATUS, &linked );

   if ( !linked )
   {
      GLint infoLen = 0;

      glGetProgramiv ( programObject, GL_INFO_LOG_LENGTH, &infoLen );

      if ( infoLen > 1 )
      {
         char *infoLog = (char*) malloc ( sizeof ( char ) * infoLen );

         glGetProgramInfoLog ( programObject, infoLen, NULL, infoLog );
         LogMessage ("POTT", "Error linking program:\n%s\n", infoLog );

         free ( infoLog );
      }

      glDeleteProgram ( programObject );
      return 0;
   }



   // Free up no longer needed shader resources
   glDeleteShader ( vertexShader );
   glDeleteShader ( tessControlShader );
   glDeleteShader ( tessEvaluationShader );
   glDeleteShader ( fragmentShader );

   return programObject;
}

GLuint esLoadProgramTessellation ( const char *vertShaderSrc,
		const char *tessControlShaderSrc,
		const char *tessEvaluationShaderSrc,
		const char *fragShaderSrc )
{
   GLuint programObject;
   GLint linked;

   GLuint vertexShader = esLoadShader ( GL_VERTEX_SHADER, vertShaderSrc );
   if ( vertexShader == 0 )
   {
      return 0;
   }

   GLuint tessControlShader = esLoadShader ( GL_TESS_CONTROL_SHADER_EXT, tessControlShaderSrc );
   if ( tessControlShader == 0 )
   {
	   glDeleteShader ( vertexShader );
       return 0;
   }
   GLuint tessEvaluationShader = esLoadShader ( GL_TESS_EVALUATION_SHADER_EXT, tessEvaluationShaderSrc);
   if ( tessEvaluationShader == 0 )
   {
	   glDeleteShader ( vertexShader );
	   glDeleteShader ( tessControlShader );
      return 0;
   }


   GLuint fragmentShader = esLoadShader ( GL_FRAGMENT_SHADER, fragShaderSrc );
   if ( fragmentShader == 0 )
   {
      glDeleteShader ( vertexShader );
	  glDeleteShader ( tessControlShader );
	  glDeleteShader ( tessEvaluationShader );
      return 0;
   }

   // Create the program object
   programObject = glCreateProgram ( );

   if ( programObject == 0 )
   {
      return 0;
   }

   glAttachShader ( programObject, vertexShader );
   glAttachShader ( programObject, tessControlShader );
   glAttachShader ( programObject, tessEvaluationShader );
   glAttachShader ( programObject, fragmentShader );

   // Free up no longer needed shader resources
   glDeleteShader ( vertexShader );
   glDeleteShader ( tessControlShader );
   glDeleteShader ( tessEvaluationShader );
   glDeleteShader ( fragmentShader );

   return programObject;
}
