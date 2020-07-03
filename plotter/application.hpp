#pragma once
#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>
#include <vector>

namespace plotter {

class application {
 public:
  application();
  ~application() = default;

  application& fit_view();
  application& fit_tiks();
  template <typename InputIt1, typename InputIt2>
  application& plot(InputIt1 x_first, InputIt1 x_last, InputIt2 y_first);
  template <typename Function>
  application& plot(Function&& f, float min, float max, size_t samples);
  application& execute();

 private:
  void process_events();
  void render();
  void resize();

 private:
  sf::RenderWindow window;
  sf::RenderTexture texture;
  sf::Color background_color{sf::Color::White};
  sf::Color point_color{sf::Color::Black};
  float point_size = 1.0f;

  int old_mouse_x = 0;
  int old_mouse_y = 0;
  int mouse_x = 0;
  int mouse_y = 0;
  int mouse_diff_x = 0;
  int mouse_diff_y = 0;

  float view_x_min;
  float view_x_max;
  float view_y_min;
  float view_y_max;

  sf::Color plot_background_color{220, 220, 220};
  sf::Color gridlines_color{sf::Color::White};
  float gridlines_size = 1.0f;
  sf::Color m_gridlines_color{240, 240, 240};
  float m_gridlines_size = 0.8f;

  float plot_pad = 100.0f;
  float plot_x_min = 5.0f;
  float plot_x_max = 100.0f;
  float plot_y_min = 5.0f;
  float plot_y_max = 150.0f;

  float x_tics = 5.f;
  float y_tics = 0.5f;
  size_t x_m_tics = 4;
  size_t y_m_tics = 4;

  float line_size = 3;
  sf::Color line_color{sf::Color::Blue};

  sf::Font font;

  float x_min = -10.0f;
  float x_max = 10.0f;
  float y_min;
  float y_max;
  std::vector<float> x_data;
  std::vector<float> y_data;
};

template <typename InputIt1, typename InputIt2>
application& application::plot(InputIt1 x_first, InputIt1 x_last,
                               InputIt2 y_first) {
  x_data.clear();
  y_data.clear();
  auto x_it = x_first;
  auto y_it = y_first;
  for (; x_it != x_last; ++x_it, ++y_it) {
    x_data.push_back(*x_it);
    y_data.push_back(*y_it);
  }
  return *this;
}

template <typename Function>
application& application::plot(Function&& f, float min, float max,
                               size_t samples) {
  x_min = min;
  x_max = max;
  x_data.resize(samples);
  y_data.resize(samples);
  for (size_t i = 0; i < samples; ++i) {
    const auto scale = static_cast<float>(i) / (samples - 1);
    const auto x = x_min * (1.0f - scale) + x_max * scale;
    const auto y = f(x);
    x_data[i] = x;
    y_data[i] = y;
  }
  return *this;
}

}  // namespace plotter