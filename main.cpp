#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "QOIParser.h"

#include <iostream>
#include <memory>

#include <argparse/argparse.hpp>
#include "utils.h"


int main(int argc, char *argv[]) {

    argparse::ArgumentParser program("qoiparser");

    program.add_argument("-i")
            .default_value(std::string("out.qoi"))
            .help("Input file path");

    program.add_argument("-o")
            .default_value(std::string("out.png"))
            .help("Output file path");

    try {
        program.parse_args(argc, argv);
    }
    catch (const std::runtime_error &err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        std::exit(1);
    }

    auto inputFilePath = program.get<std::string>("i");
    auto outputFilePath = program.get<std::string>("o");

    if(!std::filesystem::exists(inputFilePath)) {
        std::cerr << "Input file " << "\"" << inputFilePath << "\"" <<  " not found " << std::endl;
        std::exit(1);
    }

    std::ofstream outFileStream(outputFilePath.c_str(), std::ios::out | std::ios::binary);

    if (ends_with(inputFilePath, ".qoi")) {
        std::cout << "Decoding: " << inputFilePath << " ..." << std::endl;
        auto imageData = qoiparser::decode(inputFilePath);
        if(imageData) {
            stbi_write_png(outputFilePath.c_str(), imageData->width, imageData->height, imageData->channels,
                           (void *) imageData->data.data(), 0);
            std::cout << "Output file written to: " << outputFilePath << std::endl;
        } else {
            std::cerr << "Failed decoding file..." << std::endl;
        }
    } else if (ends_with(outputFilePath, ".qoi")) {
        std::cout << "Encoding: " << inputFilePath << " ..." << std::endl;
        auto outBuffer = qoiparser::encode(inputFilePath);
        if(outBuffer) {
            outFileStream.write((const char*)outBuffer->data.data(), outBuffer->data.size());
            std::cout << "Output file written to: " << outputFilePath << std::endl;
        } else {
            std::cerr << "Failed encoding file..." << std::endl;
        }
    }

    return 0;
}