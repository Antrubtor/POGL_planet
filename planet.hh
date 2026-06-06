#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>

#include "terrain_face.hh"

class Planet
{
public:
    size_t resolution = 10;
    ShapeGenerator shapeGenerator;
    glm::vec3 position = glm::vec3(0.0f);

    std::vector<GLfloat> planet_vertices;
    std::vector<GLuint> planet_indices;

    void GeneratePlanet();
};
