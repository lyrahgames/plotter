#include <plotter/application.hpp>
#include <vector>
using namespace std;

int main() {
  // const auto x_min = -5.0f;
  // const auto x_max = 10.0f;
  // const size_t samples = 100;
  // vector<float> x_data(samples), y_data(samples);
  // for (size_t i = 0; i < samples; ++i) {
  //   const auto scale = static_cast<float>(i) / (samples - 1);
  //   const auto x = x_min * (1.0f - scale) + x_max * scale;
  //   const auto y = (abs(x) < 1e-5f) ? (2.0f) : (sin(2 * x) / x);
  //   x_data[i] = x;
  //   y_data[i] = y;
  // }

  plotter::application app{};
  app  //
       // .plot(begin(x_data), end(x_data), begin(y_data))
       // .plot([](float x) { return sin(2 * x) / x; }, -5.0f, 10.0f, 100)
      .plot([](float x) { return x * sin(2 * x) + sin(x); }, -10, 10, 1000)
      .execute();
}