#ifndef SCENEOBJECT_H
#define SCENEOBJECT_H

#include <StaticMesh.h>
#include <Material.h>
#include <Camera.h>

#include <memory>

#include <glm/matrix.hpp>

namespace OM3D {

    inline float scale_base = 1.0f;
    inline float scale_modifier = 0.01f;
    inline float density_base = 0.5f;
    inline float density_modifier = 1.5f;
    inline float thickness = 500.0f;

class SceneObject {

    public:
        SceneObject(std::shared_ptr<StaticMesh> mesh = nullptr, std::shared_ptr<Material> material = nullptr);

        void render(const RenderMode& renderMode, bool fur) const;

        void set_transform(const glm::mat4& tr);
        void set_center(const glm::vec3& center);
        const glm::mat4& transform() const;
        bool is_visible(const Camera& camera) const;
        bool is_in_range(const glm::vec3& position, const float& radius) const;
        void print_info() const;


    private:
        glm::mat4 _transform = glm::mat4(1.0f);

        std::shared_ptr<StaticMesh> _mesh;
        std::shared_ptr<Material> _material;
        glm::vec3 _center;
        float _radius;
};

}

#endif // SCENEOBJECT_H
