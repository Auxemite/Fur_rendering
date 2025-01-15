#ifndef POINTLIGHT_H
#define POINTLIGHT_H

#include <utils.h>

#include <glm/vec3.hpp>

namespace OM3D {

static bool in_plane(glm::vec3 plane_normal, const glm::vec3& origin, const glm::vec3 center, const float radius)
{
    plane_normal = glm::normalize(plane_normal);
    glm::vec3 direction = center - origin;
    float d = glm::dot(plane_normal, direction);
    return d >= -radius;
}

class PointLight {

    public:
        PointLight() = default;

        void set_position(const glm::vec3& pos) {
            _position = pos;
        }

        void set_color(const glm::vec3& color) {
            _color = color;
        }

        void set_radius(float radius) {
            _radius = radius;
        }


        const glm::vec3& position() const {
            return _position;
        }

        const glm::vec3& color() const {
            return _color;
        }

        float radius() const {
            return _radius;
        }

        bool is_visible(const Camera& camera) const
        {
            const glm::vec3 origin = camera.position();
            const Frustum frut = camera.build_frustum();

            // Update radius, assuming that the scales correspond to diameter (imply *0.5f)
            const float radius = _radius;

            return (in_plane(frut._near_normal, origin, _position, radius) &&
                    in_plane(frut._left_normal, origin, _position, radius) &&
                    in_plane(frut._right_normal, origin, _position, radius) &&
                    in_plane(frut._top_normal, origin, _position, radius) &&
                    in_plane(frut._bottom_normal, origin, _position, radius));
        };

    private:
        glm::vec3 _position = {};
        glm::vec3 _color = glm::vec3(1.0f);
        float _radius = 10.0f;
};

}

#endif // POINTLIGHT_H
