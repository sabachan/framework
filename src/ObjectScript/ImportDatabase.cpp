#include "stdafx.h"

#include "ImportDatabase.h"

#include "Reader.h"
#include <Core/FilePath.h>
#include <Core/WinUtils.h>
#include <Reflection/BaseClass.h>

namespace sg {
namespace objectscript {
//=============================================================================
ImportDatabase::ImportDatabase()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
ImportDatabase::~ImportDatabase()
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
bool ImportDatabase::ImportROK(FilePath const& iFilePath, reflection::ObjectDatabase& ioScriptDatabase, Imported& iImported, IErrorHandler& iErrorHandler)
{
    std::string filepath;
    winutils::GetUniqueSystemFilePath_AssumeExists(filepath, iFilePath.GetSystemFilePath());

    auto const f = m_importFromFile.find(filepath);
    bool alreadyRead = f != m_importFromFile.end();
    size_t const importIdx = alreadyRead ? f->second : m_imports.size();
    if(!alreadyRead)
    {
        m_importFromFile.insert(std::make_pair(filepath, importIdx));
        m_imports.emplace_back(new ImportEntry);
        ImportEntry* back = m_imports.back().get();
        back->filepath = filepath;

        m_importStack.push_back(importIdx);
        bool ok = ReadImportROK(iFilePath, back->database, *this, iErrorHandler);
        SG_ASSERT(m_importStack.back() == importIdx);
        m_importStack.pop_back();

        if(!ok)
            return false;
    }
    auto r = iImported._.insert(importIdx);
    bool const alreadyImported = !r.second;
    if(!alreadyImported)
    {
        ImportEntry* entry = m_imports[importIdx].get();
        reflection::ObjectDatabase::named_object_list objects;
        entry->database.GetLastTransactionObjects(objects);
        ioScriptDatabase.BeginTransaction();
        for(auto const& it : objects)
        {
            // NB: imported script objects are made public to be available.
            ioScriptDatabase.Add(reflection::ObjectVisibility::Public, it.first, it.second.get());
        }
        ioScriptDatabase.EndTransaction();
        if(!m_importStack.empty())
        {
            size_t const stackBackIdx = m_importStack.back();
            ImportEntry* stackBack = m_imports[stackBackIdx].get();
            stackBack->imports.push_back(checked_numcastable(importIdx));
        }
    }

    return true;
}
//=============================================================================
}
}
