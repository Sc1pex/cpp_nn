#include <filesystem>
#include <fstream>
#include "cli.hpp"
#include "idx.hpp"
#include "spdlog/spdlog.h"

void process_label(const std::string& in_path, const std::string& out_path) {
    spdlog::info("Processing labels: {} -> {}", in_path, out_path);

    auto res = idx1_read_file(in_path);
    if (!res.has_value()) {
        auto err = res.error();
        spdlog::error("Failed to read IDX1 file {}: {}", in_path, err);
        std::exit(1);
    }

    auto label_inputs = res.value();
    MatrixXd labels = MatrixXd::Zero(10, label_inputs.size());
    for (int i = 0; i < label_inputs.size(); i++) {
        int label = label_inputs[i];
        if (label < 0 || label >= 10) {
            spdlog::error("Invalid label value {} at index {}", label, i);
            std::exit(1);
        }
        labels(label, i) = 1.0;
    }

    std::ofstream out(out_path, std::ios::binary);
    out.write(reinterpret_cast<const char*>(labels.data()), labels.size() * sizeof(double));
}

void process_inputs(const std::string& in_path, const std::string& out_path) {
    spdlog::info("Processing inputs: {} -> {}", in_path, out_path);

    auto res = idx3_read_file(in_path);
    if (!res.has_value()) {
        auto err = res.error();
        spdlog::error("Failed to read IDX3 file {}: {}", in_path, err);
        std::exit(1);
    }

    auto input_images = res.value();
    int num_images = input_images.size();
    int image_size = input_images[0].size();

    MatrixXd inputs(image_size, num_images);
    for (int i = 0; i < num_images; i++) {
        inputs.col(i) = input_images[i].reshaped<Eigen::RowMajor>().cast<double>() / 255.0;
    }

    std::ofstream out(out_path, std::ios::binary);
    out.write(reinterpret_cast<const char*>(inputs.data()), inputs.size() * sizeof(double));
}

int main(int argc, char** argv) {
    CliArgs args = CliArgs::parse(argc, argv);

    std::filesystem::create_directories(args.output_dir);

    process_inputs(args.train_inputs_in(), args.train_inputs_out());
    process_label(args.train_labels_in(), args.train_labels_out());
    process_inputs(args.test_inputs_in(), args.test_inputs_out());
    process_label(args.test_labels_in(), args.test_labels_out());

    spdlog::info("Preprocessing completed successfully.");
}
