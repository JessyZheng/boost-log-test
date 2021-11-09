#ifndef KUBOT_ROS2_UTILS_H
#define KUBOT_ROS2_UTILS_H


#include <iostream>
#include <list>
#include <mutex>
#include <chrono>
#include <memory>
#include <functional>
#include <boost/algorithm/string.hpp>
#include <boost/preprocessor/punctuation/comma.hpp>
#include <boost/preprocessor/control/iif.hpp>
#include <boost/preprocessor/comparison/equal.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/seq/size.hpp>
#include <boost/preprocessor/seq/seq.hpp>


#define BITS_FIL(n) ((n < 64) ? (0x0000000000000001UL << n) - 1 : 0xffffffffffffffffUL)


#define DEFINE_ENUM(name, values)                               \
  enum class name                                               \
  {                                                             \
    BOOST_PP_SEQ_FOR_EACH(DEFINE_ENUM_VALUE, , values)          \
  };                                                            \
namespace std                                                   \
{                                                               \
  inline const char* to_string(name val)                        \
  {                                                             \
    switch (val)                                                \
    {                                                           \
      BOOST_PP_SEQ_FOR_EACH(DEFINE_ENUM_FORMAT, name, values)   \
    default:                                                    \
        return "N/A";                                           \
    }                                                           \
  }                                                             \
}

#define DEFINE_INNER_ENUM(name, values)                         \
  enum class name                                               \
  {                                                             \
    BOOST_PP_SEQ_FOR_EACH(DEFINE_ENUM_VALUE, , values)          \
  };                                                            \
  static inline const char* to_string(name val)                 \
  {                                                             \
    switch (val)                                                \
    {                                                           \
      BOOST_PP_SEQ_FOR_EACH(DEFINE_ENUM_FORMAT, name, values)   \
    default:                                                    \
        return "N/A";                                           \
    }                                                           \
  }

#define DEFINE_ENUM_VALUE(r, data, elem)                        \
  BOOST_PP_SEQ_HEAD(elem)                                       \
  BOOST_PP_IIF(BOOST_PP_EQUAL(BOOST_PP_SEQ_SIZE(elem), 2),      \
               = BOOST_PP_SEQ_TAIL(elem), )                     \
  BOOST_PP_COMMA()

#define DEFINE_ENUM_FORMAT(r, data, elem)                       \
    case data :: BOOST_PP_SEQ_HEAD(elem):                       \
  return BOOST_PP_STRINGIZE(BOOST_PP_SEQ_HEAD(elem));


/* Enum Reader */

template<typename Enum>
class EnumReader
{
    Enum& e_;

    friend std::istream& operator>>(std::istream& in, const EnumReader& val) {
        typename std::underlying_type<Enum>::type asInt;
        if (in >> asInt) val.e_ = static_cast<Enum>(asInt);
        return in;
    }
public:
    EnumReader(Enum& e) : e_(e) {}
};

template<typename Enum>
EnumReader<Enum> read_enum(Enum& e)
{
    return EnumReader<Enum>(e);
}


/* set thread attribute */

static inline void bind_cpu(int cpu_id)
{
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu_id, &cpuset);
    pthread_setaffinity_np(pthread_self(), sizeof(cpuset), &cpuset);
}


/* Instance Factory */

template<typename _Tp, typename... _Args>
const std::shared_ptr<_Tp> GetInstance(bool shared, _Args&&... __args)
{
    static std::mutex instances_lock;
    static std::list<std::shared_ptr<_Tp>> instances;

    std::shared_ptr<_Tp> p_instance, n_instance;
    instances_lock.lock();
    for (const auto& instance : instances)
    {
        if (!instance->EqualInstance(std::forward<_Args>(__args)...))
            continue;
        else
        {
            p_instance = instance;
            break;
        }
    }
    if (p_instance == nullptr)
    {
        n_instance = std::make_shared<_Tp>(std::forward<_Args>(__args)...);
        instances.push_back(n_instance);
    }
    instances_lock.unlock();

    return shared ? (p_instance != nullptr ? p_instance : n_instance) : n_instance;
}


/* Lock helper */

class scoped_guard
{
private:
    std::function<void(void)> enter_scope_callback;
    std::function<void(void)> leave_scope_callback;

public:
    scoped_guard(const scoped_guard&) = delete;

    scoped_guard(std::function<void(void)> enter_scope_callback, std::function<void(void)> leave_scope_callback) :
            enter_scope_callback(enter_scope_callback),
            leave_scope_callback(leave_scope_callback)
    {
        enter_scope_callback();
    };
    ~scoped_guard()
    {
        leave_scope_callback();
    };
};


/* Fault Tolerance */

enum class FT_STATUS
{
    OK          = 0,
    FAULT       = 1,
};

struct FT_COMPONENT
{
    virtual FT_STATUS GetStatus(void) const = 0;
};

struct FT_ARGUMENTS
{
public:
    /* FT control parameters */
    int                         retry_count_limit;
    std::chrono::milliseconds   elapse_time_limit;

public:
    FT_ARGUMENTS(unsigned int retry_count_limit, unsigned int elapse_time_limit) :
            retry_count_limit(retry_count_limit),
            elapse_time_limit(elapse_time_limit)
    {};
};


#endif //KUBOT_ROS2_UTILS_H
