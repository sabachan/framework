#ifndef AppUtils_FontCodeGenerator_H
#define AppUtils_FontCodeGenerator_H

#include <Core/Config.h>
#include <Reflection/BaseClass.h>
#include <Reflection/CommonTypes.h>

#if SG_ENABLE_TOOLS

namespace sg {
namespace image {
namespace tool {
//=============================================================================
class SymbolDictionary
{
public:
    void Add(std::string const& iName, size_t iCode);
    void Dump();
private:
    struct Symbol
    {
        std::string name;
        size_t code;
    };
    std::unordered_map<size_t, size_t> m_symbolFromCode;
    std::unordered_map<std::string, size_t> m_symbolFromName;
    std::vector<Symbol> m_symbols;
};
//=============================================================================
struct Glyph : public reflection::BaseType
{
    std::string character;
    std::string symbol;
    std::string representation;
    u32 advanceReduction;

    Glyph()
        : character()
        , representation()
        , advanceReduction()
    {}
    REFLECTION_TYPE_HEADER(Glyph, reflection::BaseType)
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
struct Kerning : public reflection::BaseType
{
    std::string character1;
    std::string character2;
    i32 value;

    Kerning()
        : character1()
        , character2()
        , value()
    {}
    REFLECTION_TYPE_HEADER(Kerning, reflection::BaseType)
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class Font : public reflection::BaseClass
{
    SG_NON_COPYABLE(Font)
    REFLECTION_CLASS_HEADER(Font, reflection::BaseClass)
public:
    Font();
    virtual ~Font() override;
public:
    void GenerateCode(size_t index, std::string& datacode, std::string& fontcode, SymbolDictionary& symbols, size_t& memoryfootprint) const;
private:
    std::string m_famillyName;
    bool m_bold;
    bool m_italic;
    u32 m_advance;
    u32 m_baseline;
    uint2 m_glyphSize;
    std::vector<Glyph> m_glyphs;
    std::vector<Kerning> m_kernings;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class FontCodeGenerator : public reflection::BaseClass
{
    SG_NON_COPYABLE(FontCodeGenerator)
    REFLECTION_CLASS_HEADER(FontCodeGenerator, reflection::BaseClass)
public:
    FontCodeGenerator();
    virtual ~FontCodeGenerator() override;
public:
    void Run() const;
private:
    std::vector<FilePath> m_fonts;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void GenerateFontCodes();
//=============================================================================
}
}
}
#endif

#endif
