#pragma once

#include "decoder_cache.hpp"
#include <iostream>
#include <chrono>
#include <cstring>

#if 1
#define LOAD_EXP
#else
#define LOAD_MANUALLY
#endif

namespace riscv {
    void write_vector_to_file(const std::string &filename, const std::vector<uint8_t> &data);

    template<int W>
    static std::vector<std::uint8_t> serialize_cache_manually(DecoderCache<W> *&caches, int n) {
        std::vector<std::uint8_t> out;

        if (n == 0) {
            return out;
        }

        int size = n * caches[0].size() * sizeof(DecoderData<W>);
        out.reserve(size);

        for (int i = 0; i < n; i++) {
            auto decoder_cache = caches[i];
            for (const auto &d: decoder_cache.cache) {
                // 1) m_bytecode
                out.push_back(d.m_bytecode);

                // 2) m_handler
                out.push_back(d.m_handler);

                // 3) idxend + icount => each 8 bits
                //    We'll store idxend, then icount, in little-endian order
                out.push_back(static_cast<std::uint8_t>(d.idxend));
                out.push_back(static_cast<std::uint8_t>(d.icount));

                // 4) instr (32 bits, little-endian)
                out.push_back(static_cast<std::uint8_t>((d.instr >> 0) & 0xFF));
                out.push_back(static_cast<std::uint8_t>((d.instr >> 8) & 0xFF));
                out.push_back(static_cast<std::uint8_t>((d.instr >> 16) & 0xFF));
                out.push_back(static_cast<std::uint8_t>((d.instr >> 24) & 0xFF));
            }
        }

        return out;
    }

    template<int W>
    DecoderCache<W> *deserialize_decoder_cache_manually(const std::vector<std::uint8_t> &bytes, int n) {
        auto *cache = new DecoderCache<W>[n];

        const size_t num_entries = cache[0].size(); // N = PageSize / DIVISOR
        const size_t required_size = n * num_entries * sizeof(DecoderData<W>);

        if (bytes.size() != required_size) {
            throw std::runtime_error(
                    "deserialize_decoder_cache: invalid input size (expected "
                    + std::to_string(required_size)
                    + ", got "
                    + std::to_string(bytes.size())
                    + ")"
            );
        }

        // We read 8 bytes per entry
        size_t offset = 0;
        for (int p = 0; p < n; ++p) {
            for (size_t i = 0; i < num_entries; i++) {
                DecoderData<W> d;

                // 1) m_bytecode
                d.m_bytecode = bytes[offset + 0];

                // 2) m_handler
                d.m_handler = bytes[offset + 1];

                // 3) idxend + icount
                d.idxend = bytes[offset + 2];
                d.icount = bytes[offset + 3];

                // 4) instr (32 bits, little-endian)
                d.instr = static_cast<std::uint32_t>(bytes[offset + 4])
                          | (static_cast<std::uint32_t>(bytes[offset + 5]) << 8)
                          | (static_cast<std::uint32_t>(bytes[offset + 6]) << 16)
                          | (static_cast<std::uint32_t>(bytes[offset + 7]) << 24);

                // handle instr
                if (d.m_handler != 0) {
                    d.set_handler(CPU<W>::decode({d.instr}));
                }

                cache[p].cache[i] = d;
                offset += 8; // Move to next 8 bytes
            }
        }

        return cache;
    }

    template<int W>
    std::vector<std::uint8_t>
    serialize_decoder_cache_exp(const DecoderCache<W> &decoder_cache) {
        // We'll just do a raw copy of the entire array:
        const auto *data_ptr = reinterpret_cast<const std::uint8_t *>(decoder_cache.cache);
        const size_t num_bytes = decoder_cache.size() * sizeof(DecoderData<W>);

        std::vector<std::uint8_t> out;
        out.resize(num_bytes);
        std::memcpy(out.data(), data_ptr, num_bytes);

        return out;
    }


    template<int W>
    DecoderCache<W> deserialize_cache_item_exp(uint8_t *bytes) {
        DecoderCache<W> result;
//        const size_t expected_size = result.size() * sizeof(DecoderData<W>);
        result.cache = reinterpret_cast<DecoderData<W>*>(bytes);
//        std::memcpy(result.cache.data(), bytes, expected_size);
        return result;
    }

    template<int W>
    DecoderCache<W> *deserialize_decoder_cache(uint8_t* data, int n, int L) {
        auto t1 = std::chrono::high_resolution_clock::now();

#ifdef LOAD_EXP
        auto *cache = new DecoderCache<W>[n];

        const size_t num_entries = cache[0].size(); // N = PageSize / DIVISOR
        const size_t required_size = n * num_entries * sizeof(DecoderData<W>);

        if (L != required_size) {
            throw std::runtime_error(
                    "deserialize_decoder_cache: invalid input size (expected "
                    + std::to_string(required_size)
                    + ", got "
                    + std::to_string(L)
                    + ")"
            );
        }
        for (int i = 0; i < n; ++i) {
            cache[i] = deserialize_cache_item_exp<W>(data + (i * (num_entries * sizeof(DecoderData<W>))));
        }
#endif

#ifdef LOAD_MANUALLY
        auto *cache = deserialize_decoder_cache_manually<W>(data, n);
#endif

        auto t2 = std::chrono::high_resolution_clock::now();

        std::cout << "EXP LOADING took: " << (t2 - t1).count() << std::endl;
        return cache;
    }

    template<int W>
    static std::vector<std::uint8_t> serialize_whole_decoder_cache(DecoderCache<W> *&caches, int n) {
        std::vector<std::uint8_t> out;

        if (n == 0) {
            return out;
        }

        int size = n * caches[0].size() * sizeof(DecoderData<W>);
        out.reserve(size);


        for (int i = 0; i < n; i++) {
            auto decoder_cache = caches[i];
            auto v = serialize_decoder_cache_exp(decoder_cache);
            out.insert(out.end(), v.begin(), v.end());
        }

        return out;
    }
}
