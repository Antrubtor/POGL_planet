#include "noise_filter.hh"

#include <algorithm>

NoiseFilter::NoiseFilter()
{
    noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
}

float NoiseFilter::Evaluate(glm::vec3 point)
{
    float noiseValue = 0.0f;
    float frequency = baseRoughness;
    float amplitude = 1.0f;

    float weight = 1.0f;

    for (int i = 0; i < numLayers; i++)
    {
        glm::vec3 p = (point + center) * frequency * 100.0f;
        float v = noise.GetNoise(p.x, p.y, p.z);
        v = 1.0f - std::abs(v);
        v *= v;
        v *= weight;
        weight = std::clamp(v * weightMultiplier, 0.0f, 1.0f);
        noiseValue += v * amplitude;
        frequency *= roughness;
        amplitude *= persistence;
    }

    noiseValue = noiseValue - minValue;
    return noiseValue * strength;
}
