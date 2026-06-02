#include "planet.hh"

void Planet::GeneratePlanet()
{
    planet_vertices.clear();
    planet_indices.clear();
    shapeGenerator.elevationMinMax.Reset();

    glm::vec3 directions[6] = { glm::vec3(0, 1, 0),  glm::vec3(0, -1, 0),
                                glm::vec3(-1, 0, 0), glm::vec3(1, 0, 0),
                                glm::vec3(0, 0, 1),  glm::vec3(0, 0, -1) };

    int vertexOffset = 0;

    for (size_t i = 0; i < 6; i++)
    {
        TerrainFace face(resolution, directions[i], &shapeGenerator);
        face.ConstructMesh();

        for (const auto &v : face.vertices)
        {
            planet_vertices.push_back(v.position.x);
            planet_vertices.push_back(v.position.y);
            planet_vertices.push_back(v.position.z);
            planet_vertices.push_back(v.biome);
            planet_vertices.push_back(v.unclampedRadius);
        }

        for (GLuint index : face.indices)
            planet_indices.push_back(index + vertexOffset);

        vertexOffset += face.vertices.size();
    }
}
