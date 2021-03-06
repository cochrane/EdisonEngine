#pragma once

#include "shaderprogram.h"

#include <gl/renderstate.h>
#include <gsl-lite.hpp>

namespace render::scene
{
class UniformParameter;
class UniformBlockParameter;
class BufferParameter;

class Node;

class Material final
{
public:
  explicit Material(gsl::not_null<std::shared_ptr<ShaderProgram>> shaderProgram);

  Material(const Material&) = delete;

  Material(Material&&) = delete;

  Material& operator=(const Material&) = delete;

  Material& operator=(Material&&) = delete;

  ~Material();

  const gsl::not_null<std::shared_ptr<ShaderProgram>>& getShaderProgram() const
  {
    return m_shaderProgram;
  }

  void bind(const Node& node) const;

  gsl::not_null<std::shared_ptr<UniformParameter>> getUniform(const std::string& name) const;
  gsl::not_null<std::shared_ptr<UniformBlockParameter>> getUniformBlock(const std::string& name) const;
  gsl::not_null<std::shared_ptr<BufferParameter>> getBuffer(const std::string& name) const;

  gl::RenderState& getRenderState()
  {
    return m_renderState;
  }

private:
  gsl::not_null<std::shared_ptr<ShaderProgram>> m_shaderProgram;

  mutable std::vector<gsl::not_null<std::shared_ptr<UniformParameter>>> m_uniforms;
  mutable std::vector<gsl::not_null<std::shared_ptr<UniformBlockParameter>>> m_uniformBlocks;
  mutable std::vector<gsl::not_null<std::shared_ptr<BufferParameter>>> m_buffers;

  gl::RenderState m_renderState{};
};
} // namespace render::scene
