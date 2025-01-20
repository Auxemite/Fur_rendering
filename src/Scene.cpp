#include "Scene.h"

#include <TypedBuffer.h>
#include <iostream>

namespace OM3D {

Scene::Scene() {
}

void Scene::add_object(SceneObject obj) {
    _objects.emplace_back(std::move(obj));
}

void Scene::copy_object(int i, const glm::vec3& pos) {
    SceneObject obj = _objects[i];
    obj.set_center(pos);
    _objects.emplace_back(obj);
}

void Scene::add_light(PointLight obj) {
    _point_lights.emplace_back(std::move(obj));
}

Span<const SceneObject> Scene::objects() const {
    return _objects;
}

Span<const PointLight> Scene::point_lights() const {
    return _point_lights;
}

Camera& Scene::camera() {
    return _camera;
}

void Scene::set_camera(const Camera& camera) {
    _camera = camera;
};

const Camera& Scene::camera() const {
    return _camera;
}

void Scene::set_sun(glm::vec3 direction, glm::vec3 color) {
    _sun_direction = direction;
    _sun_color = color;
}

glm::vec3 Scene::get_sun() {
    return _sun_direction;
}

glm::vec3 Scene::get_sun_color() {
    return _sun_color;
}

TypedBuffer<shader::FrameData> Scene::get_sun_frame_data() {
    TypedBuffer<shader::FrameData> buffer(nullptr, 1);
    {
        auto mapping = buffer.map(AccessType::WriteOnly);
        mapping[0].camera.view_proj = _camera.view_proj_matrix();
        mapping[0].point_light_count = u32(_point_lights.size());
        mapping[0].sun_color = _sun_color;
        mapping[0].sun_dir = glm::normalize(_sun_direction);
    }
    return buffer;
}

TypedBuffer<shader::PointLight> Scene::get_lights_frame_data() {
    TypedBuffer<shader::PointLight> light_buffer(nullptr, std::max(_point_lights.size(), size_t(1)));
    {
        auto mapping = light_buffer.map(AccessType::WriteOnly);
        for(size_t i = 0; i != _point_lights.size(); ++i) {
            const auto& light = _point_lights[i];
            mapping[i] = {
                    light.position(),
                    light.radius(),
                    light.color(),
                    0.0f
            };
        }
    }
    return light_buffer;
}

void Scene::render(const RenderMode& renderMode, int rendered_nb, bool fur) const {
    // Fill and bind frame data buffer
    TypedBuffer<shader::FrameData> buffer(nullptr, 1);
    {
        auto mapping = buffer.map(AccessType::WriteOnly);
        mapping[0].camera.view_proj = _camera.view_proj_matrix();
        mapping[0].point_light_count = u32(_point_lights.size());
        mapping[0].sun_color = _sun_color;
        mapping[0].sun_dir = glm::normalize(_sun_direction);
    }
    buffer.bind(BufferUsage::Uniform, 0);

    // Fill and bind lights buffer
    TypedBuffer<shader::PointLight> light_buffer(nullptr, std::max(_point_lights.size(), size_t(1)));
    auto mapping = light_buffer.map(AccessType::WriteOnly);
    for(size_t i = 0; i != _point_lights.size(); ++i) {
        const auto& light = _point_lights[i];
        if (light.is_visible(_camera)) {
            mapping[i] = {
                    light.position(),
                    light.radius(),
                    light.color(),
                    0.0f
            };
        }
    }
    light_buffer.bind(BufferUsage::Storage, 1);

    int count = 0;
    // Render every object
    for(const SceneObject& obj : _objects)
    {
        if(obj.is_visible(_camera))
        {
            obj.render(renderMode, fur);
            count++;
        }
    }
    if (rendered_nb == 60)
        printf("Rendered %d objects\n", count);
}

}
