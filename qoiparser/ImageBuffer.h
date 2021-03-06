//
// Created by Antonio Della Fortuna on 11/04/2022.
//
#pragma once
#include <cstdint>
#include <memory>
#include <vector>
#include <iostream>

#include "Types.h"

namespace qoiparser {
    class RGBABuffer {
    private:
        size_t bytesWritten;

    public:
        std::vector<uint8_t> data;
        size_t width;
        size_t height;
        bool sRGB;
        int channels;
        size_t sizeInBytes;

        RGBABuffer(size_t w, size_t h, bool _sRGB, int _channels) :
                bytesWritten(0), width(w), height(h), sRGB(_sRGB), channels(_channels) {
            sizeInBytes = width * height * channels;
            data.resize(sizeInBytes);
        }

        RGBABuffer(const uint8_t *image, size_t w, size_t h, bool _sRGB, int _channels) :
                bytesWritten(0), width(w), height(h), sRGB(_sRGB), channels(_channels) {
            sizeInBytes = width * height * channels;
            data.resize(sizeInBytes);
            writeImage(image);
        }

        void writeImage(const uint8_t *image) {
            memcpy(data.data(), image, sizeInBytes);
            bytesWritten += sizeInBytes;
        }

        void writePixel(const BUFFER_VIEWS::RGBA_T &pixel) {
            memcpy(&data[bytesWritten], &pixel, static_cast<size_t>(channels));
            bytesWritten += channels;
        }

        [[nodiscard]] BUFFER_VIEWS::RGBA_T readPixel(int n) const {
            BUFFER_VIEWS::RGBA_T pixel;
            if (channels > 3) {
                pixel = *reinterpret_cast<const BUFFER_VIEWS::RGBA_T *>(&data[n * sizeof(BUFFER_VIEWS::RGBA_T)]);
            } else {
                pixel = *reinterpret_cast<const BUFFER_VIEWS::RGBA_T *>(&data[n * sizeof(BUFFER_VIEWS::RGB_T)]);
                pixel.a = 255;
            }
            return pixel;
        }

    };

};
