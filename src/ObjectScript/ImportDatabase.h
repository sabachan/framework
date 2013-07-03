#ifndef ObjectScript_ImportDatabase_H
#define ObjectScript_ImportDatabase_H

#include <Reflection/ObjectDatabase.h>
#include <Core/SmartPtr.h>
#include <string>
#include <unordered_map>
#include <unordered_set>


namespace sg {
class FilePath;
}

namespace sg {
namespace objectscript {
//=============================================================================
class IErrorHandler;
//=============================================================================
class Imported : public SafeCountable
{
    friend class ImportDatabase;
private:
    std::unordered_set<size_t> _;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class ImportDatabase : public SafeCountable
{
public:
    ImportDatabase();
    ~ImportDatabase();
    bool ImportROK(FilePath const& iFilePath, reflection::ObjectDatabase& ioScriptDatabase, Imported& iImported, IErrorHandler& iErrorHandler);
private:
    struct ImportEntry : public RefCountable
    {
        std::string filepath;
        reflection::ObjectDatabase database;
        std::vector<u32> imports;

        ImportEntry() : database(reflection::ObjectDatabase::ReferencingMode::BackwardReferenceOnly) {}
    };
private:
    std::vector<size_t> m_importStack;
    std::vector<refptr<ImportEntry>> m_imports;
    std::unordered_map<std::string, size_t> m_importFromFile;
};
//=============================================================================
}
}

#endif


