#ifndef Core_SimpleFileReader_H
#define Core_SimpleFileReader_H

#include "FilePath.h"

namespace sg {
//=============================================================================
class SimpleFileReader
{
public:
    SimpleFileReader(FilePath const& iFilename);
    ~SimpleFileReader();
    bool IsValid() const { return m_isValid; }
    void const* data() const { return m_data; }
    size_t size() const { return m_size; }
private:
    void* m_data;
    size_t m_size;
    bool m_isValid;
};
//=============================================================================
}

#endif
