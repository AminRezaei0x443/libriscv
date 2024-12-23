#include "decoder_cache_serialize.hpp"
#include <iostream>
#include <fstream>

namespace riscv {
    void write_vector_to_file(const std::string &filename, const std::vector<uint8_t> &data) {
        std::ofstream ofs(filename, std::ios::binary);
        if (!ofs) {
            throw std::runtime_error("Failed to open file for writing: " + filename);
        }

        if (!data.empty()) {
            ofs.write(reinterpret_cast<const char *>(data.data()), static_cast<std::streamsize>(data.size()));
            if (!ofs) {
                throw std::runtime_error("Failed to write data to file: " + filename);
            }
        }
    }
//    template <int W>
//    static std::vector<std::uint8_t> serialize_decoder_cache(const DecoderCache<W>& decoder_cache)
//    {
//        std::vector<std::uint8_t> out;
//        out.reserve(decoder_cache.cache.size() * sizeof(DecoderData<W>));
//
//        for (const auto& d : decoder_cache.cache)
//        {
//            // 1) m_bytecode
//            out.push_back(d.m_bytecode);
//
//            // 2) m_handler
//            out.push_back(d.m_handler);
//
//            // 3) idxend + icount => each 8 bits
//            //    We'll store idxend, then icount, in little-endian order
//            out.push_back(static_cast<std::uint8_t>(d.idxend));
//            out.push_back(static_cast<std::uint8_t>(d.icount));
//
//            // 4) instr (32 bits, little-endian)
//            out.push_back(static_cast<std::uint8_t>((d.instr >>  0) & 0xFF));
//            out.push_back(static_cast<std::uint8_t>((d.instr >>  8) & 0xFF));
//            out.push_back(static_cast<std::uint8_t>((d.instr >> 16) & 0xFF));
//            out.push_back(static_cast<std::uint8_t>((d.instr >> 24) & 0xFF));
//        }
//
//        return out;
//    }
} // riscv
