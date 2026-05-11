#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <vector>

#include "shape_generator.hh"

class TerrainFace
{
public:
    size_t resolution;
    glm::vec3 localUp;
    glm::vec3 axisA;
    glm::vec3 axisB;
    ShapeGenerator *shapeGenerator;

    std::vector<glm::vec3> vertices;
    std::vector<GLuint> indices;

    TerrainFace(int resolution, glm::vec3 localUp,
                ShapeGenerator *shapeGenerator);
    void ConstructMesh();
};
