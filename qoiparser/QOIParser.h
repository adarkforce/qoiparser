//
// Created by Antonio Della Fortuna on 14/04/2022.
//

#pragma  once
#include <cstdlib>
#include <memory>
#include <iostream>
#include <fstream>
#include <vector>

#include "stb_image.h"
#include "stb_image_write.h"

#include "Types.h"
#include "QOIBuffer.h"
#include "ImageBuffer.h"


namespace qoiparser {

    std::shared_ptr <QOIBuffer> encode(const std::shared_ptr <RGBABuffer> &image) {

        auto outBuffer = std::make_shared<QOIBuffer>(image->sizeInBytes);

        BUFFER_VIEWS::RGBA_T prevSeenPixels[64];

        memset(prevSeenPixels, 0, 64 * sizeof(BUFFER_VIEWS::RGBA_T));
        BUFFER_VIEWS::RGBA_T prev_pixel{0, 0, 0, 255};
        uint8_t runLength = 0;
        const auto w32 = static_cast<uint32_t>(image->width);
        const auto h32 = static_cast<uint32_t>(image->height);
        outBuffer->writeHeader(w32, h32, image->channels > 3, image->sRGB);

        bool wasQOI_OP_RUN = false;
        bool wasQOI_OP_INDEX = false;

        uint8_t prevIndex = -1;

        for (size_t i = 0; i < image->width * image->height; i++) {

            BUFFER_VIEWS::RGBA_T pixel = image->readPixel(static_cast<int>(i));

            uint8_t index_position = (pixel.r * 3 + pixel.g * 5 + pixel.b * 7 + pixel.a * 11) % 64;
            const BUFFER_VIEWS::RGBA_T &lookupItem = prevSeenPixels[index_position];

            const int32_t dr = static_cast<int32_t>(pixel.r) - prev_pixel.r;
            const int32_t dg = static_cast<int32_t>(pixel.g) - prev_pixel.g;
            const int32_t db = static_cast<int32_t>(pixel.b) - prev_pixel.b;

            const int32_t dr_dg = dr - dg;
            const int32_t db_dg = db - dg;

            const bool isQOI_OP_RUN = prev_pixel.r == pixel.r &&
                                      prev_pixel.g == pixel.g &&
                                      prev_pixel.b == pixel.b &&
                                      prev_pixel.a == pixel.a;

            const bool isQOI_OP_INDEX = lookupItem.r == pixel.r &&
                                        lookupItem.g == pixel.g &&
                                        lookupItem.b == pixel.b &&
                                        lookupItem.a == pixel.a;

            const bool isQOI_OP_DIFF = dr <= 1 && dr >= -2 &&
                                       dg <= 1 && dg >= -2 &&
                                       db <= 1 && db >= -2 &&
                                       (prev_pixel.a == pixel.a);

            const bool isQUOI_OP_LUMA = dg >= -32 && dg <= 31 &&
                                        dr_dg >= -8 && dr_dg <= 7 &&
                                        db_dg >= -8 && db_dg <= 7 &&
                                        (prev_pixel.a == pixel.a);

            if (isQOI_OP_RUN) {
                runLength++;
                wasQOI_OP_RUN = true;
                wasQOI_OP_INDEX = false;
                if (runLength == 62) {
                    outBuffer->writeQoiRun(runLength);
                    wasQOI_OP_INDEX = false;
                    wasQOI_OP_RUN = false;
                    runLength = 0;
                }
            } else {
                if (wasQOI_OP_RUN) {
                    outBuffer->writeQoiRun(runLength);
                    wasQOI_OP_INDEX = false;
                }
                if (isQOI_OP_INDEX && !(wasQOI_OP_INDEX && prevIndex == index_position)) {
                    outBuffer->writeQoiIndex(index_position);
                    prevIndex = index_position;
                    wasQOI_OP_INDEX = true;
                } else if (isQOI_OP_DIFF) {
                    outBuffer->writeQoiDiff(dr, dg, db);
                    wasQOI_OP_INDEX = false;
                } else if (isQUOI_OP_LUMA) {
                    outBuffer->writeQoiLuma(dg, dr_dg, db_dg);
                    wasQOI_OP_INDEX = false;
                } else {
                    if (image->channels > 3) {
                        outBuffer->writeQoiRGBA(pixel.r, pixel.g, pixel.b, pixel.a);
                    } else {
                        outBuffer->writeQoiRGB(pixel.r, pixel.g, pixel.b);
                    }
                    wasQOI_OP_INDEX = false;
                }
                wasQOI_OP_RUN = false;
                runLength = 0;
            }

            prevSeenPixels[index_position] = pixel;
            prev_pixel = pixel;
        }
        outBuffer->writeEnd();
        return outBuffer;
    }

    std::shared_ptr<QOIBuffer> encode(const std::string& inputFilePath) {
        int width;
        int height;
        int channels;
        uint8_t * data = stbi_load(inputFilePath.c_str(), &width, &height, &channels,0);
        auto rgbaBuffer = std::make_shared<RGBABuffer>(data, width,height,false,channels);
        stbi_image_free(data);
        return qoiparser::encode(rgbaBuffer);
    }

    std::shared_ptr <RGBABuffer> decode(const std::shared_ptr <QOIBuffer> &qoiBuffer) {

        auto header = qoiBuffer->readHeader();

        std::shared_ptr <RGBABuffer> rgbaBuffer = std::make_shared<RGBABuffer>(static_cast<uint32_t >(header->width),
                                                                               static_cast<uint32_t >(header->height),
                                                                               header->colorspace == 0,
                                                                               header->channels);
        BUFFER_VIEWS::RGBA_T prevSeenPixels[64];
        memset(prevSeenPixels, 0, 64 * sizeof(BUFFER_VIEWS::RGBA_T));
        BUFFER_VIEWS::RGBA_T prev_pixel{0, 0, 0, 255};

        while (!(qoiBuffer->readEnd())) {
            if (auto qoi_rgba = qoiBuffer->readRGBA()) {
                const auto px = qoi_rgba->rgba;
                rgbaBuffer->writePixel(px);
                prev_pixel = px;
                uint8_t index_position = (px.r * 3 + px.g * 5 + px.b * 7 + px.a * 11) % 64;
                prevSeenPixels[index_position] = px;
            } else if (auto qoi_rgb = qoiBuffer->readRGB()) {
                BUFFER_VIEWS::RGBA_T px{qoi_rgb->rgb.r, qoi_rgb->rgb.g, qoi_rgb->rgb.b, 255};
                rgbaBuffer->writePixel(px);
                prev_pixel = px;
                uint8_t index_position = (px.r * 3 + px.g * 5 + px.b * 7 + px.a * 11) % 64;
                prevSeenPixels[index_position] = px;
            } else if (auto qoi_run = qoiBuffer->readRun()) {
                const auto run = qoi_run->run;
                for (int i = 0; i < static_cast<int>(run) - RUN_LENGTH_BIAS; i++) {
                    rgbaBuffer->writePixel(prev_pixel);
                }
            } else if (auto qoi_index = qoiBuffer->readIndex()) {
                const auto px = prevSeenPixels[qoi_index->index];
                rgbaBuffer->writePixel(px);
                prev_pixel = px;
            } else if (auto qoi_diff = qoiBuffer->readDiff()) {
                auto r = prev_pixel.r + qoi_diff->dr - DIFF_BIAS;
                auto g = prev_pixel.g + qoi_diff->dg - DIFF_BIAS;
                auto b = prev_pixel.b + qoi_diff->db - DIFF_BIAS;
                const auto px = BUFFER_VIEWS::RGBA_T(r, g, b, prev_pixel.a);
                rgbaBuffer->writePixel(px);
                prev_pixel = px;
                uint8_t index_position = (px.r * 3 + px.g * 5 + px.b * 7 + px.a * 11) % 64;
                prevSeenPixels[index_position] = px;
            } else if (auto qoi_luma = qoiBuffer->readLuma()) {
                auto r = static_cast<uint8_t>(prev_pixel.r + qoi_luma->dr_dg + qoi_luma->diff_green -
                                              LUMA_BIAS_DR_DG_DB_DG - LUMA_BIAS_DIFF_GREEN);
                auto g = static_cast<uint8_t>(prev_pixel.g + qoi_luma->diff_green - LUMA_BIAS_DIFF_GREEN);
                auto b = static_cast<uint8_t>(prev_pixel.b + qoi_luma->db_dg + qoi_luma->diff_green -
                                              LUMA_BIAS_DR_DG_DB_DG - LUMA_BIAS_DIFF_GREEN);
                const auto px = BUFFER_VIEWS::RGBA_T(r, g, b, prev_pixel.a);
                rgbaBuffer->writePixel(px);
                uint8_t index_position = (px.r * 3 + px.g * 5 + px.b * 7 + px.a * 11) % 64;
                prevSeenPixels[index_position] = px;
                prev_pixel = px;
            } else {
                return nullptr;
            }
        }
        return rgbaBuffer;
    }

    std::shared_ptr <RGBABuffer> decode(const std::string& inputFilePath) {
        std::ifstream inFileStream(inputFilePath.c_str(), std::ios::in | std::ios::binary);
        std::vector<uint8_t> buffer(std::istreambuf_iterator<char>(inFileStream), {});
        auto qoiBuffer = std::make_shared<QOIBuffer>(buffer.size());
        qoiBuffer->write(buffer.data(), buffer.size());
        buffer.clear();
        buffer.shrink_to_fit();
        return decode(qoiBuffer);
    }

}