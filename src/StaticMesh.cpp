#include "StaticMesh.h"

#include <glad/gl.h>

namespace OM3D {

    extern bool audit_bindings_before_draw;

    StaticMesh::StaticMesh(const MeshData& data) :
        _vertex_buffer(data.vertices),
        _index_buffer(data.indices) {
        glm::vec3 center = glm::vec3(0.0f, 0.0f, 0.0f);
        for (const auto & vertice : data.vertices) {
            center += vertice.position;
        }
        center /= static_cast<float>(data.vertices.size());
        glm::vec3 max_position = data.vertices[0].position;
        for (const auto & vertice : data.vertices) {
//            if ()
//            center += vertice.position;
        }
        // ICI
    }

    void StaticMesh::draw() const {
        _vertex_buffer.bind(BufferUsage::Attribute);
        _index_buffer.bind(BufferUsage::Index);

        // Vertex position
        glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(Vertex), nullptr);
        // Vertex normal
        glVertexAttribPointer(1, 3, GL_FLOAT, false, sizeof(Vertex), reinterpret_cast<void*>(3 * sizeof(float)));
        // Vertex uv
        glVertexAttribPointer(2, 2, GL_FLOAT, false, sizeof(Vertex), reinterpret_cast<void*>(6 * sizeof(float)));
        // Tangent / bitangent sign
        glVertexAttribPointer(3, 4, GL_FLOAT, false, sizeof(Vertex), reinterpret_cast<void*>(8 * sizeof(float)));
        // Vertex color
        glVertexAttribPointer(4, 3, GL_FLOAT, false, sizeof(Vertex), reinterpret_cast<void*>(12 * sizeof(float)));

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        glEnableVertexAttribArray(3);
        glEnableVertexAttribArray(4);

        if(audit_bindings_before_draw) {
            audit_bindings();
        }

        glDrawElements(GL_TRIANGLES, int(_index_buffer.element_count()), GL_UNSIGNED_INT, nullptr);
    }

}
