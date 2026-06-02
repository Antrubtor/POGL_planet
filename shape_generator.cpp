#include "shape_generator.hh"

PointData ShapeGenerator::CalculatePointOnPlanet(glm::vec3 pointOnUnitSphere)
{
    float elevation = noiseFilter.Evaluate(pointOnUnitSphere);
    float unclampedRadius = planetRadius * (1.0f + elevation);
    float clampedRadius = planetRadius * (1.0f + std::max(0.0f, elevation));
    elevationMinMax.AddValue(unclampedRadius);
    return { pointOnUnitSphere * clampedRadius, unclampedRadius };
}

float ShapeGenerator::CalculateBiome(glm::vec3 pointOnUnitSphere)
{
    float noiseValue =
        biomeNoise.GetNoise(pointOnUnitSphere.x * biomeFrequency * 100.0f,
                            pointOnUnitSphere.y * biomeFrequency * 100.0f,
                            pointOnUnitSphere.z * biomeFrequency * 100.0f);
    return (noiseValue + 1.0f) * 0.5f;
}
