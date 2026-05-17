#include "noise_filter.hh"

class MinMax
{
public:
    float Min = std::numeric_limits<float>::max();
    float Max = std::numeric_limits<float>::lowest();

    void AddValue(float v)
    {
        if (v < Min)
            Min = v;
        if (v > Max)
            Max = v;
    }
};

class ShapeGenerator
{
public:
    float planetRadius = 3.0f;
    NoiseFilter noiseFilter;
    MinMax elevationMinMax;

    FastNoiseLite biomeNoise;
    float biomeFrequency = 0.3f;

    ShapeGenerator()
    {
        biomeNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    }

    glm::vec3 CalculatePointOnPlanet(glm::vec3 pointOnUnitSphere);
    float CalculateBiome(glm::vec3 pointOnUnitSphere);
};
