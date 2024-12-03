#ifndef SCENE_H
#define SCENE_H

#include <SceneObject.h>
#include <PointLight.h>
#include <Camera.h>

#include <vector>
#include <memory>

namespace OM3D {
/*
    enum class RenderMode {
        Default = 0,
        Albedo = 1,
        Normals = 2,
        Depth = 3,
    };

    static RenderMode render_mode = RenderMode::Default;
*/
    class Scene : NonMovable {

    public:
        Scene();

        static Result<std::unique_ptr<Scene>> from_gltf(const std::string& file_name);

        void render(const u32& renderMode, int i) const;

        void add_object(SceneObject obj);
        void add_light(PointLight obj);

        Span<const SceneObject> objects() const;
        Span<const PointLight> point_lights() const;

        Camera& camera();
        const Camera& camera() const;

        void set_sun(glm::vec3 direction, glm::vec3 color = glm::vec3(1.0f));

    private:
        std::vector<SceneObject> _objects;
        std::vector<PointLight> _point_lights;

        glm::vec3 _sun_direction = glm::vec3(0.2f, 1.0f, 0.1f);
        glm::vec3 _sun_color = glm::vec3(1.0f);


        Camera _camera;
};

}

#endif // SCENE_H
