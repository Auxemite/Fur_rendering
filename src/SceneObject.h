#ifndef SCENEOBJECT_H
#define SCENEOBJECT_H

#include <StaticMesh.h>
#include <Material.h>
#include <Camera.h>

#include <memory>

#include <glm/matrix.hpp>

namespace OM3D {

    inline int shell_number = 64;
    inline float scale_base = 1.0f;
    inline float density = 100.f;
    inline float rigidity = 10.f;
    inline float base_thickness = 1.5f; // [0. - 1.5]
    inline float tip_thickness = .2f; // [0. - 1.5]
    inline float fur_length = 0.5f;
    inline float fur_lighting = 0.0f;
    inline float roughness = 0.6f;
    inline float metaless = 0.0f;
    inline float ambient = 0.1f;

class SceneObject {

    public:
        SceneObject(std::shared_ptr<StaticMesh> mesh = nullptr, std::shared_ptr<Material> material = nullptr);

        void render(const RenderMode& renderMode, bool fur, const float dist) const;

        void set_transform(const glm::mat4& tr);
        void set_center(const glm::vec3& center);
        void set_material(std::shared_ptr<Material> material);
        glm::vec3 get_center() const;
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
