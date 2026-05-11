#include "shape_generator.hh"


glm::vec3 ShapeGenerator::CalculatePointOnPlanet(glm::vec3 pointOnUnitSphere) {
        float elevation = noiseFilter.Evaluate(pointOnUnitSphere);
        return pointOnUnitSphere * planetRadius * (1.0f + elevation);
    }
