/************************************************************************/
/*                                                                      */
/* (c) J. Fabrizio                                                      */
/*                                                                      */
/*                                                                      */
/************************************************************************/

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <fstream>
#include <glm/glm.hpp>
#include <iostream>
#include <vector>

//#include <glt_transform.hh>

#include "image.hh"
#include "image_io.hh"
/*#include "object_vbo.hh"*/
#include "planet.hh"

//#define SAVE_RENDEflat R

#define TEST_OPENGL_ERROR()                                                    \
    do                                                                         \
    {                                                                          \
        GLenum err = glGetError();                                             \
        if (err != GL_NO_ERROR)                                                \
            std::cerr << "OpenGL ERROR!" << __LINE__ << std::endl;             \
    } while (0)

GLuint planet_vao_id;
GLuint program_id;
GLuint ebo_id;
GLsizei planet_index_count = 0;

void window_resize(int width, int height)
{
    // std::cout << "glViewport(0,0,"<< width << "," << height <<
    // ");TEST_OPENGL_ERROR();" << std::endl;
    glViewport(0, 0, width, height);
    TEST_OPENGL_ERROR();
}

#if defined(SAVE_RENDER)
bool saved = false;
#endif

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    TEST_OPENGL_ERROR();
    glBindVertexArray(planet_vao_id);
    TEST_OPENGL_ERROR();
    glDrawElements(GL_TRIANGLES, planet_index_count, GL_UNSIGNED_INT,
                   (void *)0);
    TEST_OPENGL_ERROR();
    glBindVertexArray(0);
    TEST_OPENGL_ERROR();
#if defined(SAVE_RENDER)
    if (!saved)
    {
        tifo::rgb24_image *texture = new tifo::rgb24_image(800, 590);
        glReadPixels(150, 350, 800, 590, GL_RGB, GL_UNSIGNED_BYTE,
                     texture->pixels);
        TEST_OPENGL_ERROR();
        // glReadPixels(0, 0, 1024, 1024, GL_RGB, GL_UNSIGNED_BYTE,
        // texture->pixels);
        tifo::save_image(*texture, "render.tga");
        std::cout << "Save " << std::endl;
        delete texture;
        // saved = true;
    }
#endif
    glutSwapBuffers();
}

void init_glut(int &argc, char *argv[])
{
    // glewExperimental = GL_TRUE;
    glutInit(&argc, argv);
    /*glutInitContextVersion(4,5);*/
    glutInitContextProfile(GLUT_CORE_PROFILE | GLUT_DEBUG);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(1024, 1024);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Shader Programming");
    glutDisplayFunc(display);
    glutReshapeFunc(window_resize);
}

bool init_glew()
{
    if (glewInit())
    {
        std::cerr << " Error while initializing glew";
        return false;
    }
    return true;
}

void init_GL()
{
    glEnable(GL_DEPTH_TEST);
    TEST_OPENGL_ERROR();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    TEST_OPENGL_ERROR();
    glEnable(GL_CULL_FACE);
    TEST_OPENGL_ERROR();
    glClearColor(0.4, 0.4, 0.4, 1.0);
    TEST_OPENGL_ERROR();
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
}

void init_object_vbo()
{
    Planet myPlanet;
    myPlanet.resolution = 10;
    myPlanet.GeneratePlanet();
    planet_index_count = myPlanet.planet_indices.size();

    GLint vertex_location = glGetAttribLocation(program_id, "position");
    TEST_OPENGL_ERROR();
    GLint normal_smooth_location =
        glGetAttribLocation(program_id, "normalSmooth");
    TEST_OPENGL_ERROR();

    glGenVertexArrays(1, &planet_vao_id);
    TEST_OPENGL_ERROR();
    glBindVertexArray(planet_vao_id);
    TEST_OPENGL_ERROR();

    std::cout << "vertex_location " << vertex_location
              << ((vertex_location == -1) ? " NOT FOUND!" : " OK") << std::endl;
    std::cout << "normal_smooth_location " << normal_smooth_location
              << ((normal_smooth_location == -1) ? " NOT FOUND!" : " OK")
              << std::endl;

    GLuint vbo_id;
    glGenBuffers(1, &vbo_id);
    TEST_OPENGL_ERROR();
    glGenBuffers(1, &ebo_id);
    TEST_OPENGL_ERROR();

    glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
    TEST_OPENGL_ERROR();
    glBufferData(GL_ARRAY_BUFFER,
                 myPlanet.planet_vertices.size() * sizeof(GLfloat),
                 myPlanet.planet_vertices.data(), GL_STATIC_DRAW);
    TEST_OPENGL_ERROR();

    if (vertex_location != -1)
    {
        glVertexAttribPointer(vertex_location, 3, GL_FLOAT, GL_FALSE, 0, 0);
        TEST_OPENGL_ERROR();
        glEnableVertexAttribArray(vertex_location);
        TEST_OPENGL_ERROR();
    }

    if (normal_smooth_location != -1)
    {
        glVertexAttribPointer(normal_smooth_location, 3, GL_FLOAT, GL_FALSE, 0,
                              0);
        TEST_OPENGL_ERROR();
        glEnableVertexAttribArray(normal_smooth_location);
        TEST_OPENGL_ERROR();
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_id);
    TEST_OPENGL_ERROR();
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 myPlanet.planet_indices.size() * sizeof(GLuint),
                 myPlanet.planet_indices.data(), GL_STATIC_DRAW);
    TEST_OPENGL_ERROR();
    glBindVertexArray(0);
}

void init_textures()
{
    tifo::rgb24_image *texture = tifo::load_image("texture.tga");
    tifo::rgb24_image *lighting = tifo::load_image("lighting.tga");
    tifo::rgb24_image *normalmap = tifo::load_image("normalmap.tga");
    GLuint texture_id;
    GLuint lighting_id;
    GLuint normalmap_id;
    GLint tex_location;
    GLint light_location;
    GLint normalmap_location;

    std::cout << "texture size " << texture->sx << " ," << texture->sy << "\n";
    std::cout << "light texture size " << lighting->sx << " ," << lighting->sy
              << "\n";
    std::cout << "normalmap size " << normalmap->sx << " ," << normalmap->sy
              << std::endl;

    GLint texture_units, combined_texture_units;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &texture_units);
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &combined_texture_units);
    std::cout << "Limit 1 " << texture_units << " limit 2 "
              << combined_texture_units << std::endl;

    glGenTextures(1, &texture_id);
    TEST_OPENGL_ERROR();
    glActiveTexture(GL_TEXTURE0);
    TEST_OPENGL_ERROR();
    glBindTexture(GL_TEXTURE_2D, texture_id);
    TEST_OPENGL_ERROR();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture->sx, texture->sy, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, texture->pixels);
    TEST_OPENGL_ERROR();
    tex_location = glGetUniformLocation(program_id, "texture_sampler");
    TEST_OPENGL_ERROR();
    std::cout << "texture sampler location " << tex_location
              << ((tex_location == -1) ? " NOT FOUND!" : " OK") << std::endl;
    glUniform1i(tex_location, 0);
    TEST_OPENGL_ERROR();

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    TEST_OPENGL_ERROR();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    TEST_OPENGL_ERROR();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    TEST_OPENGL_ERROR();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    TEST_OPENGL_ERROR();

    glGenTextures(1, &lighting_id);
    TEST_OPENGL_ERROR();
    glActiveTexture(GL_TEXTURE1);
    TEST_OPENGL_ERROR();
    glBindTexture(GL_TEXTURE_2D, lighting_id);
    TEST_OPENGL_ERROR();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, lighting->sx, lighting->sy, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, lighting->pixels);
    TEST_OPENGL_ERROR();
    light_location = glGetUniformLocation(program_id, "lighting_sampler");
    TEST_OPENGL_ERROR();
    std::cout << "lighting sampler location " << light_location
              << ((light_location == -1) ? " NOT FOUND!" : " OK") << std::endl;
    glUniform1i(light_location, 1);
    TEST_OPENGL_ERROR();

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    TEST_OPENGL_ERROR();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    TEST_OPENGL_ERROR();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    TEST_OPENGL_ERROR();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    TEST_OPENGL_ERROR();

    glGenTextures(1, &normalmap_id);
    TEST_OPENGL_ERROR();
    glActiveTexture(GL_TEXTURE2);
    TEST_OPENGL_ERROR();
    glBindTexture(GL_TEXTURE_2D, normalmap_id);
    TEST_OPENGL_ERROR();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, normalmap->sx, normalmap->sy, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, normalmap->pixels);
    TEST_OPENGL_ERROR();
    normalmap_location = glGetUniformLocation(program_id, "normalmap_sampler");
    TEST_OPENGL_ERROR();
    std::cout << "normalmap location " << normalmap_location
              << ((normalmap_location == -1) ? " NOT FOUND!" : " OK")
              << std::endl;
    glUniform1i(normalmap_location, 2);
    TEST_OPENGL_ERROR();

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    TEST_OPENGL_ERROR();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    TEST_OPENGL_ERROR();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    TEST_OPENGL_ERROR();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    TEST_OPENGL_ERROR();

    delete texture;
    delete lighting;
    delete normalmap;
}

std::string load(const std::string &filename)
{
    std::ifstream input_src_file(filename, std::ios::in);
    std::string ligne;
    std::string file_content = "";
    if (input_src_file.fail())
    {
        std::cerr << "FAILURE: can not load " << filename << "\n";
        return "";
    }
    while (getline(input_src_file, ligne))
    {
        file_content = file_content + ligne + "\n";
    }
    file_content += '\0';
    input_src_file.close();
    return file_content;
}

bool load_and_compile_shader(const GLenum shader_type,
                             const std::string shader_src_filename,
                             GLuint &shader_id)
{
    GLint compile_status = GL_TRUE;
    std::string shader_src = load(shader_src_filename);
    const GLchar *sources[1];
    sources[0] = shader_src.c_str();
    shader_id = glCreateShader(shader_type);
    TEST_OPENGL_ERROR();
    glShaderSource(shader_id, 1, sources, 0);
    TEST_OPENGL_ERROR();
    glCompileShader(shader_id);
    TEST_OPENGL_ERROR();
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compile_status);
    if (compile_status != GL_TRUE)
    {
        GLint log_size;
        char *shader_log;
        glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &log_size);
        shader_log = (char *)std::malloc(
            log_size + 1); /* +1 pour le caractere de fin de chaine '\0' */
        if (shader_log != 0)
        {
            glGetShaderInfoLog(shader_id, log_size, &log_size, shader_log);
            std::cerr << "FAILURE can not compile shader "
                      << shader_src_filename << ": " << shader_log << std::endl;
            std::free(shader_log);
        }
        glDeleteShader(shader_id);
        return false;
    }
    return true;
}

bool attach_and_link_program(const std::vector<GLuint> &shaders_id,
                             GLuint &program_id)
{
    GLint link_status = GL_TRUE;
    program_id = glCreateProgram();
    TEST_OPENGL_ERROR();
    if (program_id == 0)
        return false;
    for (unsigned int i = 0; i < shaders_id.size(); i++)
    {
        glAttachShader(program_id, shaders_id[i]);
        TEST_OPENGL_ERROR();
    }
    glLinkProgram(program_id);
    TEST_OPENGL_ERROR();
    glGetProgramiv(program_id, GL_LINK_STATUS, &link_status);
    if (link_status != GL_TRUE)
    {
        GLint log_size;
        char *program_log;
        glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &log_size);
        program_log = (char *)std::malloc(
            log_size + 1); /* +1 pour le caractere de fin de chaine '\0' */
        if (program_log != 0)
        {
            glGetProgramInfoLog(program_id, log_size, &log_size, program_log);
            std::cerr << "FAILURE: Program can not be linked " << program_log
                      << std::endl;
            std::free(program_log);
        }
        for (unsigned int i = 0; i < shaders_id.size(); i++)
        {
            glDetachShader(program_id, shaders_id[i]);
            TEST_OPENGL_ERROR();
        }
        glDeleteProgram(program_id);
        TEST_OPENGL_ERROR();
        program_id = 0;
        return false;
    }
    // glUseProgram(program_id);TEST_OPENGL_ERROR();
    return true;
}

bool init_shaders()
{
    GLuint vertex_shader_id, fragment_shader_id;
    if (!load_and_compile_shader(GL_VERTEX_SHADER, "vertex.glsl",
                                 vertex_shader_id))
    {
        return false;
    }
    if (!load_and_compile_shader(GL_FRAGMENT_SHADER, "fragment.glsl",
                                 fragment_shader_id))
    {
        return false;
    }
    std::vector<GLuint> shaders_id;
    shaders_id.push_back(vertex_shader_id);
    shaders_id.push_back(fragment_shader_id);
    if (!attach_and_link_program(shaders_id, program_id))
    {
        for (unsigned int i = 0; i < shaders_id.size(); i++)
        {
            glDeleteShader(shaders_id[i]);
            TEST_OPENGL_ERROR();
        }
        return false;
    }

    for (unsigned int i = 0; i < shaders_id.size(); i++)
    {
        glDetachShader(program_id, shaders_id[i]);
        TEST_OPENGL_ERROR();
    }

    for (unsigned int i = 0; i < shaders_id.size(); i++)
    {
        glDeleteShader(shaders_id[i]);
        TEST_OPENGL_ERROR();
    }
    glUseProgram(program_id);
    return true;
}

/*void tmp() {
  glt::matrix4 look = glt::matrix4::identity();
  glt::matrix4 frustum = glt::matrix4::identity();

  glt::frustum(frustum,
           -1, 1, -1, 1,
           5, 50000
         );

  glt::look_at(look,
           20, 20, 20,
           0, 0, 0,
           0, 1, 0
           );

  std::cout << "Look\n" << look << "\n";
  std::cout << "frustum\n" << frustum << std::endl;
  }*/

int main(int argc, char *argv[])
{
    //  tmp();
    init_glut(argc, argv);
    if (!init_glew())
        std::exit(-1);
    init_GL();
    init_shaders();
    init_object_vbo();
    init_textures();
    glutMainLoop();
}
