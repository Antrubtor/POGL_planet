#include "noise_filter.hh"
#include <algorithm>

NoiseFilter::NoiseFilter() {
        noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    }

float NoiseFilter::Evaluate(glm::vec3 point) {
        float noiseValue = 0.0f;
        float frequency = baseRoughness;
        float amplitude = 1.0f;

        for (int i = 0; i < numLayers; i++) {
            glm::vec3 p = (point + center) * frequency * 100.0f;
            float v = noise.GetNoise(p.x, p.y, p.z);
            v = (v + 1.0f) * 0.5f;
            noiseValue += v * amplitude;
            frequency *= roughness;
            amplitude *= persistence;
        }

        noiseValue = std::max(0.0f, noiseValue - minValue);
        return noiseValue * strength;
    }
