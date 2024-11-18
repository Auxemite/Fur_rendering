#include "SceneObject.h"

#include <glm/gtc/matrix_transform.hpp>

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

void SceneObject::render() const {
    if(!_material || !_mesh) {
        return;
    }

    _material->set_uniform(HASH("model"), transform());
    _material->bind();
    _mesh->draw();
}

void SceneObject::set_transform(const glm::mat4& tr) {
    _transform = tr;
}

bool SceneObject::is_visible(const Camera& camera) const
{
    // glm::vec4 trans_center = glm::vec3(_center, 1.0f) * _transform;
    glm::vec3 direction = glm::normalize(_center - camera.position());

    return glm::dot(camera.forward(), direction) > 0.5; // TODO fix with the frutsum to get the right HitBox
}

const glm::mat4& SceneObject::transform() const {
    return _transform;
}

}
