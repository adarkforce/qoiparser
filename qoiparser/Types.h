//
// Created by Antonio Della Fortuna on 09/04/2022.
//
#pragma once
#include <cstdint>

namespace qoiparser {

    constexpr uint8_t RGB_TAG = 0b11111110;
    constexpr uint8_t RGBA_TAG = 0b11111111;
    constexpr uint8_t INDEX_TAG = 0b00;
    constexpr uint8_t LUMA_TAG = 0b10;
    constexpr uint8_t DIFF_TAG = 0b01;
    constexpr uint8_t RUN_TAG = 0b11;
    constexpr uint64_t STREAM_END = 0x0000000000000001;

    namespace BUFFER_VIEWS {

        struct QOI_HEADER {
            const char magic[4]{'q', 'o', 'i', 'f'};      // magic bytes "qoif"
            uint32_t width{};     // image width in pixels (BE)
            uint32_t height{};    // image height in pixels (BE)
            uint8_t channels{};   // 3 = RGB, 4 = RGBA
            uint8_t colorspace{}; // 0 = sRGB with linear alpha 1 = all channels linear
        };


        struct RGB_T {
            uint8_t r;
            uint8_t g;
            uint8_t b;

            RGB_T() : r(0), g(0), b(0) {};

            RGB_T(uint8_t _r, uint8_t _g, uint8_t _b) :
                    r(_r), g(_g), b(_b) {};
        };

        struct RGBA_T {
            uint8_t r;
            uint8_t g;
            uint8_t b;
            uint8_t a;

            RGBA_T() : r(0), g(0), b(0), a(255) {};

            RGBA_T(uint8_t _r, uint8_t _g, uint8_t _b, uint8_t _a) :
                    r(_r), g(_g), b(_b), a(_a) {};
        };

        struct QOI_OP_RGB {
            uint8_t tag;
            RGB_T rgb;

            QOI_OP_RGB() : tag(RGB_TAG), rgb() {};
        };

        struct QOI_OP_RGBA {
            const uint8_t tag;
            RGBA_T rgba;

            QOI_OP_RGBA() : tag(RGBA_TAG), rgba() {};
        };


        struct QOI_OP_INDEX {
            uint8_t index: 6;
            uint8_t tag: 2;

            QOI_OP_INDEX() : index(0), tag(INDEX_TAG) {};
        };

        struct QOI_OP_DIFF {
            uint8_t db: 2, dg: 2, dr: 2;
            uint8_t tag: 2;

            QOI_OP_DIFF() : db(0), dg(0), dr(0), tag(DIFF_TAG) {};
        };

        struct QOI_OP_LUMA {
            uint8_t diff_green: 6;
            uint8_t tag: 2;
            uint8_t db_dg: 4, dr_dg: 4;

            QOI_OP_LUMA() : diff_green(0), tag(LUMA_TAG), db_dg(0), dr_dg(0) {};
        };

        struct QOI_OP_RUN {
            uint8_t run: 6;
            uint8_t tag: 2;

            QOI_OP_RUN() : run(0), tag(RUN_TAG) {};
        };

        struct QOI_STREAM_END {
            const uint64_t end = STREAM_END;
        };

    }
    constexpr int RUN_LENGTH_BIAS = -1;
    constexpr uint8_t LUMA_BIAS_DIFF_GREEN = 32;
    constexpr uint8_t LUMA_BIAS_DR_DG_DB_DG = 8;
    constexpr uint8_t DIFF_BIAS = 2;


};