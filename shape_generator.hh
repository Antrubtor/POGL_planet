#include "noise_filter.hh"
class ShapeGenerator
{
public:
    float planetRadius = 3.0f;
    NoiseFilter noiseFilter;

    glm::vec3 CalculatePointOnPlanet(glm::vec3 pointOnUnitSphere);
};
