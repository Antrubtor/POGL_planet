#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <vector>

#include "terrain_face.hh"

class Planet
{
public:
    size_t resolution = 10;
    ShapeGenerator shapeGenerator;

    std::vector<GLfloat> planet_vertices;
    std::vector<GLuint> planet_indices;

    void GeneratePlanet();
};
