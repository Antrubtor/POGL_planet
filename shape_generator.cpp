#include "shape_generator.hh"

glm::vec3 ShapeGenerator::CalculatePointOnPlanet(glm::vec3 pointOnUnitSphere)
{
    float elevation = noiseFilter.Evaluate(pointOnUnitSphere);
    float currentRadius = planetRadius * (1.0f + elevation);
    elevationMinMax.AddValue(currentRadius);
    return pointOnUnitSphere * currentRadius;
}

float ShapeGenerator::CalculateBiome(glm::vec3 pointOnUnitSphere)
{
    float noiseValue =
        biomeNoise.GetNoise(pointOnUnitSphere.x * biomeFrequency * 100.0f,
                            pointOnUnitSphere.y * biomeFrequency * 100.0f,
                            pointOnUnitSphere.z * biomeFrequency * 100.0f);
    return (noiseValue + 1.0f) * 0.5f;
}
