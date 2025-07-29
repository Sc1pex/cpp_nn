#include <cstdlib>
#include <iostream>
#include <print>
#include <vector>
#include "idx.h"

void print_block(double v) {
    v *= 24.;
    int c = (int) v + 232;
    if (c > 255) {
        c = 255;
    }
    std::print("\x1b[38;5;{}m██\x1b[0m", c);
}

void print_random_digit(const std::vector<Matrix>& digits, int digit) {
    std::println("Label: {}", digit);
    int idx = rand() % digits.size();
    for (int r = 0; r < digits[idx].rows(); r++) {
        for (int c = 0; c < digits[idx].cols(); c++) {
            print_block(digits[idx].element(r, c));
        }
        std::println();
    }
}

int main() {
    auto data1 = idx3_read_file("data/train-images.idx3-ubyte");
    auto data2 = idx3_read_file("data/t10k-images.idx3-ubyte");
    auto label1 = idx1_read_file("data/train-labels.idx1-ubyte");
    auto label2 = idx1_read_file("data/t10k-labels.idx1-ubyte");

    std::unordered_map<int, std::vector<Matrix>> data;
    for (int i = 0; i <= 9; i++) {
        data.emplace(std::make_pair(i, std::vector<Matrix>()));
    }
    for (int i = 0; i < data1.size(); i++) {
        data[label1[i]].push_back(std::move(data1[i]));
    }
    for (int i = 0; i < data2.size(); i++) {
        data[label2[i]].push_back(std::move(data2[i]));
    }
    srand(time(nullptr));

    std::println("(0-9): random image of chosen digit\nEmpty line: random image of random digit");
    std::string line;
    while (1) {
        std::getline(std::cin, line);

        int digit;
        if (line.length() == 0) {
            digit = rand() % 10;
        } else if ('0' <= line[0] && line[0] <= '9' && line.length() == 1) {
            digit = line[0] - '0';
        } else {
            std::println(
                "(0-9): random image of chosen digit\nEmpty line: random image of random digit"
            );
            continue;
        }

        print_random_digit(data[digit], digit);
    }
}
