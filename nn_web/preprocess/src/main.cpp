#include "cli.hpp"
#include "spdlog/spdlog.h"

int main(int argc, char** argv) {
    CliArgs args = CliArgs::parse(argc, argv);

    spdlog::info("Train inputs: {} -> {}", args.train_inputs_in(), args.train_inputs_out());
}
