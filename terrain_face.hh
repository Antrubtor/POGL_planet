#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <vector>

class TerrainFace
{
public:
    size_t resolution;
    glm::vec3 localUp;
    glm::vec3 axisA;
    glm::vec3 axisB;

    std::vector<glm::vec3> vertices;
    std::vector<GLuint> indices;

    TerrainFace(int resolution, glm::vec3 localUp);
    void ConstructMesh();
};
