#include "state.hpp"

void to_json(json& j, const Sample& v) {
    int expected_output;
    v.label.col(0).maxCoeff(&expected_output);
    j = json{
        { "expected_output", expected_output },
    };

    j["input"] = json::array();
    for (int i = 0; i < v.input.size(); ++i) {
        j["input"].push_back(static_cast<uint8_t>(std::round(v.input(i) * 255)));
    }
}

std::optional<Sample> State::get_test_sample(int index) const {
    if (index < 0 || index >= TEST_SIZE) {
        return std::nullopt;
    }

    auto input = test_inputs.mat().col(index);
    auto label = test_labels.mat().col(index);
    return Sample{ input, label };
}

std::optional<Sample> State::get_train_sample(int index) const {
    if (index < 0 || index >= TRAIN_SIZE) {
        return std::nullopt;
    }

    auto input = train_inputs.mat().col(index);
    auto label = train_labels.mat().col(index);
    return Sample{ input, label };
}
