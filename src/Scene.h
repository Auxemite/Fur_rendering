#ifndef SCENE_H
#define SCENE_H

#include <SceneObject.h>
#include <PointLight.h>
#include <Camera.h>
#include <shader_structs.h>

#include <vector>
#include <memory>

namespace OM3D {
    class Scene : NonMovable {

    public:
        Scene();

        static Result<std::unique_ptr<Scene>> from_gltf(const std::string& file_name);

        TypedBuffer<shader::FrameData> get_sun_frame_data();
        TypedBuffer<shader::PointLight> get_lights_frame_data();
        void render(const RenderMode& renderMode, int rendered_nb, bool fur) const;

        void add_object(SceneObject obj);
        void copy_object(int i, const glm::vec3& pos);
        void add_light(PointLight obj);

        Span<const SceneObject> objects() const;
        Span<const PointLight> point_lights() const;

        Camera& camera();
        const Camera& camera() const;
        void set_camera(const Camera& camera);

        void set_sun(glm::vec3 direction, glm::vec3 color = glm::vec3(1.0f));
        glm::vec3 get_sun();
        glm::vec3 get_sun_color();

    private:
        std::vector<SceneObject> _objects;
        std::vector<PointLight> _point_lights;

        glm::vec3 _sun_direction = glm::vec3(0.2f, 1.0f, 0.1f);
        glm::vec3 _sun_color = glm::vec3(1.0f);


        Camera _camera;
};

}

#endif // SCENE_H
