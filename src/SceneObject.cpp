#include "SceneObject.h"

#include <glm/gtc/matrix_transform.hpp>

namespace OM3D {

SceneObject::SceneObject(std::shared_ptr<StaticMesh> mesh, std::shared_ptr<Material> material) :
    _mesh(std::move(mesh)),
    _material(std::move(material)) {
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
    glm::vec3 direction = glm::normalize(this->_mesh.get()->center() - camera.position());

    return glm::dot(camera.forward(), direction) > 0.8; // TODO fix with the frutsum to get the right HitBox
}

const glm::mat4& SceneObject::transform() const {
    return _transform;
}

}
