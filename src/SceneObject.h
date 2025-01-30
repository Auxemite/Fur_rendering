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
    inline float fur_density = 325.f;
    inline float hair_rigidity = 25.f;
    inline float base_thickness = 1.5f; // [0. - 1.5]
    inline float tip_thickness = .2f; // [0. - 1.5]
    inline float fur_length = 0.5f;

    // Hair constraint
    inline float hair_fuzziness = 2.f; // [0. - 10.]
    inline float hair_fuzz_seed = 2.f; // [0. - 10.]
    
    inline float hair_curliness = 2.f; // [0. - 50.]
    inline float hair_curl_size = .2f; // [0. - 1.]

    inline float hair_min_length = 0.f; // [0. - 1.]
    inline float hair_max_length = 1.f; // [0. - 1.]
  
    // BRDF
    inline float fur_lighting = 0.0f;
    inline float roughness = 0.2f;
    inline float metaless = 0.7f;
    inline float ambient = 0.0f;
    inline float ambient_occlusion = 1.0f;
  
    // Wind
    inline float wind_strength = 0.f;
    inline float wind_alpha = 0.f; // [-10. - 10.]
    inline float wind_beta = 5.f; // // [0. - 10.]
    inline float turbulence_strength = 0.f; // // [0. - 10.]

    // Movement
    // inline glm::vec3 movement;

    // Fins
    inline bool show_shell = true;
    inline bool show_fins = true;
    inline float fins_threshold = 0.5; // [0. - 1.]

class SceneObject {

    public:
        SceneObject(std::shared_ptr<StaticMesh> mesh = nullptr, std::shared_ptr<Material> material = nullptr);

        void render(const RenderMode& renderMode, bool active_fur, float time) const;

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
        bool fur = true;
};

}

#endif // SCENEOBJECT_H
