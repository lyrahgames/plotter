#pragma once

namespace plotter {

template <typename T>
struct aabb {
  aabb() = default;

  T min[2]{};
  T max[2]{};
};

}  // namespace plotter