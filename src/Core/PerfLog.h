#ifndef Core_PerfLog_H
#define Core_PerfLog_H

#include "Assert.h"
#include "Config.h"

#if !SG_ENABLE_PERF_LOG

#define SIMPLE_CPU_PERF_LOG_SCOPE(NAME) ((void)0)

#if SG_COMPILER_IS_MSVC
#define CPU_PERF_LOG_SCOPE(...) ((void)0)
#define GPU_PERF_LOG_SCOPE(...) ((void)0)
#else
#error "todo"
#endif

#else

#include "IntTypes.h"
#include "Preprocessor.h"
#include "SmartPtr.h"
#include "ArrayView.h"
#include <unordered_set>
#include <type_traits>

#define DETAIL_SIMPLE_CPU_PERF_LOG_SCOPE(NAME, LINE) \
    static u64 perflog_best_##LINE=::sg::all_ones; sg::SimpleCPUPerfLog perflog_##LINE = SimpleCPUPerfLog(NAME, &perflog_best_##LINE)
#define DETAIL_SIMPLE_CPU_PERF_LOG_SCOPE_I(NAME, LINE) \
    DETAIL_SIMPLE_CPU_PERF_LOG_SCOPE(NAME, LINE)

#define SIMPLE_CPU_PERF_LOG_SCOPE(NAME) \
    DETAIL_SIMPLE_CPU_PERF_LOG_SCOPE_I(NAME, __LINE__)

#define SG_IMPL_PERF_LOG_BEGIN_I(FCT_NAME, FILE_NAME, LINE, TYPE) \
    static size_t const perflog_uid_##LINE = sg::PerfLogger::GetNextUID(); \
    sg::PerfLogger perflog_##LINE (perflog_uid_##LINE, FCT_NAME, FILE_NAME, LINE, TYPE);

#define SG_IMPL_PERF_LOG_BEGIN(FCT_NAME, FILE_NAME, LINE, TYPE) \
    SG_IMPL_PERF_LOG_BEGIN_I(FCT_NAME, FILE_NAME, LINE, TYPE)

#define SG_IMPL_PERF_LOG_RUN_I(LINE) \
    perflog_##LINE.Run()

#define SG_IMPL_PERF_LOG_RUN(LINE) \
    SG_IMPL_PERF_LOG_RUN_I(LINE)

#define SG_IMPL_PERF_LOG_READ_ARG_II(ARG, LINE) \
    perflog_##LINE ( STRIP_PARENTHESIS(ARG) );

#define SG_IMPL_PERF_LOG_READ_ARG_I(ARG, LINE) \
    SG_IMPL_PERF_LOG_READ_ARG_II(ARG, LINE)

#define SG_IMPL_PERF_LOG_READ_ARG(ARG, LINE) \
    /* Here, we replace argument LINE by __LINE__ as it seems there may be a bug in msvc 2013 */ \
    /* which prevents correct transmission of __LINE__ in previous macro stack. */ \
    SG_IMPL_PERF_LOG_READ_ARG_I(ARG, __LINE__)

#define SG_IMPL_PERF_LOG(FCT_NAME, FILE_NAME, LINE, TYPE, ARGS) \
    /* Here, we replace argument LINE by __LINE__ as it seems there may be a bug in msvc 2013 */ \
    /* which prevents correct usage of __LINE__ in previous macro stack. */ \
    SG_IMPL_PERF_LOG_BEGIN(FCT_NAME, FILE_NAME, __LINE__, TYPE) \
    APPLY_MULTIARG_MACRO_FOR_EACH(SG_IMPL_PERF_LOG_READ_ARG, ARGS, LINE) \
    SG_IMPL_PERF_LOG_RUN(__LINE__)

#define CPU_PERF_LOG_SCOPE(...) \
    SG_IMPL_PERF_LOG(SG_FUNCTION_NAME, __FILE__, __LINE__, sg::PerfLogType::CPU, (__VA_ARGS__))

#define GPU_PERF_LOG_SCOPE(...) \
    SG_IMPL_PERF_LOG(SG_FUNCTION_NAME, __FILE__, __LINE__, sg::PerfLogType::CPU_GPU, (__VA_ARGS__))

// usage:
// CPU_PERF_LOG_SCOPE(LEVEL [, NAME] [, (PARAM_NAME, VALUE)]*)
//      LEVEL is used to show/hide some log entries
//      NAME is used to identify log entry by user in addition to function name and line.
//          if NAME is a dynamic std::string, it is used to differentiate entries sharing same location.
//      PARAM_NAME is the name of a parameter on which statistics will be computed.
//      PARAM_VALUE is the value of a parameter.
// exemple usage:
// CPU_PERF_LOG_SCOPE(1);
// CPU_PERF_LOG_SCOPE(1,"My sub function",("width", 128),("height", 64));
// CPU_PERF_LOG_SCOPE(1,"My sub function");
// CPU_PERF_LOG_SCOPE(1,"My sub function",("width", 128),("height", 64));
// {
//     int validCount = 0;
//     CPU_PERF_LOG_SCOPE(1,("object count", 128),("valid count", &validCount));
//     [...] ++validCount [...]
// }

namespace sg {
//=============================================================================
// SimpleCPUPerfLog outputs each perf measure in the log.
class SimpleCPUPerfLog
{
public:
    SimpleCPUPerfLog(char const* iName, u64* ioBest);
    ~SimpleCPUPerfLog();
private:
    char const* m_name;
    u64 m_begin;
    u64* m_best;
};
//=============================================================================
enum class PerfLogType { CPU, CPU_GPU };
//=============================================================================
class PerfLogger
{
public:
    PerfLogger(size_t iUID, char const* iFunction, char const* iFile, size_t iLine, PerfLogType iType);
    ~PerfLogger();
    PerfLogger& operator() (int iLevel);
    PerfLogger& operator() (char const* iName);
    PerfLogger& operator() (std::string const& iDynamicName);
    PerfLogger& operator() (char const* iVarName, int iValue);
    PerfLogger& operator() (char const* iVarName, float iValue);
    enum class VarType { Unknown, Int, Float };
    class IVar : public RefCountable
    {
    public:
        virtual ~IVar() {}
        virtual VarType GetVarType() const = 0;
        virtual i32 GetValueInt() const = 0;
        virtual float GetValueFloat() const = 0;
    };
    class VarInt : public IVar
    {
    public:
        explicit VarInt(i32 iValue) : m_value(iValue) {}
        template <typename T>
        explicit VarInt(T iValue) : m_value(checked_numcastable(iValue)) {}
        virtual VarType GetVarType() const override { return VarType::Int; }
        virtual i32 GetValueInt() const override { return m_value; }
        virtual float GetValueFloat() const override { SG_ASSERT_NOT_REACHED(); return 0.f; }
    private:
        i32 m_value;
    };
    class VarFloat : public IVar
    {
    public:
        explicit VarFloat(float iValue) : m_value(iValue) {}
        virtual VarType GetVarType() const override { return VarType::Int; }
        virtual i32 GetValueInt() const override { SG_ASSERT_NOT_REACHED(); return 0; }
        virtual float GetValueFloat() const override { return m_value; }
    private:
        float m_value;
    };
    template <typename T>
    class VarByPointer : public IVar
    {
        SG_NO_COPY_OPERATOR(VarByPointer)
    public:
        static_assert(std::is_pointer<T>::value, "T must be a pointer type");
        typedef typename std::remove_const<typename std::remove_pointer<T>::type>::type base_type;
        static VarType constexpr var_type = std::is_integral<base_type>::value ? VarType::Int : std::is_floating_point<base_type>::value ? VarType::Float : VarType::Unknown;
        typedef typename std::conditional<VarType::Int == var_type, i32, typename std::conditional<VarType::Float == var_type, float, void>::type>::type projected_type;

        explicit VarByPointer(base_type const* iValue) : m_value(iValue) { SG_ASSERT(nullptr != m_value); }
        virtual VarType GetVarType() const override
        {
            return var_type;
        }
        virtual i32 GetValueInt() const override
        {
            SG_ASSERT(VarType::Int == var_type);
            SG_ASSERT(nullptr != m_value);
            return checked_numcastable(*m_value);
        }
        virtual float GetValueFloat() const override
        {
            SG_ASSERT(VarType::Float == var_type);
            SG_ASSERT(nullptr != m_value);
            return checked_numcastable(*m_value);
        }
    private:
        base_type const*const m_value;
    };
    template <typename T>
    PerfLogger& operator() (char const* iVarName, T iValue)
    {
        typedef typename std::conditional<std::is_integral<T>::value, VarInt, typename std::conditional<std::is_floating_point<T>::value, VarFloat, VarByPointer<T> >::type>::type var_type;
        m_variables.emplace_back(iVarName, new var_type(iValue));
        return *this;
    }
    void Run();

    static size_t GetNextUID() { static size_t s_uid = all_ones; ++s_uid; return s_uid;}
private:
    size_t m_uid;
    size_t m_level;
    char const* m_function;
    char const* m_file;
    size_t m_line;
    PerfLogType m_type;
    char const* m_name;
    std::string m_dynamicName;
    u64 m_begin;
    size_t m_occurenceIndex;
    std::vector<std::pair<char const*, refptr<IVar> > > m_variables;
};
//=============================================================================
template <typename T>
struct PerfLogVariable
{
    char const* name;
    size_t index;
    T value;
    PerfLogVariable(char const* iName, size_t iIndex, T iValue) : name(iName), index(iIndex), value(iValue) {}
};
template <typename T, typename U>
struct PerfLogVariableStat
{
    char const* name;
    size_t index;
    T min;
    T max;
    U sum;
    U sumSq;
};
struct PerfLogItem : public RefAndSafeCountable
{
    size_t uid;
    size_t level;
    char const* function;
    char const* file;
    size_t line;
    PerfLogType type;
    char const* name;
    std::string dynamicName;

    size_t count;
    PerfLogVariableStat<u32, u64> delta;
    std::vector<PerfLogVariableStat<i32, i64> > intVariables;
    std::vector<PerfLogVariableStat<float, double> > floatVariables;
};
struct PerfLogItemOccurence
{
    safeptr<PerfLogItem const> item;
    u64 begin;
    u64 end;
    std::vector<PerfLogVariable<i32> > intVariables;
    std::vector<PerfLogVariable<float> > floatVariables;
};
struct PerfLogParameters
{
    u32 logActiveLevels;
    u32 frameActiveLevels;
    u32 statActiveLevels;

    PerfLogParameters()
        : logActiveLevels(all_ones)
        , frameActiveLevels(all_ones)
        , statActiveLevels(all_ones)
    {
    }
    void SetMaxLevel(u32 iLevel)
    {
        SG_ASSERT(iLevel < 32);
        SG_ASSERT(-1 == logActiveLevels);
        SG_ASSERT(-1 == frameActiveLevels);
        SG_ASSERT(-1 == statActiveLevels);
        logActiveLevels = 0xFFFFFFFF >> (32 - iLevel - 1);
        frameActiveLevels = logActiveLevels;
        statActiveLevels = logActiveLevels;
    }
    void SetMaxFameAndStatLevel(u32 iFrameLevel, u32 iStatLevel)
    {
        SG_ASSERT(iFrameLevel < 32);
        SG_ASSERT(iStatLevel < 32);
        SG_ASSERT(-1 == logActiveLevels);
        SG_ASSERT(-1 == frameActiveLevels);
        SG_ASSERT(-1 == statActiveLevels);
        frameActiveLevels = 0xFFFFFFFF >> (32 - iFrameLevel - 1);
        statActiveLevels = 0xFFFFFFFF >> (32 - iStatLevel - 1);
        logActiveLevels = frameActiveLevels | statActiveLevels;
    }
};
//=============================================================================
namespace perflog {
    void Init();
    void Shutdown();
    u64 TicksPerSecond();
    void Clear();
    void OnNewFrame();
    ArrayView<PerfLogItemOccurence const> GetPreviousFrame();
    void GetPreviousFrameCopy(std::vector<PerfLogItemOccurence>& oFrame);
    void GetStats(std::vector<refptr<PerfLogItem const> >& oLog);
    void SetParameters(PerfLogParameters const& iParam);
    PerfLogParameters const& GetParameters();
    void PrintPreviousFrame(std::ostream& os);
    void PrintStats(std::ostream& os);
}
//=============================================================================
}
#endif
#endif
