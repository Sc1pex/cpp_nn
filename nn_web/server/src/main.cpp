#include <optional>
#include <string_view>
#include "app.hpp"

int main() {
    const char* assets_path = std::getenv("ASSETS_PATH");
    std::optional<std::string_view> assets_path_opt = std::nullopt;
    if (assets_path) {
        assets_path_opt = std::string_view{ assets_path };
    }

    App app(assets_path_opt);
    app.run();
}
