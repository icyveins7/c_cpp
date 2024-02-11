#ifndef ANIMAL_RECORD_H_
#define ANIMAL_RECORD_H_

// This is a generated file! Please edit source .ksy file and use kaitai-struct-compiler to rebuild

#include "kaitai/kaitaistruct.h"
#include <stdint.h>

#if KAITAI_STRUCT_VERSION < 9000L
#error "Incompatible Kaitai Struct C++/STL API: version 0.9 or later is required"
#endif

class animal_record_t : public kaitai::kstruct {

public:

    animal_record_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent = 0, animal_record_t* p__root = 0);

private:
    void _read();
    void _clean_up();

public:
    ~animal_record_t();

private:
    std::string m_uuid;
    std::string m_name;
    uint16_t m_birth_year;
    double m_weight;
    int32_t m_rating;
    animal_record_t* m__root;
    kaitai::kstruct* m__parent;

public:
    std::string uuid() const { return m_uuid; }
    std::string name() const { return m_name; }
    uint16_t birth_year() const { return m_birth_year; }
    double weight() const { return m_weight; }
    int32_t rating() const { return m_rating; }
    animal_record_t* _root() const { return m__root; }
    kaitai::kstruct* _parent() const { return m__parent; }
};

#endif  // ANIMAL_RECORD_H_
