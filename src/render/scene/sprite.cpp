#include "sprite.h"

#include "camera.h"
#include "material.h"
#include "mesh.h"
#include "names.h"
#include "node.h"
#include "render/gl/vertexarray.h"
#include "scene.h"

namespace render::scene
{
gsl::not_null<std::shared_ptr<Mesh>> Sprite::createMesh(const float x0,
                                                        const float y0,
                                                        const float x1,
                                                        const float y1,
                                                        const glm::vec2& t0,
                                                        const glm::vec2& t1,
                                                        const gsl::not_null<std::shared_ptr<Material>>& materialFull,
                                                        const Axis pole)
{
  struct SpriteVertex
  {
    glm::vec3 pos;

    glm::vec2 uv;

    glm::vec3 color{1.0f};
  };

  const SpriteVertex vertices[]{
    {{x0, y0, 0}, {t0.x, t0.y}}, {{x1, y0, 0}, {t1.x, t0.y}}, {{x1, y1, 0}, {t1.x, t1.y}}, {{x0, y1, 0}, {t0.x, t1.y}}};

  gl::StructureLayout<SpriteVertex> layout{{VERTEX_ATTRIBUTE_POSITION_NAME, &SpriteVertex::pos},
                                           {VERTEX_ATTRIBUTE_TEXCOORD_PREFIX_NAME, &SpriteVertex::uv},
                                           {VERTEX_ATTRIBUTE_COLOR_NAME, &SpriteVertex::color}};
  auto vb = std::make_shared<gl::StructuredArrayBuffer<SpriteVertex>>(layout);
  vb->setData(&vertices[0], 4, ::gl::BufferUsageARB::StaticDraw);

  static const uint16_t indices[6] = {0, 1, 2, 0, 2, 3};

  auto indexBuffer = std::make_shared<gl::ElementArrayBuffer<uint16_t>>();
  indexBuffer->setData(gsl::not_null<const uint16_t*>(&indices[0]), 6, ::gl::BufferUsageARB::StaticDraw);

  auto vao = std::make_shared<gl::VertexArray<uint16_t, SpriteVertex>>(
    indexBuffer, vb, std::vector<const gl::Program*>{&materialFull->getShaderProgram()->getHandle()});
  auto mesh = std::make_shared<MeshImpl<uint16_t, SpriteVertex>>(vao);
  mesh->getMaterial().set(RenderMode::Full, materialFull);

  mesh->registerMaterialUniformSetter([pole](const Node& node, Material& material) {
    material.getUniform("u_spritePole")->set(static_cast<int32_t>(pole));
  });

  return mesh;
}

void Sprite::render(RenderContext& context)
{
  m_mesh->render(context);
}
} // namespace render::scene
