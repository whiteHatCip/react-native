/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <cstdlib>
#include <limits>

#include <react/components/image/ImageShadowNode.h>
#include <react/core/LayoutContext.h>
#include "ImageState.h"

namespace facebook {
namespace react {

const char ImageComponentName[] = "Image";

void ImageShadowNode::setImageManager(const SharedImageManager &imageManager) {
  ensureUnsealed();
  imageManager_ = imageManager;
}

void ImageShadowNode::updateStateIfNeeded() {
  ensureUnsealed();

  auto const &imageSource = getImageSource();
  auto const &currentState = getStateData();
  bool hasSameRadius =
      getConcreteProps().blurRadius == currentState.getBlurRadius();
  bool hasSameImageSource = currentState.getImageSource() == imageSource;

  if (hasSameImageSource && hasSameRadius) {
    return;
  }

  auto state =
      ImageState{imageSource,
                 imageManager_->requestImage(imageSource, getSurfaceId()),
                 getConcreteProps().blurRadius};
  setStateData(std::move(state));
}

ImageSource ImageShadowNode::getImageSource() const {
  auto sources = getConcreteProps().sources;

  if (sources.size() == 0) {
    return {
        /* .type = */ ImageSource::Type::Invalid,
    };
  }

  if (sources.size() == 1) {
    return sources[0];
  }

  auto layoutMetrics = getLayoutMetrics();
  auto size = layoutMetrics.getContentFrame().size;
  auto scale = layoutMetrics.pointScaleFactor;
  auto targetImageArea = size.width * size.height * scale * scale;
  auto bestFit = std::numeric_limits<Float>::infinity();

  auto bestSource = ImageSource{};

  for (const auto &source : sources) {
    auto sourceSize = source.size;
    auto sourceScale = source.scale == 0 ? scale : source.scale;
    auto sourceArea =
        sourceSize.width * sourceSize.height * sourceScale * sourceScale;

    auto fit = std::abs(1 - (sourceArea / targetImageArea));

    if (fit < bestFit) {
      bestFit = fit;
      bestSource = source;
    }
  }

  return bestSource;
}

#pragma mark - LayoutableShadowNode

void ImageShadowNode::layout(LayoutContext layoutContext) {
  updateStateIfNeeded();
  ConcreteViewShadowNode::layout(layoutContext);
}

} // namespace react
} // namespace facebook
