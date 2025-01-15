#include "SceneObject.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>
#include <iostream>

namespace OM3D {

SceneObject::SceneObject(std::shared_ptr<StaticMesh> mesh, std::shared_ptr<Material> material) :
    _mesh(std::move(mesh)),
    _material(std::move(material))
    {
        _center = glm::vec3(0.0f, 0.0f, 0.0f);
        Vertex* vertices = _mesh.get()->vertices();
        size_t vertices_count = _mesh.get()->vertices_count();

        for (size_t i = 0; i < vertices_count; i++)
            _center += vertices[i].position;
        
        _center /= static_cast<float>(vertices_count);
        for (size_t i = 0; i < vertices_count; i++)
        {
            const float center_dist = glm::length(vertices[i].position - _center);
            if (center_dist > _radius)
                _radius = center_dist;
        }
}

void SceneObject::render(const RenderMode& renderMode) const {
    if(!_material || !_mesh) {
        return;
    }

    _material->set_uniform(HASH("model"), transform());
    _material->bind(renderMode);
    _mesh->draw();
}

void SceneObject::set_transform(const glm::mat4& tr) {
    _transform = tr;
}

void SceneObject::set_center(const glm::vec3& center) {
    _center = center;
}

/*
struct Frustum {
    public:
    glm::vec3 _near_normal;
    // No far plane (zFar is +inf)
    glm::vec3 _top_normal;
    glm::vec3 _bottom_normal;
    glm::vec3 _right_normal;
    glm::vec3 _left_normal;
};
*/

static bool in_plane(glm::vec3 plane_normal, const glm::vec3& origin, const glm::vec3 center, const float radius)
{
    plane_normal = glm::normalize(plane_normal);
    glm::vec3 direction = center - origin;
    float d = glm::dot(plane_normal, direction);
    return d >= -radius;
}

void SceneObject::print_info() const {
    std::cout << "SceneObject: " << std::endl;
    std::cout << "Center: " << _center.x << ", " << _center.y << ", " << _center.z << std::endl;
    std::cout << "Radius: " << _radius << std::endl;
}

bool SceneObject::is_visible(const Camera& camera) const
{
    const glm::vec3 origin = camera.position();
    const Frustum frut = camera.build_frustum();

    //Get scale from transform diagonal
    // const glm::vec3 scale = glm::vec3(_transform[0][0], _transform[1][1], _transform[2][2]);

    //Get our center with transform offsets
    const glm::vec3 center = _center + glm::vec3(_transform[3][0], _transform[3][1], _transform[3][2]);

    // Get the max scale to update the radius of our bounding box
    // const float max_scale = std::max(std::max(scale.x, scale.y), scale.z);

    // Update radius, assuming that the scales correspond to diameter (imply *0.5f)
    const float radius = _radius;// * (max_scale * .5f);

    return (in_plane(frut._near_normal, origin, center, radius) &&
            in_plane(frut._left_normal, origin, center, radius) &&
            in_plane(frut._right_normal, origin, center, radius) &&
            in_plane(frut._top_normal, origin, center, radius) &&
            in_plane(frut._bottom_normal, origin, center, radius));
};

bool SceneObject::is_in_range(const glm::vec3& position, const float& radius) const
{
    return glm::length(position - _center) <= radius;
}

const glm::mat4& SceneObject::transform() const {
    return _transform;
}

}
