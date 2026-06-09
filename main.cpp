/************************************************************************/
/*                                                                      */
/* (c) J. Fabrizio                                                      */
/*                                                                      */
/*                                                                      */
/************************************************************************/

#include <GL/glew.h>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>

#include "GL/freeglut.h"
#include "image.hh"
#include "image_io.hh"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glut.h"
#include "imgui/imgui_impl_opengl3.h"
#include "planet.hh"

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
GLuint vbo_id;
GLsizei planet_index_count = 0;
Planet myPlanet;

std::vector<Planet> planets;
struct PlanetInstance
{
    GLuint vao_id;
    GLuint vbo_id;
    GLuint ebo_id;
    GLsizei index_count;
};
std::vector<PlanetInstance> planetInstances;

glm::vec3 cameraPos = glm::vec3(0.0f, 15.0f, 55.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, -0.25f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float cameraSpeed = 0.2f;

float yaw = -90.0f;
float pitch = 0.0f;
float lastMouseX = 512.0f;
float lastMouseY = 512.0f;
bool firstMouse = true;
float mouseSensitivity = 0.1f;

float g_time = 0.0f;

int g_window_width = 1024;
int g_window_height = 1024;

GLuint sky_program_id;
GLuint sky_vao_id;
GLuint sky_vbo_id;

void window_resize(int width, int height)
{
    g_window_width = width;
    g_window_height = height;
    glViewport(0, 0, width, height);
    TEST_OPENGL_ERROR();
}

#if defined(SAVE_RENDER)
bool saved = false;
#endif

void keyboard(unsigned char key, int x, int y)
{
    ImGui_ImplGLUT_KeyboardFunc(key, x, y);

    ImGuiIO &io = ImGui::GetIO();
    if (io.WantCaptureKeyboard)
        return;

    glm::vec3 right = glm::normalize(glm::cross(cameraFront, cameraUp));

    switch (key)
    {
    case 'w':
        cameraPos += cameraSpeed * cameraFront;
        break;
    case 's':
        cameraPos -= cameraSpeed * cameraFront;
        break;
    case 'a':
        cameraPos -= cameraSpeed * right;
        break;
    case 'd':
        cameraPos += cameraSpeed * right;
        break;
    case 'e':
        cameraPos += cameraSpeed * cameraUp;
        break;
    case 'q':
        cameraPos -= cameraSpeed * cameraUp;
        break;
    }
    glutPostRedisplay();
}

void update_camera()
{
    glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    float aspect = (g_window_height > 0)
        ? (float)g_window_width / (float)g_window_height
        : 1.0f;
    glm::mat4 proj =
        glm::perspective(glm::radians(45.0f), aspect, 0.1f, 1000.0f);

    GLint mv_loc = glGetUniformLocation(program_id, "model_view_matrix");
    GLint pr_loc = glGetUniformLocation(program_id, "projection_matrix");
    GLint time_loc = glGetUniformLocation(program_id, "u_time");
    GLint cam_loc = glGetUniformLocation(program_id, "u_cameraPos");

    glUniformMatrix4fv(mv_loc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(pr_loc, 1, GL_FALSE, glm::value_ptr(proj));
    glUniform1f(time_loc, g_time);
    glUniform3fv(cam_loc, 1, glm::value_ptr(cameraPos));

    for (int i = 1; i < (int)planets.size(); i++)
    {
        std::string pos_name =
            "u_reflPlanets[" + std::to_string(i - 1) + "].center";
        std::string rad_name =
            "u_reflPlanets[" + std::to_string(i - 1) + "].radius";
        GLint ploc = glGetUniformLocation(program_id, pos_name.c_str());
        GLint rloc = glGetUniformLocation(program_id, rad_name.c_str());
        glUniform3fv(ploc, 1, glm::value_ptr(planets[i].position));
        glUniform1f(rloc, planets[i].shapeGenerator.planetRadius);
    }
}

void mousemotion(int x, int y)
{
    ImGuiIO &io = ImGui::GetIO();
    ImGui_ImplGLUT_MotionFunc(x, y);
    if (io.WantCaptureMouse)
    {
        firstMouse = true;
        return;
    }
    if (firstMouse)
    {
        lastMouseX = x;
        lastMouseY = y;
        firstMouse = false;
        return;
    }
    float offsetX = (x - lastMouseX) * mouseSensitivity;
    float offsetY = (lastMouseY - y) * mouseSensitivity;
    lastMouseX = x;
    lastMouseY = y;
    yaw += offsetX;
    pitch += offsetY;
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;
    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
    glutPostRedisplay();
}

void mouse_button(int button, int state, int x, int y)
{
    ImGui_ImplGLUT_MouseFunc(button, state, x, y);

    ImGuiIO &io = ImGui::GetIO();
    if (io.WantCaptureMouse)
        return;

    if (button == GLUT_LEFT_BUTTON)
    {
        if (state == GLUT_DOWN)
        {
            lastMouseX = x;
            lastMouseY = y;
            firstMouse = false;
        }
        else
        {
            firstMouse = true;
        }
    }
}

void display()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGLUT_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Parametres de Generation");
    bool changed = false;
    NoiseFilter &nf = planets[0].shapeGenerator.noiseFilter;
    changed |= ImGui::SliderFloat("Strength", &nf.strength, 0.0f, 2.0f);
    changed |= ImGui::SliderInt("Num Layers", &nf.numLayers, 1, 10);
    changed |=
        ImGui::SliderFloat("Base Roughness", &nf.baseRoughness, 0.1f, 5.0f);
    changed |= ImGui::SliderFloat("Roughness", &nf.roughness, 0.1f, 5.0f);
    changed |= ImGui::SliderFloat("Persistence", &nf.persistence, 0.1f, 1.0f);
    changed |= ImGui::SliderFloat("Min Value", &nf.minValue, 0.0f, 2.0f);
    changed |=
        ImGui::SliderFloat("Weight Mult", &nf.weightMultiplier, 0.0f, 3.0f);
    changed |= ImGui::SliderFloat("Taille Biomes (Freq)",
                                  &planets[0].shapeGenerator.biomeFrequency,
                                  0.01f, 8.0f);
    changed |=
        ImGui::SliderFloat3("Center (Offset)", &nf.center.x, -5.0f, 5.0f);

    ImGui::End();
    if (changed)
    {
        for (int i = 0; i < (int)planets.size() - 1; i++)
        {
            planets[i].shapeGenerator.noiseFilter = nf;
            planets[i].shapeGenerator.biomeFrequency =
                planets[0].shapeGenerator.biomeFrequency;
            planets[i].GeneratePlanet();
            planetInstances[i].index_count = planets[i].planet_indices.size();

            glBindVertexArray(planetInstances[i].vao_id);
            glBindBuffer(GL_ARRAY_BUFFER, planetInstances[i].vbo_id);
            glBufferData(GL_ARRAY_BUFFER,
                         planets[i].planet_vertices.size() * sizeof(GLfloat),
                         planets[i].planet_vertices.data(), GL_DYNAMIC_DRAW);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, planetInstances[i].ebo_id);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                         planets[i].planet_indices.size() * sizeof(GLuint),
                         planets[i].planet_indices.data(), GL_STATIC_DRAW);
            glBindVertexArray(0);
        }

        glUseProgram(program_id);
        GLint min_loc = glGetUniformLocation(program_id, "minElevation");
        GLint max_loc = glGetUniformLocation(program_id, "maxElevation");
        GLint radius_loc = glGetUniformLocation(program_id, "planetRadius");
        glUniform1f(min_loc, planets[0].shapeGenerator.elevationMinMax.Min);
        glUniform1f(max_loc, planets[0].shapeGenerator.elevationMinMax.Max);
        glUniform1f(radius_loc, planets[0].shapeGenerator.planetRadius);
    }

    g_time += 0.006f;

    g_window_width = glutGet(GLUT_WINDOW_WIDTH);
    g_window_height = glutGet(GLUT_WINDOW_HEIGHT);
    glViewport(0, 0, g_window_width, g_window_height);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    TEST_OPENGL_ERROR();

    glDepthMask(GL_FALSE);
    glUseProgram(sky_program_id);

    glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    float aspect = (g_window_height > 0)
        ? (float)g_window_width / (float)g_window_height
        : 1.0f;
    glm::mat4 proj =
        glm::perspective(glm::radians(45.0f), aspect, 0.1f, 1000.0f);
    glm::mat4 inv_vp = glm::inverse(proj * view);

    GLint ivp_loc = glGetUniformLocation(sky_program_id, "inv_view_proj");
    GLint time_loc = glGetUniformLocation(sky_program_id, "u_time");
    glUniformMatrix4fv(ivp_loc, 1, GL_FALSE, glm::value_ptr(inv_vp));
    glUniform1f(time_loc, g_time);

    glBindVertexArray(sky_vao_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    glDepthMask(GL_TRUE);

    glUseProgram(program_id);

    GLint cam_loc = glGetUniformLocation(program_id, "u_cameraPos");
    GLint time_loc2 = glGetUniformLocation(program_id, "u_time");
    glUniform3fv(cam_loc, 1, glm::value_ptr(cameraPos));
    glUniform1f(time_loc2, g_time);

    for (int i = 0; i < (int)planets.size(); i++)
    {
        glm::mat4 view =
            glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        float aspect = (g_window_height > 0)
            ? (float)g_window_width / (float)g_window_height
            : 1.0f;
        glm::mat4 proj =
            glm::perspective(glm::radians(45.0f), aspect, 0.1f, 1000.0f);
        glm::mat4 model = glm::translate(glm::mat4(1.0f), planets[i].position);
        glm::mat4 mv = view * model;

        GLint mv_loc = glGetUniformLocation(program_id, "model_view_matrix");
        GLint pr_loc = glGetUniformLocation(program_id, "projection_matrix");
        GLint model_loc = glGetUniformLocation(program_id, "model_matrix");
        GLint min_loc = glGetUniformLocation(program_id, "minElevation");
        GLint max_loc = glGetUniformLocation(program_id, "maxElevation");
        GLint radius_loc = glGetUniformLocation(program_id, "planetRadius");

        glUniformMatrix4fv(mv_loc, 1, GL_FALSE, glm::value_ptr(mv));
        glUniformMatrix4fv(pr_loc, 1, GL_FALSE, glm::value_ptr(proj));
        glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(model));
        glUniform1f(min_loc, planets[i].shapeGenerator.elevationMinMax.Min);
        glUniform1f(max_loc, planets[i].shapeGenerator.elevationMinMax.Max);
        glUniform1f(radius_loc, planets[i].shapeGenerator.planetRadius);

        int reflIdx = 0;
        for (int j = 0; j < (int)planets.size(); j++)
        {
            if (j == i)
                continue;
            std::string pc =
                "u_reflPlanets[" + std::to_string(reflIdx) + "].center";
            std::string pr =
                "u_reflPlanets[" + std::to_string(reflIdx) + "].radius";
            GLint ploc = glGetUniformLocation(program_id, pc.c_str());
            GLint rloc = glGetUniformLocation(program_id, pr.c_str());
            glUniform3fv(ploc, 1, glm::value_ptr(planets[j].position));
            glUniform1f(rloc, planets[j].shapeGenerator.planetRadius);
            reflIdx++;
        }

        int sunPlanetGlobalIdx = (int)planets.size() - 1;
        int sunReflIdx = -1;
        int tmpIdx2 = 0;
        for (int j = 0; j < (int)planets.size(); j++)
        {
            if (j == i)
                continue;
            if (j == sunPlanetGlobalIdx)
            {
                sunReflIdx = tmpIdx2;
                break;
            }
            tmpIdx2++;
        }
        GLint sunrefl_loc = glGetUniformLocation(program_id, "u_reflSunIndex");
        glUniform1i(sunrefl_loc, sunReflIdx);

        GLint nrefl_loc = glGetUniformLocation(program_id, "u_numReflPlanets");
        GLint cc_loc =
            glGetUniformLocation(program_id, "u_currentPlanetCenter");
        GLint cr_loc =
            glGetUniformLocation(program_id, "u_currentPlanetRadiusMax");
        GLint sun_loc = glGetUniformLocation(program_id, "u_isSun");
        glUniform1i(sun_loc, (i == (int)planets.size() - 1) ? 1 : 0);
        glUniform3fv(cc_loc, 1, glm::value_ptr(planets[i].position));
        glUniform1f(cr_loc, planets[i].shapeGenerator.elevationMinMax.Max);
        glUniform1i(nrefl_loc, reflIdx);

        glBindVertexArray(planetInstances[i].vao_id);
        glDrawElements(GL_TRIANGLES, planetInstances[i].index_count,
                       GL_UNSIGNED_INT, (void *)0);
        glBindVertexArray(0);
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

#if defined(SAVE_RENDER)
    if (!saved)
    {
        tifo::rgb24_image *texture = new tifo::rgb24_image(800, 590);
        glReadPixels(150, 350, 800, 590, GL_RGB, GL_UNSIGNED_BYTE,
                     texture->pixels);
        TEST_OPENGL_ERROR();
        tifo::save_image(*texture, "render.tga");
        std::cout << "Save " << std::endl;
        delete texture;
    }
#endif
    glutSwapBuffers();
    glutPostRedisplay();
}

void init_glut(int &argc, char *argv[])
{
    glutInit(&argc, argv);
    glutInitContextProfile(GLUT_CORE_PROFILE | GLUT_DEBUG);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(1024, 1024);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Shader Programming");
    glutDisplayFunc(display);
    glutReshapeFunc(window_resize);
    glutKeyboardFunc(keyboard);
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

void init_imgui()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();

    ImGui_ImplGLUT_Init();
    ImGui_ImplOpenGL3_Init("#version 450");
}

void init_planet_instance(Planet &planet, PlanetInstance &inst)
{
    planet.GeneratePlanet();
    inst.index_count = planet.planet_indices.size();

    GLint pos_loc = glGetAttribLocation(program_id, "position");
    GLint biome_loc = glGetAttribLocation(program_id, "biome");
    GLint unclamped_loc = glGetAttribLocation(program_id, "unclampedRadius");

    glGenVertexArrays(1, &inst.vao_id);
    glBindVertexArray(inst.vao_id);

    glGenBuffers(1, &inst.vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, inst.vbo_id);
    glBufferData(GL_ARRAY_BUFFER,
                 planet.planet_vertices.size() * sizeof(GLfloat),
                 planet.planet_vertices.data(), GL_STATIC_DRAW);

    if (pos_loc != -1)
    {
        glVertexAttribPointer(pos_loc, 3, GL_FLOAT, GL_FALSE,
                              5 * sizeof(GLfloat), 0);
        glEnableVertexAttribArray(pos_loc);
    }
    if (biome_loc != -1)
    {
        glVertexAttribPointer(biome_loc, 1, GL_FLOAT, GL_FALSE,
                              5 * sizeof(GLfloat),
                              (void *)(3 * sizeof(GLfloat)));
        glEnableVertexAttribArray(biome_loc);
    }
    if (unclamped_loc != -1)
    {
        glVertexAttribPointer(unclamped_loc, 1, GL_FLOAT, GL_FALSE,
                              5 * sizeof(GLfloat),
                              (void *)(4 * sizeof(GLfloat)));
        glEnableVertexAttribArray(unclamped_loc);
    }

    glGenBuffers(1, &inst.ebo_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, inst.ebo_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 planet.planet_indices.size() * sizeof(GLuint),
                 planet.planet_indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
}

void init_object_vbo()
{
    Planet p0;
    p0.resolution = 256;
    p0.position = glm::vec3(22.0f, 0.0f, 0.0f);
    planets.push_back(p0);

    Planet p1;
    p1.resolution = 128;
    p1.position = glm::vec3(-18.0f, 4.0f, 12.0f);
    planets.push_back(p1);

    Planet p2;
    p2.resolution = 128;
    p2.position = glm::vec3(5.0f, -8.0f, -28.0f);
    planets.push_back(p2);

    Planet sun;
    sun.resolution = 64;
    sun.position = glm::vec3(0.0f, 0.0f, 0.0f);
    sun.shapeGenerator.planetRadius = 8.0f;
    sun.shapeGenerator.noiseFilter.strength = 0.0f;
    sun.shapeGenerator.noiseFilter.minValue = 999.0f;
    planets.push_back(sun);

    for (auto &p : planets)
    {
        PlanetInstance inst;
        init_planet_instance(p, inst);
        planetInstances.push_back(inst);
    }

    glUseProgram(program_id);
    GLint min_loc = glGetUniformLocation(program_id, "minElevation");
    GLint max_loc = glGetUniformLocation(program_id, "maxElevation");
    GLint radius_loc = glGetUniformLocation(program_id, "planetRadius");
    glUniform1f(min_loc, planets[0].shapeGenerator.elevationMinMax.Min);
    glUniform1f(max_loc, planets[0].shapeGenerator.elevationMinMax.Max);
    glUniform1f(radius_loc, planets[0].shapeGenerator.planetRadius);
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
        shader_log = (char *)std::malloc(log_size + 1);
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
        program_log = (char *)std::malloc(log_size + 1);
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

bool init_sky_shader()
{
    GLuint vert_id, frag_id;
    if (!load_and_compile_shader(GL_VERTEX_SHADER, "sky_vertex.glsl", vert_id))
        return false;
    if (!load_and_compile_shader(GL_FRAGMENT_SHADER, "sky_fragment.glsl",
                                 frag_id))
        return false;

    std::vector<GLuint> ids = { vert_id, frag_id };
    if (!attach_and_link_program(ids, sky_program_id))
        return false;

    for (auto id : ids)
    {
        glDetachShader(sky_program_id, id);
        glDeleteShader(id);
    }
    return true;
}

void init_sky_vao()
{
    float verts[] = { -1, -1, 1, -1, 1, 1, -1, -1, 1, 1, -1, 1 };

    glGenVertexArrays(1, &sky_vao_id);
    glBindVertexArray(sky_vao_id);
    glGenBuffers(1, &sky_vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, sky_vbo_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

    GLint pos_loc = glGetAttribLocation(sky_program_id, "position");
    glVertexAttribPointer(pos_loc, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(pos_loc);
    glBindVertexArray(0);
}

int main(int argc, char *argv[])
{
    init_glut(argc, argv);
    if (!init_glew())
        std::exit(-1);
    init_GL();
    init_shaders();
    init_sky_shader();
    init_object_vbo();
    init_sky_vao();
    init_textures();
    init_imgui();
    ImGui_ImplGLUT_InstallFuncs();
    glutKeyboardFunc(keyboard);
    glutMotionFunc(mousemotion);
    glutPassiveMotionFunc(ImGui_ImplGLUT_MotionFunc);
    glutMouseFunc(mouse_button);
    glutMainLoop();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGLUT_Shutdown();
    ImGui::DestroyContext();
}
