#include "Material.h"

#include <glad/gl.h>

#include <algorithm>
#include <iostream>

namespace OM3D {

Material::Material() {
}

void Material::set_program(std::shared_ptr<Program> prog) {
    _program = std::move(prog);
}

void Material::set_blend_mode(BlendMode blend) {
    _blend_mode = blend;
}

void Material::set_depth_test_mode(DepthTestMode depth) {
    _depth_test_mode = depth;
}

void Material::set_texture(u32 slot, std::shared_ptr<Texture> tex) {
    if(const auto it = std::find_if(_textures.begin(), _textures.end(), [&](const auto& t) { return t.second == tex; }); it != _textures.end()) {
        it->second = std::move(tex);
    } else {
        _textures.emplace_back(slot, std::move(tex));
    }
}

void Material::bind([[__maybe_unused__]]const RenderMode& renderMode, bool fur) const {
    switch(_blend_mode) {
        case BlendMode::None:
            glDisable(GL_BLEND);

            // Backface Culling
            glEnable(GL_CULL_FACE); // For fur
            glCullFace(GL_BACK);
            glFrontFace(GL_CCW);
        break;

        case BlendMode::Alpha:
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            // Disable backface culling for transparent object
            glDisable(GL_CULL_FACE);

        break;

        case BlendMode::Additif:
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);

            break;
    }

    switch(_depth_test_mode) {
        case DepthTestMode::None:
            glDisable(GL_DEPTH_TEST);
        break;

        case DepthTestMode::Equal:
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_EQUAL);
        break;

        case DepthTestMode::Standard:
            glEnable(GL_DEPTH_TEST);
            // We are using reverse-Z
            glDepthFunc(GL_GEQUAL);
        break;

        case DepthTestMode::Reversed:
            glEnable(GL_DEPTH_TEST);
            // We are using reverse-Z
            glDepthFunc(GL_LEQUAL);
        break;
    }

    for (const auto &texture: _textures) {
        texture.second->bind(texture.first);
    }
    if (fur) {
        if (kajyia_Kay)
            _program_fur_kjk->bind();
        else
            _program_fur->bind();
    }
    else
        _program->bind();
}

std::shared_ptr<Material> Material::empty_material() {
    static std::weak_ptr<Material> weak_material;
    auto material = weak_material.lock();
    std::cout << "Creating empty material" << std::endl;
    if(!material) {
        material = std::make_shared<Material>();
        material->_program = Program::from_files("lit.frag", "basic.vert", "");
        material->_program_fur = Program::from_files("fur/fur.frag", "fur/fur.vert", "fur/fur.geom", std::array<std::string, 2>{"TEXTURED", "NORMAL_MAPPED"});
        material->_program_fur_kjk = Program::from_files("fur/fur_kjk.frag", "fur/fur.vert", "fur/fur.geom", std::array<std::string, 2>{"TEXTURED", "NORMAL_MAPPED"});
        weak_material = material;
    }
    return material;
}

Material Material::textured_material() {
    Material material;
    std::cout << "Creating texture material" << std::endl;
    material._program = Program::from_files("g_buffer.frag", "basic.vert", "", {"TEXTURED"});
    material._program_fur = Program::from_files("fur/fur.frag", "fur/fur.vert", "fur/fur.geom", std::array<std::string, 2>{"TEXTURED", "NORMAL_MAPPED"});
    material._program_fur_kjk = Program::from_files("fur/fur_kjk.frag", "fur/fur.vert", "fur/fur.geom", std::array<std::string, 2>{"TEXTURED", "NORMAL_MAPPED"});
    return material;
}

Material Material::textured_normal_mapped_material() {
    Material material;
    std::cout << "Creating normal mapped material" << std::endl;
    material._program = Program::from_files("g_buffer.frag", "basic.vert", "",
                                            std::array<std::string, 2>{"TEXTURED", "NORMAL_MAPPED"});
    material._program_fur = Program::from_files("fur/fur.frag", "fur/fur.vert", "fur/fur.geom",
                                                std::array<std::string, 2>{"TEXTURED", "NORMAL_MAPPED"});                             
    material._program_fur_kjk = Program::from_files("fur/fur_kjk.frag", "fur/fur.vert", "fur/fur.geom",
                                                    std::array<std::string, 2>{"TEXTURED", "NORMAL_MAPPED"});
    return material;
}


}
