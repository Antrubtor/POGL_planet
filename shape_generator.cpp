#include "shape_generator.hh"

glm::vec3 ShapeGenerator::CalculatePointOnPlanet(glm::vec3 pointOnUnitSphere)
{
    float elevation = noiseFilter.Evaluate(pointOnUnitSphere);
    float currentRadius = planetRadius * (1.0f + elevation);
    elevationMinMax.AddValue(currentRadius);
    return pointOnUnitSphere * currentRadius;
}
