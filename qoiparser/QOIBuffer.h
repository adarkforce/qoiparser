//
// Created by Antonio Della Fortuna on 11/04/2022.
//
#pragma once
#include <cstdint>
#include <memory>

#include "Types.h"

namespace qoiparser {


    class QOIBuffer {
    private:
        size_t maxSize;



        void write_32(const uint32_t &v) {

            uint8_t v1 = (0xff000000 & v) >> 24;
            uint8_t v2 = (0x00ff0000 & v) >> 16;
            uint8_t v3 = (0x0000ff00 & v) >> 8;
            uint8_t v4 = (0x000000ff & v);

            write(&v1, sizeof(uint8_t));
            write(&v2, sizeof(uint8_t));
            write(&v3, sizeof(uint8_t));
            write(&v4, sizeof(uint8_t));

        }

        static uint32_t swapEndian(uint32_t val) {

            uint32_t res =
                    ((val & 0x000000FF) << 24) |
                    ((val & 0x0000FF00) << 8) |
                    ((val & 0x00FF0000) >> 8) |
                    ((val & 0xFF000000) >> 24);

            return res;
        }

    public:
        size_t bytesRead = 0;

        uint8_t *data;
        size_t size;

        explicit QOIBuffer(size_t _size = 0): maxSize(_size), size(0) {
            data = (uint8_t *) malloc(_size);
        }

        ~QOIBuffer() {
            delete data;
        }

        void write(void *source, size_t itemSize) {
            if (size + itemSize > maxSize) {
                data = (uint8_t *) realloc(data, size + itemSize);
            }
            memcpy(data + size, source, itemSize);
            size += itemSize;
        }

        void writeHeader(const uint32_t& width, const uint32_t& height, bool hasAlpha, bool sRGB) {
            BUFFER_VIEWS::QOI_HEADER header;
            header.width = static_cast<uint32_t>(width);
            header.height = static_cast<uint32_t>(height);
            header.channels = static_cast<uint8_t>(hasAlpha ? 4 : 3);
            header.colorspace = static_cast<uint8_t>(sRGB ? 0 : 1);
            write((void *) &header.magic, sizeof(char) * 4);
            write_32(header.width);
            write_32(header.height);
            write(&header.channels, sizeof(uint8_t));
            write(&header.colorspace, sizeof(uint8_t));
        }

        void writeQoiRun(const uint8_t& runLength) {
            BUFFER_VIEWS::QOI_OP_RUN qoi_op_run;
            qoi_op_run.run = runLength + RUN_LENGTH_BIAS;
            write(&qoi_op_run, sizeof(BUFFER_VIEWS::QOI_OP_RUN));
        }

        void writeQoiIndex(const uint8_t& index) {
            BUFFER_VIEWS::QOI_OP_INDEX qoi_op_index;
            qoi_op_index.index = index;
            write(&qoi_op_index, sizeof(BUFFER_VIEWS::QOI_OP_INDEX));
        }

        void writeQoiDiff(const int32_t& dr,const int32_t& dg,const int32_t& db) {
            BUFFER_VIEWS::QOI_OP_DIFF qoi_op_diff;
            qoi_op_diff.dr = static_cast<uint8_t>(dr + DIFF_BIAS);
            qoi_op_diff.dg = static_cast<uint8_t>(dg + DIFF_BIAS);
            qoi_op_diff.db = static_cast<uint8_t>(db + DIFF_BIAS);
            write(&qoi_op_diff, sizeof(BUFFER_VIEWS::QOI_OP_DIFF));
        }

        void writeQoiLuma(const int32_t& dg, const int32_t& dr_dg, const int32_t& db_dg) {
            BUFFER_VIEWS::QOI_OP_LUMA qoi_op_luma;
            qoi_op_luma.diff_green = static_cast<uint8_t>(dg + LUMA_BIAS_DIFF_GREEN);
            qoi_op_luma.dr_dg = static_cast<uint8_t>(dr_dg + LUMA_BIAS_DR_DG_DB_DG);
            qoi_op_luma.db_dg = static_cast<uint8_t>(db_dg + LUMA_BIAS_DR_DG_DB_DG);
            write(&qoi_op_luma, sizeof(BUFFER_VIEWS::QOI_OP_LUMA));
        }

        void writeQoiRGBA(const uint8_t& r, const uint8_t& g, const uint8_t& b, const uint8_t& a) {
            BUFFER_VIEWS::QOI_OP_RGBA qoi_op_rgba;
            qoi_op_rgba.rgba.r = r;
            qoi_op_rgba.rgba.g = g;
            qoi_op_rgba.rgba.b = b;
            qoi_op_rgba.rgba.a = a;
            write(&qoi_op_rgba, sizeof(BUFFER_VIEWS::QOI_OP_RGBA));
        }

        void writeQoiRGB(const uint8_t& r, const uint8_t& g, const uint8_t& b) {
            BUFFER_VIEWS::QOI_OP_RGB qoi_op_rgb;
            qoi_op_rgb.rgb.r = r;
            qoi_op_rgb.rgb.g = g;
            qoi_op_rgb.rgb.b = b;
            write(&qoi_op_rgb, sizeof(BUFFER_VIEWS::QOI_OP_RGB));
        }

        void writeEnd() {
            BUFFER_VIEWS::QOI_STREAM_END end;
            write(&end, sizeof(BUFFER_VIEWS::QOI_STREAM_END));
            data = (uint8_t *) realloc(data, size);
        }

        const BUFFER_VIEWS::QOI_HEADER* readHeader() {
            auto h = reinterpret_cast<BUFFER_VIEWS::QOI_HEADER*>(data);
            if( h->magic[0] == 'q' &&
                h->magic[1] == 'o' &&
                h->magic[2] == 'i' &&
                h->magic[3] == 'f') {
                h->width = swapEndian(h->width);
                h->height = swapEndian(h->height);
                bytesRead += 14;
                return h;
            }
            return nullptr;
        }

        const BUFFER_VIEWS::QOI_OP_RGBA* readRGBA() {
            auto qoi_rgba = reinterpret_cast<BUFFER_VIEWS::QOI_OP_RGBA*>(data + bytesRead);
            if(qoi_rgba->tag == RGBA_TAG) {
                bytesRead += sizeof (BUFFER_VIEWS::QOI_OP_RGBA);
                return qoi_rgba;
            } else {
                return nullptr;
            }
        }

        const BUFFER_VIEWS::QOI_OP_RGB* readRGB() {
            auto qoi_rgb = reinterpret_cast<BUFFER_VIEWS::QOI_OP_RGB*>(data + bytesRead);
            if(qoi_rgb->tag == RGB_TAG) {
                bytesRead += sizeof(BUFFER_VIEWS::QOI_OP_RGB);
                return qoi_rgb;
            } else {
                return nullptr;
            }
        }

        const BUFFER_VIEWS::QOI_OP_INDEX* readIndex() {
            auto qoi_index = reinterpret_cast<BUFFER_VIEWS::QOI_OP_INDEX*>(data + bytesRead);
            if(qoi_index->tag == INDEX_TAG) {
                bytesRead += sizeof (BUFFER_VIEWS::QOI_OP_INDEX);
                return qoi_index;
            } else {
                return nullptr;
            }
        }

        const BUFFER_VIEWS::QOI_OP_DIFF* readDiff() {
            auto qoi_diff = reinterpret_cast<BUFFER_VIEWS::QOI_OP_DIFF*>(data + bytesRead);
            if(qoi_diff->tag == DIFF_TAG) {
                bytesRead += sizeof (BUFFER_VIEWS::QOI_OP_DIFF);
                return qoi_diff;
            } else {
                return nullptr;
            }
        }

        const BUFFER_VIEWS::QOI_OP_LUMA* readLuma() {
            auto qoi_luma = reinterpret_cast<BUFFER_VIEWS::QOI_OP_LUMA*>(data + bytesRead);
            if(qoi_luma->tag == LUMA_TAG) {
                bytesRead += sizeof (BUFFER_VIEWS::QOI_OP_LUMA);
                return qoi_luma;
            } else {
                return nullptr;
            }
        }

        const BUFFER_VIEWS::QOI_OP_RUN* readRun() {
            auto qoi_run = reinterpret_cast<BUFFER_VIEWS::QOI_OP_RUN*>(data + bytesRead);
            if(qoi_run->tag == RUN_TAG) {
                bytesRead += sizeof(BUFFER_VIEWS::QOI_OP_RUN);
                return qoi_run;
            } else {
                return nullptr;
            }
        }

        const uint64_t* readEnd() {
            auto end = reinterpret_cast<uint64_t*>(data + bytesRead);
            if ( *end == STREAM_END ) {
                bytesRead += sizeof(uint64_t);
                return end;
            } else {
                return nullptr;
            }
        }

    };
};

