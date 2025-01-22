#ifndef SCENEOBJECT_H
#define SCENEOBJECT_H

#include <StaticMesh.h>
#include <Material.h>
#include <Camera.h>

#include <memory>

#include <glm/matrix.hpp>

namespace OM3D {

    // Fur
    inline int shell_number = 32;
    inline float scale_base = 1.0f;
    inline float density = 325.f;
    inline float rigidity = 25.f;
    inline float base_thickness = 1.5f; // [0. - 1.5]
    inline float tip_thickness = .05f; // [0. - 1.5]
    inline float fur_length = 3.75f;
    inline float min_length = 0.f; // [0. - 1.]
    inline float max_length = 1.f; // [0. - 1.]

    // Wind
    inline float wind_strength = 10.f;
    inline float wind_alpha = 0.f; // [-10. - 10.]
    inline float wind_beta = 5.f; // // [0. - 10.]

class SceneObject {

    public:
        SceneObject(std::shared_ptr<StaticMesh> mesh = nullptr, std::shared_ptr<Material> material = nullptr);

        void render(const RenderMode& renderMode, bool fur, float time) const;

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
