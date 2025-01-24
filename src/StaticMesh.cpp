#include "StaticMesh.h"

#include <glad/gl.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

namespace OM3D {

    extern bool audit_bindings_before_draw;

    StaticMesh::StaticMesh(const MeshData& data) :
        _vertex_buffer(data.vertices),
        _index_buffer(data.indices) {}

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

    void StaticMesh::draw_fur(std::vector<float> shells) const {
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

        // Fur shell instancing
        TypedBuffer<float> shells_rank(shells);
        shells_rank.bind(BufferUsage::Attribute);
        glVertexAttribPointer(5, 1, GL_FLOAT, false, sizeof(float), reinterpret_cast<void*>(0 * sizeof(float)));
        glEnableVertexAttribArray(5);
        glVertexAttribDivisor(5, 1);

        if(audit_bindings_before_draw) {
            audit_bindings();
        }

        // Draw instances
        glDrawElementsInstanced(GL_TRIANGLES, int(_index_buffer.element_count()), GL_UNSIGNED_INT, nullptr, int(shells_rank.element_count()));
    }

    size_t StaticMesh::vertices_count()
    {
        return this->_vertex_buffer.element_count();
    }

    Vertex* StaticMesh::vertices()
    {
        return this->_vertex_buffer.map(AccessType::ReadOnly).data();
    }
}
