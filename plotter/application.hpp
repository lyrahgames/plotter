#pragma once
#include <SFML/Graphics.hpp>
#include <algorithm>
#include <atomic>
#include <cmath>
#include <future>
#include <list>
#include <thread>
#include <vector>

namespace plotter {

class application {
 public:
  application();
  ~application();

  application& fit_view();
  application& fit_aspect_view();
  application& fit_tiks();
  template <typename InputIt1, typename InputIt2>
  application& plot(InputIt1 x_first, InputIt1 x_last, InputIt2 y_first);
  template <typename Function>
  application& plot(Function&& f, float min, float max, size_t samples);
  application& execute();

 private:
  void process_mouse();
  void process_events();
  void render();
  void resize();

  void draw_plot_background();
  void draw_tiks();
  void draw_function();
  void draw_plot_border();

 private:
  std::future<void> execute_task;
  sf::RenderWindow window;
  sf::RenderTexture texture;

  bool update = true;

  sf::Color background_color{sf::Color::White};

  int old_mouse_x = 0;
  int old_mouse_y = 0;
  int mouse_x = 0;
  int mouse_y = 0;
  int mouse_diff_x = 0;
  int mouse_diff_y = 0;
  enum {
    NONE,
    PLOT_FOCUS,
    X_AXIS_FOCUS,
    Y_AXIS_FOCUS
  } mouse_focus,
      mouse_click_focus;

  float view_x_min;
  float view_x_max;
  float view_y_min;
  float view_y_max;

  sf::Color plot_background_color{220, 220, 220};
  sf::Color gridlines_color{sf::Color::White};
  float gridlines_size = 1.0f;
  sf::Color m_gridlines_color{240, 240, 240};
  float m_gridlines_size = 0.8f;

  sf::Color plot_border_color{sf::Color::Black};
  float plot_border_size = 2.0f;

  float plot_pad = 100.0f;
  float plot_x_min = 5.0f;
  float plot_x_max = 100.0f;
  float plot_y_min = 5.0f;
  float plot_y_max = 150.0f;

  float x_tics = 5.f;
  float y_tics = 0.5f;
  size_t x_m_tics = 4;
  size_t y_m_tics = 4;

  size_t x_precision = 2;
  size_t y_precision = 2;

  sf::Font font;

  struct sampled_path {
    sampled_path() = default;
    template <typename InputIt1, typename InputIt2>
    sampled_path(InputIt1 x_first, InputIt1 x_last, InputIt2 y_first);
    template <typename Function>
    sampled_path(Function&& f, float min, float max, size_t samples);

    sf::Color point_color{sf::Color::Black};
    float point_size = 0.0f;
    sf::Color line_color{sf::Color::Black};
    float line_size = 1.5f;
    std::vector<float> x_data{};
    std::vector<float> y_data{};
    float x_min;
    float x_max;
    float y_min;
    float y_max;
  };
  std::list<sampled_path> sampled_paths{};

  sf::Color point_color{sf::Color::Black};
  float point_size = 2.0f;
  float line_size = 3;
  sf::Color line_color{sf::Color::Blue};

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
  sampled_paths.emplace_back(x_first, x_last, y_first);
  // x_data.clear();
  // y_data.clear();
  // auto x_it = x_first;
  // auto y_it = y_first;
  // for (; x_it != x_last; ++x_it, ++y_it) {
  //   x_data.push_back(*x_it);
  //   y_data.push_back(*y_it);
  // }
  return *this;
}

template <typename InputIt1, typename InputIt2>
application::sampled_path::sampled_path(InputIt1 x_first, InputIt1 x_last,
                                        InputIt2 y_first) {
  x_data.clear();
  y_data.clear();
  auto x_it = x_first;
  auto y_it = y_first;
  for (; x_it != x_last; ++x_it, ++y_it) {
    x_data.push_back(*x_it);
    y_data.push_back(*y_it);
  }
}

template <typename Function>
application& application::plot(Function&& f, float min, float max,
                               size_t samples) {
  sampled_paths.emplace_back(std::forward<Function>(f), min, max, samples);

  // x_min = min;
  // x_max = max;
  // x_data.resize(samples);
  // y_data.resize(samples);
  // for (size_t i = 0; i < samples; ++i) {
  //   const auto scale = static_cast<float>(i) / (samples - 1);
  //   const auto x = x_min * (1.0f - scale) + x_max * scale;
  //   const auto y = f(x);
  //   x_data[i] = x;
  //   y_data[i] = y;
  // }
  return *this;
}

template <typename Function>
application::sampled_path::sampled_path(Function&& f, float min, float max,
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
}

}  // namespace plotter