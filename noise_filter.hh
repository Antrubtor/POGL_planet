#include <glm/glm.hpp>

#include "FastNoiseLite.hh"

class NoiseFilter
{
public:
    FastNoiseLite noise;

    float strength = 0.1f;
    int numLayers = 1;
    float baseRoughness = 2.0f;
    float roughness = 2.0f;
    float persistence = 0.5f;
    glm::vec3 center = glm::vec3(0.0f, 0.0f, 0.0f);
    float minValue = 0.1f;

    NoiseFilter();
    float Evaluate(glm::vec3 point);
};
