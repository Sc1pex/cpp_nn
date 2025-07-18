module;

#include <algorithm>
#include <ranges>
#include <vector>

export module nn:nn;

import :layer;
import :matrix;

export class NN {
public:
    template<std::ranges::range T>
    NN(T shape);

    // Feed forward a column matrix
    Matrix feed_forward(Matrix input);

private:
    std::vector<Layer> m_layers;
};

Matrix NN::feed_forward(Matrix input) {
    return std::ranges::fold_left(
        m_layers, std::move(input), [](auto m, auto& l) {
            return l.feed_forward(std::move(m));
        }
    );
}

template<std::ranges::range T>
NN::NN(T shape) {
    auto layers =
        shape | std::views::slide(2) | std::views::transform([](auto r) {
            return Layer::random(r[0], r[1]);
        });
    m_layers = std::ranges::to<std::vector<Layer>>(layers);
}
