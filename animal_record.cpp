// This is a generated file! Please edit source .ksy file and use kaitai-struct-compiler to rebuild

#include "animal_record.h"

animal_record_t::animal_record_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent, animal_record_t* p__root) : kaitai::kstruct(p__io) {
    m__parent = p__parent;
    m__root = this;

    try {
        _read();
    } catch(...) {
        _clean_up();
        throw;
    }
}

void animal_record_t::_read() {
    m_uuid = m__io->read_bytes(16);
    m_name = kaitai::kstream::bytes_to_str(m__io->read_bytes(24), std::string("UTF-8"));
    m_birth_year = m__io->read_u2be();
    m_weight = m__io->read_f8be();
    m_rating = m__io->read_s4be();
}

animal_record_t::~animal_record_t() {
    _clean_up();
}

void animal_record_t::_clean_up() {
}
