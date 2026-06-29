#include "cli.hpp"
#include <print>

void print_usage_and_exit() {
    std::println("Usage: preprocess [--input <input_dir>] [--output <output_dir>]");
    std::println(
        "  --input <input_dir>   Directory containing the input IDX files (default: idx_data)"
    );
    std::println(
        "  --output <output_dir> Directory to write the output binary files (default: mnist_data)"
    );
    std::exit(1);
}

CliArgs CliArgs::parse(int argc, char** argv) {
    CliArgs args;
    args.input_dir = "idx_data";
    args.output_dir = "mnist_data";

    for (int i = 1; i < argc; i++) {
        if (std::string_view{ argv[i] } == "--input") {
            if (i == argc - 1) {
                print_usage_and_exit();
            }
            args.input_dir = argv[i + 1];
            i++;
            continue;
        } else if (std::string_view{ argv[i] } == "--output") {
            if (i == argc - 1) {
                print_usage_and_exit();
            }
            args.output_dir = argv[i + 1];
            i++;
            continue;
        } else {
            print_usage_and_exit();
        }
    }

    return args;
}
