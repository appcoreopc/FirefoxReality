/* -*- Mode: C++; tab-width: 20; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "VRLayer.h"
#include "vrb/ConcreteClass.h"
#include "vrb/Matrix.h"

namespace crow {

static uint64_t sIndex = 0;

struct VRLayer::State {
  int32_t priority;
  uint64_t drawIndex;
  bool drawRequested;
  VRLayer::LayerType layerType;
  vrb::Matrix modelView[2];
  device::Eye currentEye;
  State(): priority(0),
           drawIndex(0),
           drawRequested(false),
           currentEye(device::Eye::Left)
  {
    modelView[0] = vrb::Matrix::Identity();
    modelView[1] = vrb::Matrix::Identity();
  }
};

VRLayer::LayerType
VRLayer::GetLayerType() const {
  return m.layerType;
}

bool
VRLayer::IsDrawRequested() const {
  return m.drawRequested;
}


const vrb::Matrix&
VRLayer::GetModelView(device::Eye aEye) const {
  return m.modelView[device::EyeIndex(aEye)];
}

device::Eye
VRLayer::GetCurrentEye() const {
  return m.currentEye;
}

int32_t
VRLayer::GetPriority() const {
  return m.priority;
}

bool
VRLayer::ShouldDrawBefore(const VRLayer& aLayer) {
  if (m.layerType == VRLayer::LayerType::CUBEMAP || m.layerType == VRLayer::LayerType::EQUIRECTANGULAR) {
    return true;
  }

  if (aLayer.m.layerType == VRLayer::LayerType::CUBEMAP || aLayer.m.layerType == VRLayer::LayerType::EQUIRECTANGULAR) {
    return false;
  }

  if (m.priority != aLayer.m.priority) {
    return m.priority > aLayer.m.priority;
  }

  return m.drawIndex < aLayer.m.priority;
}

void
VRLayer::RequestDraw() {
  if (!m.drawRequested) {
    m.drawIndex = ++sIndex;
  }
  m.drawRequested = true;
}

void
VRLayer::ClearRequestDraw() {
  m.drawRequested = false;
}

void
VRLayer::SetModelView(device::Eye aEye, const vrb::Matrix& aModelView) {
  m.modelView[device::EyeIndex(aEye)] = aModelView;
}

void
VRLayer::SetCurrentEye(crow::device::Eye aEye) {
  m.currentEye = aEye;
}

void
VRLayer::SetPriority(int32_t aPriority) {
  m.priority = aPriority;
}


VRLayer::VRLayer(State& aState, LayerType aLayerType): m(aState) {
  m.layerType = aLayerType;
}

struct VRLayerQuad::State: public VRLayer::State {
  int32_t width;
  int32_t height;
  int32_t priority;
  float worldWidth;
  float worldHeight;
  VRLayerQuad::SurfaceChangeDelegate surfaceDelegate;
  VRLayerQuad::ResizeDelegate resizeDelegate;
  jobject surface;
  State():
      width(0),
      height(0),
      worldWidth(0),
      worldHeight(0),
      priority(0),
      surface(nullptr)
  {}
};

VRLayerQuadPtr
VRLayerQuad::Create(const int32_t aWidth, const int32_t aHeight) {
  auto result = std::make_shared<vrb::ConcreteClass<VRLayerQuad, VRLayerQuad::State>>();
  result->m.width = aWidth;
  result->m.height = aHeight;
  return result;
}

int32_t
VRLayerQuad::GetWidth() const {
  return m.width;
}

int32_t
VRLayerQuad::GetHeight() const {
  return m.height;
}

float
VRLayerQuad::GetWorldWidth() const {
  return m.worldWidth;
}

float
VRLayerQuad::GetWorldHeight() const {
  return m.worldHeight;
}


void
VRLayerQuad::SetWorldSize(const float aWidth, const float aHeight) {
  m.worldWidth = aWidth;
  m.worldHeight = aHeight;
}

void
VRLayerQuad::Resize(const int32_t aWidth, const int32_t aHeight) {
  if (m.width == aWidth && m.height == aHeight) {
    return;
  }
  m.width = aWidth;
  m.height = aHeight;
  if (m.resizeDelegate) {
    m.resizeDelegate();
  }
}

void
VRLayerQuad::SetSurfaceChangeDelegate(const SurfaceChangeDelegate& aDelegate) {
  m.surfaceDelegate = aDelegate;
  if (m.surface && m.surfaceDelegate) {
    m.surfaceDelegate(m.surface, m.width, m.height);
  }
}

void
VRLayerQuad::SetResizeDelegate(const ResizeDelegate& aDelegate) {
  m.resizeDelegate = aDelegate;
}

void
VRLayerQuad::SetSurface(jobject aSurface) {
  if (m.surfaceDelegate) {
    m.surfaceDelegate(aSurface, m.width, m.height);
  }
}


VRLayerQuad::VRLayerQuad(State& aState): VRLayer(aState, LayerType::QUAD), m(aState) {

}

VRLayerQuad::~VRLayerQuad() {}

} // namespace crow
