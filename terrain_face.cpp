#include "terrain_face.hh"

TerrainFace::TerrainFace(int resolution, glm::vec3 localUp,
                         ShapeGenerator *shapeGenerator)
    : resolution(resolution)
    , localUp(localUp)
    , axisA(glm::vec3(localUp.y, localUp.z, localUp.x))
    , axisB(glm::cross(localUp, axisA))
    , shapeGenerator(shapeGenerator)
{}

void TerrainFace::ConstructMesh()
{
    vertices.clear();
    indices.clear();

    for (size_t y = 0; y < resolution; y++)
    {
        for (size_t x = 0; x < resolution; x++)
        {
            size_t i = x + y * resolution;
            glm::vec2 percent = glm::vec2(x, y) / (float)(resolution - 1);
            glm::vec3 pointOnUnitCube = localUp + (percent.x - 0.5f) * 2 * axisA
                + (percent.y - 0.5f) * 2 * axisB;
            glm::vec3 pointOnUnitSphere = glm::normalize(pointOnUnitCube);

            glm::vec3 pointOnPlanet =
                shapeGenerator->CalculatePointOnPlanet(pointOnUnitSphere);
            vertices.push_back(pointOnPlanet);

            if (x != resolution - 1 && y != resolution - 1)
            {
                indices.push_back(i);
                indices.push_back(i + resolution + 1);
                indices.push_back(i + resolution);

                indices.push_back(i);
                indices.push_back(i + 1);
                indices.push_back(i + resolution + 1);
            }
        }
    }
}
