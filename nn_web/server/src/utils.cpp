#include "utils.hpp"

std::expected<int, ApiResponse> parse_int_param(
    const httc::Request& req, std::string_view param_name, std::string_view error_msg
) {
    int param;
    try {
        auto param_str = req.path_params.at(std::string(param_name));
        param = std::stoi(param_str);
    } catch (const std::exception&) {
        return std::unexpected(ApiResponse::bad_request(error_msg));
    }
    return param;
}

std::expected<Sample, ApiResponse>
    load_sample(const State& state, std::string_view source, int idx) {
    std::optional<Sample> sample;
    if (source == "test") {
        sample = state.get_test_sample(idx);
    } else if (source == "train") {
        sample = state.get_train_sample(idx);
    } else {
        return std::unexpected(ApiResponse::bad_request("Invalid source"));
    }

    if (!sample) {
        return std::unexpected(ApiResponse::not_found("Training sample not found"));
    }
    return *sample;
}

asio::awaitable<std::expected<std::pair<NetworkFull, nn::Network>, ApiResponse>>
    load_network(Db& db, int network_id) {
    auto network_res = co_await db.get_full_network_by_id(network_id);
    if (!network_res) {
        co_return std::unexpected(map_db_error(
            network_res.error(), "Failed to retrieve network",
            "Failed to retrieve network with id {}", network_id
        ));
    }
    if (!network_res.value()) {
        co_return std::unexpected(ApiResponse::not_found("Network not found"));
    }
    auto network_info = network_res.value().value();

    auto hidden_activations_view =
        network_info.activations | std::views::take(network_info.activations.size() - 1);
    auto hidden_activations = nn::strs_to_hidden_activation(hidden_activations_view).value();
    auto output_activation = nn::str_to_output_activation(network_info.activations.back()).value();
    auto loss = nn::str_to_loss(network_info.loss).value();

    auto network_opt = nn::Network::from_data(
        network_info.layer_sizes, network_info.weights, network_info.biases, hidden_activations,
        output_activation, loss
    );
    if (!network_opt) {
        co_return std::unexpected(ApiResponse::internal_error("Failed to parse network from db"));
    }

    co_return std::make_pair(network_info, *network_opt);
}
