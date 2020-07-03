#include <iomanip>
#include <iostream>
#include <plotter/application.hpp>
#include <sstream>

namespace plotter {

application::application() {
  sf::ContextSettings settings;
  settings.antialiasingLevel = 8;
  window.create(sf::VideoMode(500, 500), "Plotter", sf::Style::Default,
                settings);
  window.setVerticalSyncEnabled(true);

  if (!font.loadFromFile("font.otf"))
    throw std::runtime_error("Font could not be loaded!");
}

application& application::fit_view() {
  x_min = y_min = INFINITY;
  x_max = y_max = -INFINITY;
  for (size_t i = 0; i < x_data.size(); ++i) {
    x_min = std::min(x_min, x_data[i]);
    x_max = std::max(x_max, x_data[i]);
    y_min = std::min(y_min, y_data[i]);
    y_max = std::max(y_max, y_data[i]);
  }
  view_x_min = x_min - 0.2 * (x_max - x_min);
  view_x_max = x_max + 0.2 * (x_max - x_min);
  view_y_min = y_min - 0.2 * (y_max - y_min);
  view_y_max = y_max + 0.2 * (y_max - y_min);

  return *this;
}

application& application::fit_tiks() {
  float scales[] = {0.1f, 0.2f, 0.25f, 0.5f, 1.0f, 2.0f, 2.5f, 5.0f, 10.0f};

  const float x_tolerance = (plot_x_max - plot_x_min) / 60.0f;
  x_tics = std::exp(
      std::log(10.0f) *
      std::floor(std::log((view_x_max - view_x_min)) / std::log(10.0f)));
  for (int i = 0; i < 9; ++i) {
    const float new_x_tics = scales[i] * x_tics;
    if ((view_x_max - view_x_min) / new_x_tics < x_tolerance) {
      x_tics = new_x_tics;
      break;
    }
  }

  const float y_tolerance = (plot_y_max - plot_y_min) / 40.0f;
  y_tics = std::exp(
      std::log(10.0f) *
      std::floor(std::log((view_y_max - view_y_min)) / std::log(10.0f)));
  for (int i = 0; i < 9; ++i) {
    const float new_y_tics = scales[i] * y_tics;
    if ((view_x_max - view_x_min) / new_y_tics < y_tolerance) {
      y_tics = new_y_tics;
      break;
    }
  }

  return *this;
}

application& application::execute() {
  fit_view();

  while (window.isOpen()) {
    const auto mouse_pos = sf::Mouse::getPosition(window);
    mouse_x = mouse_pos.x;
    mouse_y = mouse_pos.y;
    mouse_diff_x = mouse_x - old_mouse_x;
    mouse_diff_y = mouse_y - old_mouse_y;

    process_events();

    if (update) {
      fit_tiks();
      window.clear(background_color);
      render();
      update = false;
    }
    // We have to draw every time, such that vsync will prevent full CPU usage.
    window.display();

    old_mouse_x = mouse_x;
    old_mouse_y = mouse_y;
  }
  return *this;
}

void application::process_events() {
  sf::Event event{};
  while (window.pollEvent(event)) {
    switch (event.type) {
      case sf::Event::Closed:
        window.close();
        break;

      case sf::Event::Resized:
        resize();
        break;

      case sf::Event::MouseWheelMoved: {
        auto scale_x = view_x_max - view_x_min;
        auto origin_x = 0.5f * (view_x_max + view_x_min);
        auto scale_y = view_y_max - view_y_min;
        auto origin_y = 0.5f * (view_y_max + view_y_min);
        const float wheel_scale = exp(-event.mouseWheel.delta * 0.05f);
        scale_x *= wheel_scale;
        scale_y *= wheel_scale;
        view_x_min = origin_x - 0.5f * scale_x;
        view_x_max = origin_x + 0.5f * scale_x;
        view_y_min = origin_y - 0.5f * scale_y;
        view_y_max = origin_y + 0.5f * scale_y;
        update = true;
      } break;

      case sf::Event::KeyPressed:
        switch (event.key.code) {
          case sf::Keyboard::Escape:
            window.close();
            break;
        }
        break;
    }
  }

  if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
    const auto move_x =
        (view_x_max - view_x_min) * mouse_diff_x / (plot_x_max - plot_x_min);
    const auto move_y =
        (view_y_max - view_y_min) * mouse_diff_y / (plot_y_max - plot_y_min);
    view_x_min -= move_x;
    view_x_max -= move_x;
    view_y_min += move_y;
    view_y_max += move_y;
    update = true;
  }
}

void application::draw_plot_background() {
  sf::RectangleShape plot_background;
  plot_background.setSize({plot_x_max - plot_x_min, plot_y_max - plot_y_min});
  plot_background.setPosition({plot_x_min, plot_y_min});
  plot_background.setFillColor(plot_background_color);
  window.draw(plot_background);
}

void application::draw_tiks() {
  int min_x_tic = std::ceil(view_x_min * (x_m_tics + 1) / x_tics);
  int max_x_tic = std::floor(view_x_max * (x_m_tics + 1) / x_tics);
  for (auto i = min_x_tic; i <= max_x_tic; ++i) {
    sf::RectangleShape tics_shape;
    tics_shape.setFillColor(point_color);

    float thickness = 1.0f;
    float length = 10.0f;
    const float m_thickness = 0.5f;
    const float m_length = 5.0f;

    const float x = i * x_tics / (x_m_tics + 1);
    const auto pixel_i = (x - view_x_min) / (view_x_max - view_x_min) *
                             (plot_x_max - plot_x_min) +
                         plot_x_min;

    if (std::abs(i) % (x_m_tics + 1)) {
      thickness = m_thickness;
      length = m_length;

      sf::RectangleShape gridline;
      gridline.setFillColor(m_gridlines_color);
      gridline.setSize({m_gridlines_size, plot_y_max - plot_y_min});
      gridline.setPosition({pixel_i - 0.5f * m_gridlines_size, plot_y_min});
      window.draw(gridline);

    } else {
      std::stringstream output{};
      output << std::fixed << std::setprecision(2) << x;

      sf::Text text;
      text.setFont(font);
      text.setString(output.str());
      text.setCharacterSize(11);
      text.setFillColor(sf::Color::Black);
      text.setStyle(sf::Text::Bold);
      text.setPosition(pixel_i - 0.5f * text.getLocalBounds().width,
                       plot_y_max + 2 * length);
      window.draw(text);
      // text.setPosition(pixel_i - 0.5f * text.getLocalBounds().width,
      //                  plot_y_min - 2 * length -
      //                  text.getLocalBounds().height);
      // window.draw(text);

      sf::RectangleShape gridline;
      gridline.setFillColor(gridlines_color);
      gridline.setSize({gridlines_size, plot_y_max - plot_y_min});
      gridline.setPosition({pixel_i - 0.5f * gridlines_size, plot_y_min});
      window.draw(gridline);
    }

    tics_shape.setPosition({pixel_i - 0.5f * thickness, plot_y_min});
    tics_shape.setSize({thickness, -length});
    window.draw(tics_shape);
    tics_shape.setPosition({pixel_i - 0.5f * thickness, plot_y_max});
    tics_shape.setSize({thickness, length});
    window.draw(tics_shape);
  }

  int min_y_tic = std::ceil(view_y_min * (y_m_tics + 1) / y_tics);
  int max_y_tic = std::floor(view_y_max * (y_m_tics + 1) / y_tics);
  for (auto i = min_y_tic; i <= max_y_tic; ++i) {
    sf::RectangleShape tics_shape;
    tics_shape.setFillColor(point_color);

    float thickness = 1.0f;
    float length = 10.0f;
    const float m_thickness = 0.5f;
    const float m_length = 5.0f;

    const float y = i * y_tics / (y_m_tics + 1);
    const auto pixel_j = (view_y_max - y) / (view_y_max - view_y_min) *
                             (plot_y_max - plot_y_min) +
                         plot_y_min;

    if (std::abs(i) % (y_m_tics + 1)) {
      thickness = m_thickness;
      length = m_length;

      sf::RectangleShape gridline;
      gridline.setFillColor(m_gridlines_color);
      gridline.setSize({plot_x_max - plot_x_min, m_gridlines_size});
      gridline.setPosition({plot_x_min, pixel_j - 0.5f * m_gridlines_size});
      window.draw(gridline);

    } else {
      std::stringstream output{};
      output << std::fixed << std::setprecision(2) << y;

      sf::Text text;
      text.setFont(font);
      text.setString(output.str());
      text.setCharacterSize(11);
      text.setFillColor(sf::Color::Black);
      text.setStyle(sf::Text::Bold);
      text.setPosition(plot_x_min - 2 * length - text.getLocalBounds().width,
                       pixel_j - text.getLocalBounds().height);
      window.draw(text);
      // text.setPosition(pixel_i - 0.5f * text.getLocalBounds().width,
      //                  plot_y_min - 2 * length -
      //                  text.getLocalBounds().height);
      // window.draw(text);

      sf::RectangleShape gridline;
      gridline.setFillColor(gridlines_color);
      gridline.setSize({plot_x_max - plot_x_min, gridlines_size});
      gridline.setPosition({plot_x_min, pixel_j - 0.5f * gridlines_size});
      window.draw(gridline);
    }

    tics_shape.setPosition({plot_x_min, pixel_j - 0.5f * thickness});
    tics_shape.setSize({-length, thickness});
    window.draw(tics_shape);
    tics_shape.setPosition({plot_x_max, pixel_j - 0.5f * thickness});
    tics_shape.setSize({length, thickness});
    window.draw(tics_shape);
  }
}

void application::draw_function() {
  texture.clear(sf::Color{0, 0, 0, 0});

  for (size_t i = 0; i < x_data.size() - 1; ++i) {
    const auto pixel_i = (x_data[i] - view_x_min) / (view_x_max - view_x_min) *
                         (plot_x_max - plot_x_min);
    const auto pixel_j = (view_y_max - y_data[i]) / (view_y_max - view_y_min) *
                         (plot_y_max - plot_y_min);

    const auto pixel_i1 = (x_data[i + 1] - view_x_min) /
                          (view_x_max - view_x_min) * (plot_x_max - plot_x_min);
    const auto pixel_j1 = (view_y_max - y_data[i + 1]) /
                          (view_y_max - view_y_min) * (plot_y_max - plot_y_min);

    sf::RectangleShape line;
    line.setSize({std::sqrt((pixel_i1 - pixel_i) * (pixel_i1 - pixel_i) +
                            (pixel_j1 - pixel_j) * (pixel_j1 - pixel_j)),
                  line_size});
    line.setOrigin(0, 0.5f * line_size);
    line.rotate(180.0f / M_PI *
                std::atan((pixel_j1 - pixel_j) / (pixel_i1 - pixel_i)));
    line.setFillColor(line_color);
    line.setPosition({pixel_i, pixel_j});
    texture.draw(line);

    sf::CircleShape dot{0.5f * line_size};
    dot.setOrigin(0.5f * line_size, 0.5f * line_size);
    dot.setPosition(pixel_i, pixel_j);
    dot.setFillColor(line_color);
    texture.draw(dot);
  }

  for (size_t i = 0; i < x_data.size(); ++i) {
    sf::CircleShape point_shape(point_size);
    point_shape.setFillColor(point_color);
    point_shape.setOrigin(point_size, point_size);

    const auto pixel_i = (x_data[i] - view_x_min) / (view_x_max - view_x_min) *
                         (plot_x_max - plot_x_min);

    const auto pixel_j = (view_y_max - y_data[i]) / (view_y_max - view_y_min) *
                         (plot_y_max - plot_y_min);

    point_shape.setPosition({pixel_i, pixel_j});
    texture.draw(point_shape);
  }

  texture.display();
  sf::Sprite sprite(texture.getTexture());
  sprite.setPosition(plot_x_min, plot_y_min);
  window.draw(sprite);
}

void application::draw_plot_border() {
  sf::RectangleShape plot_border;
  plot_border.setSize({plot_x_max - plot_x_min, plot_y_max - plot_y_min});
  plot_border.setPosition({plot_x_min, plot_y_min});
  plot_border.setFillColor(sf::Color{0, 0, 0, 0});
  plot_border.setOutlineThickness(plot_border_size);
  plot_border.setOutlineColor(plot_border_color);
  window.draw(plot_border);
}

void application::render() {
  draw_plot_background();
  draw_tiks();
  draw_function();
  draw_plot_border();
}

void application::resize() {
  window.setView(
      sf::View{sf::FloatRect{0, 0, static_cast<float>(window.getSize().x),
                             static_cast<float>(window.getSize().y)}});

  plot_x_min = plot_pad;
  plot_y_min = plot_pad;
  plot_x_max = window.getSize().x - plot_pad;
  plot_y_max = window.getSize().y - plot_pad;

  sf::ContextSettings settings;
  settings.antialiasingLevel = 8;
  texture.create(plot_x_max - plot_x_min, plot_y_max - plot_y_min, settings);
  texture.setSmooth(true);

  update = true;
}

}  // namespace plotter