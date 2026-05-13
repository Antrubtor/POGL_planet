#include <glm/glm.hpp>

#include "FastNoiseLite.h"

class NoiseFilter
{
public:
    FastNoiseLite noise;

    float strength = 0.10f;
    int numLayers = 6;
    float baseRoughness = 0.9f;
    float roughness = 2.0f;
    float persistence = 0.54f;
    glm::vec3 center = glm::vec3(0.0f, 0.0f, 0.0f);
    float minValue = 0.5;
    float weightMultiplier = 1.9f;

    NoiseFilter();
    float Evaluate(glm::vec3 point);
};
