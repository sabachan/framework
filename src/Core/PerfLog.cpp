#include "stdafx.h"

#include "PerfLog.h"

#if SG_ENABLE_PERF_LOG

#include "Cast.h"
#include "Log.h"
#include "Platform.h"
#include "StringFormat.h"
#include "TestFramework.h"
#include <algorithm>
#include <memory>
#include <sstream>
#include <unordered_map>

#if SG_PLATFORM_IS_WIN
#include <Core/WindowsH.h>
#endif

namespace sg {
//=============================================================================
SimpleCPUPerfLog::SimpleCPUPerfLog(char const* iName, u64* ioBest)
: m_name(iName)
, m_best(ioBest)
{
    LARGE_INTEGER t;
    BOOL rc = QueryPerformanceCounter(&t);
    SG_ASSERT_AND_UNUSED(rc);
    m_begin = t.QuadPart;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
SimpleCPUPerfLog::~SimpleCPUPerfLog()
{
    LARGE_INTEGER t;
    BOOL rc = QueryPerformanceCounter(&t);
    SG_ASSERT(rc);
    LARGE_INTEGER freq;
    rc = QueryPerformanceFrequency(&freq);
    std::ostringstream oss;
    u64 const delta = t.QuadPart - m_begin;
    oss << m_name << ": " << 1000000 * delta / freq.QuadPart << "us";
    if(delta < *m_best)
        *m_best = delta;
    oss << " (best: " << 1000000 * *m_best / freq.QuadPart << "us)";
    SG_LOG_INFO("Perf", oss.str().c_str());
}
//=============================================================================
namespace {
    struct PerfLog
    {
        PerfLogParameters param;
        std::unordered_multimap<size_t, refptr<PerfLogItem> > items;
        std::vector<PerfLogItemOccurence> frame;
        std::vector<PerfLogItemOccurence> previousFrame;
    };
    PerfLog* g_perfLog = nullptr;
}
//=============================================================================
namespace perflog {
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Init()
{
    SG_ASSERT(nullptr == g_perfLog);
    g_perfLog = new PerfLog();
    Clear();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Shutdown()
{
    SG_ASSERT(nullptr != g_perfLog);
    delete g_perfLog;
    g_perfLog = nullptr;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
u64 TicksPerSecond()
{
    LARGE_INTEGER freq;
    BOOL rc = QueryPerformanceFrequency(&freq);
    SG_ASSERT_AND_UNUSED(rc);
    return freq.QuadPart;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Clear()
{
    SG_ASSERT(nullptr != g_perfLog);
    PerfLog& perfLog = *g_perfLog;
    perfLog.previousFrame.clear();
    perfLog.frame.clear();
    perfLog.items.clear();

    LARGE_INTEGER t;
    BOOL const rc = QueryPerformanceCounter(&t);
    SG_ASSERT_AND_UNUSED(rc);
    u64 const date = t.QuadPart;
    perfLog.frame.emplace_back();

    PerfLogItemOccurence& occ = perfLog.frame[0];
    occ.begin = date;
    occ.end = all_ones;
    occ.item = nullptr;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void OnNewFrame()
{
    SG_ASSERT(nullptr != g_perfLog);
    PerfLog& perfLog = *g_perfLog;

    LARGE_INTEGER t;
    BOOL const rc = QueryPerformanceCounter(&t);
    SG_ASSERT_AND_UNUSED(rc);
    u64 const date = t.QuadPart;
    {
        SG_ASSERT(!perfLog.frame.empty());
        PerfLogItemOccurence& occ = perfLog.frame[0];
        SG_ASSERT(all_ones == occ.end);
        occ.end = date;
        SG_ASSERT(nullptr == occ.item);
    }
    swap(perfLog.previousFrame, perfLog.frame);
    perfLog.frame.clear();
    {
        perfLog.frame.emplace_back();
        PerfLogItemOccurence& occ = perfLog.frame[0];
        occ.begin = date;
        occ.end = all_ones;
        occ.item = nullptr;
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
ArrayView<PerfLogItemOccurence const> GetPreviousFrame()
{
    SG_ASSERT(nullptr != g_perfLog);
    PerfLog& perfLog = *g_perfLog;
    return ArrayView<PerfLogItemOccurence const>(perfLog.previousFrame.data(), perfLog.previousFrame.size());
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void GetPreviousFrameCopy(std::vector<PerfLogItemOccurence>& oFrame)
{
    SG_ASSERT(nullptr != g_perfLog);
    PerfLog& perfLog = *g_perfLog;
    std::vector<PerfLogItemOccurence> frame;
    auto const begin = perfLog.previousFrame.begin();
    auto const end = perfLog.previousFrame.end();
    for(auto it = begin; it != end; ++it)
    {
        frame.emplace_back(*it);
    }
    swap(frame, oFrame);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void GetStats(std::vector<refptr<PerfLogItem const> >& oLog)
{
    SG_ASSERT(nullptr != g_perfLog);
    PerfLog& perfLog = *g_perfLog;
    std::vector<refptr<PerfLogItem const> > log;
    auto const begin = perfLog.items.begin();
    auto const end = perfLog.items.end();
    for(auto it = begin; it != end; ++it)
    {
        log.emplace_back(it->second.get());
    }
    std::sort(log.begin(), log.end(), [](refptr<PerfLogItem const> const& a, refptr<PerfLogItem const> const& b)
    {
        if(a->uid != b->uid)
            return a->uid < b->uid;
        else
            return a->dynamicName < b->dynamicName;
    });
    swap(log, oLog);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void SetParameters(PerfLogParameters const& iParam)
{
    SG_ASSERT(nullptr != g_perfLog);
    PerfLog& perfLog = *g_perfLog;
    SG_ASSERT_MSG(0 == ((~iParam.logActiveLevels) & iParam.frameActiveLevels), "levels activated for frame must be activated for log");
    SG_ASSERT_MSG(0 == ((~iParam.logActiveLevels) & iParam.statActiveLevels), "levels activated for stats must be activated for log");
    SG_ASSERT((iParam.frameActiveLevels | iParam.statActiveLevels) == iParam.logActiveLevels);
    perfLog.param = iParam;
    Clear();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
PerfLogParameters const& GetParameters()
{
    SG_ASSERT(nullptr != g_perfLog);
    PerfLog& perfLog = *g_perfLog;
    return perfLog.param;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
namespace {
    template <typename T, typename U>
    void PrintVarStat(std::ostream& os, PerfLogVariableStat<T, U> const& stat, size_t statCount)
    {
        double ooStatCount = 1./statCount;
        double const avg = double(stat.sum) * ooStatCount;
        double const var = double(stat.sumSq) * ooStatCount - avg*avg;
        double const stddev = std::sqrt(var);
        os << "min: " << stat.min;
        os << ", avg: " << avg;
        os << ", max: " << stat.max;
        os << ", var: " << var;
        os << ", dev: " << stddev;
    }
    template <typename T, typename U>
    void PrintVar(std::ostream& os, char const* tab, PerfLogVariableStat<T, U> const& var, size_t statCount)
    {
        os << tab << var.name << " - ";
        PrintVarStat(os, var, statCount);
        os << std::endl;
    }
    template <typename T, typename U>
    void PrintVar(std::ostream& os, char const* tab, PerfLogVariable<T> const& var, std::vector<PerfLogVariableStat<T, U> > const& itemVars, size_t statCount)
    {
        os << tab << var.name << ": " << var.value;
        for(auto const& stat : itemVars)
        {
            if(stat.index == var.index)
            {
                os << " (";
                PrintVarStat(os, stat, statCount);
                os << ")";
                break;
            }
        }
        os << std::endl;
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void PrintPreviousFrame(std::ostream& os)
{
    u64 const freq = TicksPerSecond();
    size_t tabCount = 0;
    size_t const MAX_TAB = 256;
    size_t const TAB_WIDTH = 2;
    char nextTab[(MAX_TAB + 1) * TAB_WIDTH + 1] = "                ";
    char* tab = nextTab + 2;
    std::vector<u64> endStack;
    ArrayView<PerfLogItemOccurence const> frame = GetPreviousFrame();
    SG_ASSERT(!frame.empty());
    SG_ASSERT(nullptr == frame[0].item);
    endStack.push_back(frame[0].end);
    os << "frame: " << 1000000 * (frame[0].end - frame[0].begin) / freq << "us" << std::endl;
    size_t const count = frame.size();
    for(size_t i = 1; i < count; ++i)
    {
        PerfLogItemOccurence const& it = frame[i];
        if(i64(it.begin - endStack.back()) < 0)
        {
            SG_ASSERT(i64(it.end - endStack.back()) < 0);
            ++tabCount;
            SG_ASSERT(tabCount < MAX_TAB);
            for(size_t t = 0; t < tabCount; ++t)
                for(size_t w = 0; w < TAB_WIDTH; ++w)
                    tab[TAB_WIDTH * t + w] = ' ';
            tab[TAB_WIDTH * tabCount] = 0;
        }
        else
        {
            while(i64(it.begin - endStack.back()) >= 0)
            {
                SG_ASSERT(!endStack.empty());
                endStack.pop_back();
                --tabCount;
            }
            ++tabCount;
            for(size_t t = 0; t < tabCount; ++t)
            for(size_t w = 0; w < TAB_WIDTH; ++w)
                tab[TAB_WIDTH * t + w] = ' ';
            tab[TAB_WIDTH * tabCount] = 0;
        }
        SG_ASSERT(nullptr != it.item);
        auto const& item = *it.item;
        u64 const delta = it.end - it.begin;
        char const* shortFunction = item.function;
        {
            char const* tmp;
            while(nullptr != (tmp = strstr(shortFunction, "::"))) { shortFunction = tmp + 2; }
        }
        char const* shortFile = item.file;
        {
            char const* tmp;
            while(nullptr != (tmp = strstr(shortFile, "\\"))) { shortFile = tmp + 1; }
            while(nullptr != (tmp = strstr(shortFile, "/"))) { shortFile = tmp + 1; }
            SG_ASSERT(nullptr == strstr(shortFile, "\\"));
        }
        os << tab;
        os << shortFile << "(" <<  item.line << "): ";
        if(nullptr != item.name)
            os << item.name << " in ";
        else if(!item.dynamicName.empty())
            os << item.dynamicName << " in ";
        os << shortFunction << ": ";
        os << 1000000 * delta / freq << "us";
        if(!it.intVariables.empty() || !it.floatVariables.empty())
        {
            os << " {" << std::endl;
            size_t intCursor = 0;
            size_t floatCursor = 0;
            size_t const intCount = it.intVariables.size();
            size_t const floatCount = it.floatVariables.size();
            size_t const statCount = item.count;
            while(intCursor < intCount && floatCursor < floatCount)
            {
                if(it.intVariables[intCursor].index < it.floatVariables[floatCursor].index)
                {
                    auto const& var = it.intVariables[intCursor];
                    PrintVar(os, nextTab, var, item.intVariables, statCount);
                    ++intCursor;
                }
                else
                {
                    SG_ASSERT(item.intVariables[intCursor].index > item.floatVariables[floatCursor].index);
                    auto const& var = it.floatVariables[floatCursor];
                    PrintVar(os, nextTab, var, item.floatVariables, statCount);
                    ++floatCursor;
                }
            }
            while(intCursor < intCount)
            {
                auto const& var = it.intVariables[intCursor];
                PrintVar(os, nextTab, var, item.intVariables, statCount);
                ++intCursor;
            }
            while(floatCursor < floatCount)
            {
                auto const& var = it.floatVariables[floatCursor];
                PrintVar(os, nextTab, var, item.floatVariables, statCount);
                ++floatCursor;
            }
            os << tab << "}";
        }
        os << std::endl;
        //os << it.item-> it.begin << " " << it.end << std::endl;
        endStack.push_back(it.end);
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void PrintStats(std::ostream& os)
{
    char const* const tab = "    ";
    std::vector<refptr<PerfLogItem const> > log;
    GetStats(log);
    for(auto const& it : log)
    {
        PerfLogItem const& item = *it;
        char const* shortFunction = item.function;
        {
            char const* tmp;
            while(nullptr != (tmp = strstr(shortFunction, "::"))) { shortFunction = tmp + 2; }
        }
        char const* shortFile = item.file;
        {
            char const* tmp;
            while(nullptr != (tmp = strstr(shortFile, "\\"))) { shortFile = tmp + 1; }
            while(nullptr != (tmp = strstr(shortFile, "/"))) { shortFile = tmp + 1; }
            SG_ASSERT(nullptr == strstr(shortFile, "\\"));
        }
        os << shortFile << "(" <<  item.line << "): ";
        if(nullptr != item.name)
            os << item.name << " in ";
        else if(!item.dynamicName.empty())
            os << item.dynamicName << " in ";
        os << shortFunction;
        os << std::endl;
        os << tab << "called " << item.count << " times";
        //os << " {";
        os << std::endl;

        os << tab << "duration - ";
        PrintVarStat(os, item.delta, item.count);
        os << std::endl;

        size_t intCursor = 0;
        size_t floatCursor = 0;
        size_t const intCount = item.intVariables.size();
        size_t const floatCount = item.floatVariables.size();
        while(intCursor < intCount && floatCursor < floatCount)
        {
            if(item.intVariables[intCursor].index < item.floatVariables[floatCursor].index)
            {
                auto const& var = item.intVariables[intCursor];
                PrintVar(os, tab, var, item.count);
                ++intCursor;
            }
            else
            {
                SG_ASSERT(item.intVariables[intCursor].index > item.floatVariables[floatCursor].index);
                auto const& var = item.floatVariables[floatCursor];
                PrintVar(os, tab, var, item.count);
                ++floatCursor;
            }
        }
        while(intCursor < intCount)
        {
            auto const& var = item.intVariables[intCursor];
            PrintVar(os, tab, var, item.count);
            ++intCursor;
        }
        while(floatCursor < floatCount)
        {
            auto const& var = item.floatVariables[floatCursor];
            PrintVar(os, tab, var, item.count);
            ++floatCursor;
        }

        //os << "}" << std::endl;
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
}
//=============================================================================
PerfLogger::PerfLogger(size_t iUID, char const* iFunction, char const* iFile, size_t iLine, PerfLogType iType)
: m_uid(iUID)
, m_level(0)
, m_function(iFunction)
, m_file(iFile)
, m_line(iLine)
, m_type(iType)
, m_name(nullptr)
, m_dynamicName()
, m_begin(0)
, m_occurenceIndex(all_ones)
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
PerfLogger::~PerfLogger()
{
    SG_ASSERT(PerfLogType::CPU == m_type); // else, not impl
    if(0 != m_begin)
    {
        LARGE_INTEGER t;
        BOOL const rc = QueryPerformanceCounter(&t);
        SG_ASSERT_AND_UNUSED(rc);

        SG_ASSERT(nullptr != g_perfLog);
        PerfLog& perfLog = *g_perfLog;

        u32 const levelMask = 1 << m_level;

        u64 const end = t.QuadPart;
        u32 const delta = checked_numcastable(end - m_begin);
        SG_ASSERT(m_dynamicName.empty() || nullptr == m_name);

        std::vector<PerfLogVariable<i32> > intVariables;
        std::vector<PerfLogVariable<float> > floatVariables;
        {
            size_t const varCount = m_variables.size();
            for(size_t i = 0; i < varCount; ++i)
            {
                char const* const varName = m_variables[i].first;
                VarType type = m_variables[i].second->GetVarType();
                switch(type)
                {
                case VarType::Int:
                    {
                        i32 const value = m_variables[i].second->GetValueInt();
                        intVariables.emplace_back(varName, i, value);
                    }
                    break;
                case VarType::Float:
                    {
                        float const value = m_variables[i].second->GetValueFloat();
                        floatVariables.emplace_back(varName, i, value);
                    }
                    break;
                default:
                    SG_ASSERT_NOT_REACHED();
                }
            }
        }

        auto const& f = perfLog.items.equal_range(m_uid);
        PerfLogItem* pItem = nullptr;
        for(auto it = f.first; it != f.second; ++it)
        {
            SG_ASSERT(f.first == it || !m_dynamicName.empty());
            PerfLogItem& item = *it->second;
            SG_ASSERT(item.uid      == m_uid);
            SG_ASSERT(item.level    == m_level);
            SG_ASSERT(item.function == m_function);
            SG_ASSERT(item.file     == m_file);
            SG_ASSERT(item.line     == m_line);
            SG_ASSERT(item.level    == m_level);
            SG_ASSERT(item.type     == m_type);
            SG_ASSERT_MSG(item.name == m_name, "name must be a c-string constant");
            if(item.dynamicName == m_dynamicName)
            {
                pItem = &item;
                ++item.count;
                SG_ASSERT(nullptr == item.delta.name);
                item.delta.min = std::min(item.delta.min, delta);
                item.delta.max = std::max(item.delta.max, delta);
                item.delta.sum += delta;
                item.delta.sumSq += u64(delta) * delta;

                if(0 != (levelMask & perfLog.param.statActiveLevels))
                {
                    SG_ASSERT(item.intVariables.size() == intVariables.size());
                    SG_ASSERT(item.floatVariables.size() == floatVariables.size());
                    size_t const intVarCount = intVariables.size();
                    for(size_t i = 0; i < intVarCount; ++i)
                    {
                        SG_ASSERT(i < item.intVariables.size());
                        auto& var = item.intVariables[i];
                        SG_ASSERT_MSG(var.name == intVariables[i].name, "variable name must be a c-string constant");
                        SG_ASSERT(var.index == intVariables[i].index);
                        i32 const value = intVariables[i].value;
                        var.min = std::min(var.min, value);
                        var.max = std::max(var.max, value);
                        var.sum += value;
                        var.sumSq += value * value;
                    }
                    size_t const floatVarCount = floatVariables.size();
                    for(size_t i = 0; i < floatVarCount; ++i)
                    {
                        SG_ASSERT(i < item.floatVariables.size());
                        auto& var = item.floatVariables[i];
                        SG_ASSERT_MSG(var.name == floatVariables[i].name, "variable name must be a c-string constant");
                        SG_ASSERT(var.index == floatVariables[i].index);
                        float const value = floatVariables[i].value;
                        var.min = std::min(var.min, value);
                        var.max = std::max(var.max, value);
                        var.sum += value;
                        var.sumSq += value * value;
                    }
                }
            }
        }
        if(nullptr == pItem)
        {
            PerfLogItem* pnew = new PerfLogItem();
            auto const it = perfLog.items.emplace(m_uid, pnew);
            SG_ASSERT(it != perfLog.items.end());
            SG_ASSERT(it->first == m_uid);
            pItem = it->second.get();
            PerfLogItem& item = *pItem;
            item.uid         = m_uid;
            item.level       = m_level;
            item.function    = m_function;
            item.file        = m_file;
            item.line        = m_line;
            item.level       = m_level;
            item.type        = m_type;
            item.name        = m_name;
            item.dynamicName = m_dynamicName;

            item.count = 1;
            item.delta.name = nullptr;
            item.delta.min = delta;
            item.delta.max = delta;
            item.delta.sum = delta;
            item.delta.sumSq = delta * delta;
            if(0 != (levelMask & perfLog.param.statActiveLevels))
            {
                size_t const intVarCount = intVariables.size();
                for(size_t i = 0; i < intVarCount; ++i)
                {
                    item.intVariables.emplace_back();
                    auto& var = item.intVariables.back();
                    var.name = intVariables[i].name;
                    var.index = intVariables[i].index;
                    i32 const value = intVariables[i].value;
                    var.min = value;
                    var.max = value;
                    var.sum = value;
                    var.sumSq = value * value;
                }
                size_t const floatVarCount = floatVariables.size();
                for(size_t i = 0; i < floatVarCount; ++i)
                {
                    item.floatVariables.emplace_back();
                    auto& var = item.floatVariables.back();
                    var.name = floatVariables[i].name;
                    var.index = floatVariables[i].index;
                    float const value = floatVariables[i].value;
                    var.min = value;
                    var.max = value;
                    var.sum = value;
                    var.sumSq = value * value;
                }
            }
        }

        if(0 != (levelMask & perfLog.param.frameActiveLevels))
        {
            SG_ASSERT(m_occurenceIndex < perfLog.frame.size());
            PerfLogItemOccurence& occ = perfLog.frame[m_occurenceIndex];
            occ.begin = m_begin;
            occ.end = end;
            occ.item = pItem;
            swap(occ.intVariables, intVariables);
            swap(occ.floatVariables, floatVariables);
        }
    }
    else
    {
        SG_ASSERT(nullptr == g_perfLog);
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
PerfLogger& PerfLogger::operator() (int iLevel)
{
    SG_ASSERT(0 == m_level);
    SG_ASSERT_MSG(nullptr == m_name, "level should be set first");
    SG_ASSERT(m_dynamicName.empty());
    SG_ASSERT_MSG(m_variables.empty(), "level should be set first");
    SG_ASSERT(iLevel >= 0);
    m_level = iLevel;
    return *this;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
PerfLogger& PerfLogger::operator() (char const* iName)
{
    SG_ASSERT(nullptr == m_name);
    SG_ASSERT(m_dynamicName.empty());
    SG_ASSERT_MSG(m_variables.empty(), "name should be set second");
    m_name = iName;
    return *this;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
PerfLogger& PerfLogger::operator() (std::string const& iDynamicName)
{
    SG_ASSERT(nullptr == m_name);
    SG_ASSERT(m_dynamicName.empty());
    SG_ASSERT_MSG(m_variables.empty(), "name should be set first");
    m_dynamicName = iDynamicName;
    return *this;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
PerfLogger& PerfLogger::operator() (char const* iVarName, int iValue)
{
    m_variables.emplace_back(iVarName, new VarInt(iValue));
    return *this;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
PerfLogger& PerfLogger::operator() (char const* iVarName, float iValue)
{
    m_variables.emplace_back(iVarName, new VarFloat(iValue));
    return *this;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void PerfLogger::Run()
{
    SG_ASSERT(PerfLogType::CPU == m_type); // else, not impl
    SG_ASSERT(m_level < 32);
    u32 const levelMask = 1 << m_level;
    if(nullptr != g_perfLog && 0 != (levelMask & g_perfLog->param.logActiveLevels))
    {
        SG_ASSERT(nullptr != g_perfLog);
        PerfLog& perfLog = *g_perfLog;
        if(0 != (levelMask & perfLog.param.frameActiveLevels))
        {
            m_occurenceIndex = perfLog.frame.size();
            perfLog.frame.emplace_back();
        }

        LARGE_INTEGER t;
        BOOL const rc = QueryPerformanceCounter(&t);
        SG_ASSERT_AND_UNUSED(rc);
        m_begin = t.QuadPart;
    }
}
//=============================================================================
#if SG_ENABLE_UNIT_TESTS
namespace testperflog {
namespace {
    size_t FunctionA(size_t x)
    {
        CPU_PERF_LOG_SCOPE(0);
        size_t r = 0;
        for(size_t i = 0; i < 100000; ++i) { r += x; }
        return r;
    }
    size_t FunctionB(size_t x)
    {
        CPU_PERF_LOG_SCOPE(0);
        size_t r = 1;
        for(size_t i = 0; i < 100000; ++i) { r *= x; }
        return r;
    }
    size_t FunctionC(size_t x)
    {
        CPU_PERF_LOG_SCOPE(0);
        size_t r = 1;
        {
            CPU_PERF_LOG_SCOPE(1, "sum");
            for(size_t i = 0; i < 100000; ++i) { r += x & i; }
        }
        {
            CPU_PERF_LOG_SCOPE(2, "mul");
            for(size_t i = 0; i < 100000; ++i) { r *= x & i; }
        }
        return r;
    }
    size_t FunctionD1(size_t x)
    {
        size_t r = 0;
        CPU_PERF_LOG_SCOPE(3,("x", x),("return", &r));
        for(size_t i = 1; i < 1000; ++i)
        {
            r += i + x;
            while(0 == (r % 2)) r /= 2;
            while(0 == (r % 3)) r /= 3;
        }
        return r;
    }
    size_t FunctionD2(size_t x)
    {
        size_t r = x+1;
        CPU_PERF_LOG_SCOPE(4,("x", x),("return", &r));
        for(size_t i = 1; i < 1000; ++i)
        {
            r *= i;
            size_t const prime = 1009;
            while(prime < r)
                r -= prime;
        }
        return r;
    }
    size_t FunctionD(size_t x)
    {
        CPU_PERF_LOG_SCOPE(0);
        size_t r = 1;
        for(size_t i = 0; i < 2; ++i)
        {
            CPU_PERF_LOG_SCOPE(1,Format("iteration %0", i));
            r ^= FunctionD1(x + i * 13) * FunctionD2(x + i * 13);
        }
        return r;
    }
}
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
SG_TEST((sg), PerfLog, (Core, slow))
{
    using namespace testperflog;
    perflog::Init();

    PerfLogParameters param;
    param.frameActiveLevels = 1 | 1 << 1 | 1 << 3 | 1 << 4;
    param.statActiveLevels =  1 | 1 << 1 | 1 << 2 | 1 << 3;
    param.logActiveLevels = param.frameActiveLevels | param.statActiveLevels;
    perflog::SetParameters(param);

    size_t r = 0;
    for(size_t i = 0; i < 3; ++i)
    {
        r ^= FunctionA(i);
        r ^= FunctionB(i);
        r ^= FunctionC(i);
        r ^= FunctionD(i);
    }

    std::vector<refptr<PerfLogItem const> > log;
    perflog::GetStats(log);

    perflog::OnNewFrame();
    std::vector<PerfLogItemOccurence> frameLog;
    perflog::GetPreviousFrameCopy(frameLog);

    std::ostringstream oss;
    oss << "r = " << r << std::endl;
    oss << "-----------------------------------------------------------------------------" << std::endl;
    perflog::PrintStats(oss);
    oss << "-----------------------------------------------------------------------------" << std::endl;
    perflog::PrintPreviousFrame(oss);
    oss << "-----------------------------------------------------------------------------" << std::endl;

    perflog::Clear();

    SG_LOG_INFO("Test", oss.str().c_str());
    perflog::Shutdown();
}
//=============================================================================
}

#endif
#endif
