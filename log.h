#pragma once
#ifndef KUBOTROBOTICS_LOG_H
#define KUBOTROBOTICS_LOG_H

#include <map>
#include <list>
#include <sys/timeb.h>
#include <string>
#include <iostream>
#include <sstream>
#include <chrono>
#include <deque>
#include <thread>
#include <stdexcept>
#include <boost/format.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/sources/severity_channel_logger.hpp>
#include <boost/log/utility/manipulators/dump.hpp>
#include <sys/time.h>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <sys/prctl.h>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <sys/prctl.h>
#include <condition_variable>

#include "json.hpp"
//#include "utils.h"

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace sinks = logging::sinks;
using namespace logging::trivial;

#define ANSI_CLEAR      "\x1B[0;00m"
#define ANSI_RED        "\x1B[1;31m"
#define ANSI_GREEN      "\x1B[1;32m"
#define ANSI_YELLOW     "\x1B[1;33m"
#define ANSI_BLUE       "\x1B[1;34m"
#define ANSI_MAGENTA    "\x1B[1;35m"
#define ANSI_CYAN       "\x1B[1;36m"

const std::string   printf_timestamp(const std::string format = "%Y-%m-%d %H:%M:%S");
const std::string   printf_timestamp(struct timeval tv, const std::string format = "%Y-%m-%d %H:%M:%S");
const std::string   printf_timestamp(std::chrono::system_clock::time_point tp, const std::string format = "%Y-%m-%d %H:%M:%S");
const std::string   printf_timestamp(std::chrono::steady_clock::time_point tp, const std::string format = "%Y-%m-%d %H:%M:%S");
const std::string   serial_timestamp(void);
const std::string   log_elapse(std::chrono::steady_clock::time_point start_time, std::chrono::steady_clock::time_point end_time);



class BoostLogBuffer : public std::stringbuf
{
    src::severity_channel_logger<severity_level, std::string>& logger;
    severity_level                                              level;
public:
    BoostLogBuffer(src::severity_channel_logger<severity_level, std::string>& logger, severity_level level) :
            std::stringbuf(),
            logger(logger),
            level(level)
    {};

    virtual int sync()
    {
//        std::cout << "KubotLogBuffer[" << (int)level << "] sync: " << this->str() << std::endl;

        // add this->str() to database here
        BOOST_LOG_SEV(logger, level) << this->str();
        // (optionally clear buffer afterwards)
        this->str("");

        return 0;
    }
};
class BoostLogLogger : public std::ostream
{
public:
    BoostLogLogger(BoostLogBuffer &buffer) :
            std::ostream(&buffer)
    {};

    ~BoostLogLogger()
    {
        this->flush();
    };
};

class PrintLogBuffer : public std::stringbuf
{
    std::string                                                 title;
    severity_level                                              level;
public:
    PrintLogBuffer(std::string title, severity_level level) :
            std::stringbuf(),
            title(title),
            level(level)
    {};

    virtual int sync()
    {
        if ((int)level >= (int)info)
        {
            switch (level)
            {
                case trace:
                    std::cerr << "[" << printf_timestamp("%Y-%b-%d %H:%M:%S") << "] [" << ANSI_CYAN    << "TRAC" << ANSI_CLEAR << "] [" << title << "] " << this->str() << std::endl;
                    break;
                case debug:
                    std::cerr << "[" << printf_timestamp("%Y-%b-%d %H:%M:%S") << "] [" << ANSI_GREEN   << "DBUG" << ANSI_CLEAR << "] [" << title << "] " << this->str() << std::endl;
                    break;
                case info:
                    std::cerr << "[" << printf_timestamp("%Y-%b-%d %H:%M:%S") << "] [" << ANSI_BLUE    << "INFO" << ANSI_CLEAR << "] [" << title << "] " << this->str() << std::endl;
                    break;
                case warning:
                    std::cerr << "[" << printf_timestamp("%Y-%b-%d %H:%M:%S") << "] [" << ANSI_YELLOW  << "WARN" << ANSI_CLEAR << "] [" << title << "] " << this->str() << std::endl;
                    break;
                case error:
                    std::cerr << "[" << printf_timestamp("%Y-%b-%d %H:%M:%S") << "] [" << ANSI_MAGENTA << "ERRR" << ANSI_CLEAR << "] [" << title << "] " << this->str() << std::endl;
                    break;
                case fatal:
                    std::cerr << "[" << printf_timestamp("%Y-%b-%d %H:%M:%S") << "] [" << ANSI_RED     << "FATL" << ANSI_CLEAR << "] [" << title << "] " << this->str() << std::endl;
                    break;
                default:
                    std::cerr << "[" << printf_timestamp("%Y-%b-%d %H:%M:%S") << "] [" << (int) level;
            }
        }
        this->str("");

        return 0;
    }
};

class PrintLogLogger : public std::ostream
{
public:
    PrintLogLogger(PrintLogBuffer &buffer) :
            std::ostream(&buffer)
    {};

    ~PrintLogLogger()
    {
        this->flush();
    };
};

class KubotLogHelper
{
private:
    enum class log_impl
    {
        boost_log,
        print_log
    };
    static bool init_done;
    static log_impl    log_impl_select;
    static std::string default_channel;

private:
    static std::map<std::string, src::severity_channel_logger<severity_level, std::string>> scl_map;
    static src::severity_channel_logger<severity_level, std::string>& GetLogger(std::string channel);
    static boost::shared_ptr<sinks::synchronous_sink<sinks::text_ostream_backend>> console_sink;
    static logging::trivial::severity_level console_log_level;

public:
    static bool InitBoostLogEnvironment(const std::string config_file);
    static bool InitBoostLogEnvironment(const std::string config_file, const std::string channel);
    static int  SetConsoleSeverity(const std::string severity);
private:
    std::unique_ptr<BoostLogBuffer> boost_log_buffer;
public:
    std::unique_ptr<BoostLogLogger> boost_log_logger;

public:
    static bool InitPrintLogEnvironment();
    static bool InitPrintLogEnvironment(const std::string channel);
private:
    std::unique_ptr<PrintLogBuffer> print_log_buffer;
public:
    std::unique_ptr<PrintLogLogger> print_log_logger;

public:
    KubotLogHelper(severity_level level) :
            KubotLogHelper(default_channel, level)
    {};

    KubotLogHelper(std::string channel, severity_level level) :
            boost_log_buffer(log_impl_select != log_impl::boost_log ? nullptr : new BoostLogBuffer(GetLogger(channel==""?default_channel:channel), level)),
            boost_log_logger(log_impl_select != log_impl::boost_log ? nullptr : new BoostLogLogger(*boost_log_buffer.get())),
            print_log_buffer(log_impl_select != log_impl::print_log ? nullptr : new PrintLogBuffer((channel==""?default_channel:channel), level)),
            print_log_logger(log_impl_select != log_impl::print_log ? nullptr : new PrintLogLogger(*print_log_buffer.get()))
    {};

public:
    std::ostream& logger()
    {
        switch (log_impl_select)
        {
            case log_impl::boost_log:
                return *boost_log_logger.get();
            case log_impl::print_log:
                return *print_log_logger.get();
        }

        return std::cerr;
    }
};

#define FILENAME(path) (strrchr(path,'/') ? (strrchr(path,'/')+1) : path)
#define KUBOT_LOG(_LEVEL_)                      (KubotLogHelper(_LEVEL_).logger() << "["<< FILENAME(__FILE__) << ":"<< __LINE__ << "]: ")
#define KUBOT_CHANNEL_LOG(__CHANNEL_, _LEVEL_)  (KubotLogHelper(__CHANNEL_, _LEVEL_).logger() << "["<< FILENAME(__FILE__) << ":"<< __LINE__ << "]: ")



namespace boost
{
    namespace serialization
    {
        template<class Archive>
        void serialize(Archive&ar, std::chrono::steady_clock::time_point &time, const unsigned int version)
        {
            auto count = time.time_since_epoch().count();
            ar & count;
            time = std::chrono::steady_clock::time_point(std::chrono::steady_clock::duration(count));
        }

        template<class Archive>
        void serialize(Archive&ar, std::chrono::steady_clock::duration &time, const unsigned int version)
        {
            auto count = time.count();
            ar & count;
            time = std::chrono::steady_clock::duration(count);
        }

        template<class Archive>
        void serialize(Archive&ar, nlohmann::json &param, const unsigned int version)
        {
            auto paramstr = param.dump();
            ar & paramstr;
            param = nlohmann::json::parse(paramstr);
        }
    }
}


template<typename _Tp>
struct RECORD_CACHE
{
private:
    std::string                     log_channel;
    severity_level                  log_level;

    std::string                     cache_name;
    time_t                          limit_time;
    size_t                          limit_size;
    std::shared_ptr<std::list<std::tuple<struct timeval,_Tp>>> record_buffer;

    std::mutex                      m_dump_mtx;
    std::condition_variable         m_dump_cv;
    std::shared_ptr<std::list<std::tuple<struct timeval,_Tp>>> dump_buffer;

public:
    RECORD_CACHE(std::string channel, severity_level level, std::string name, time_t time, size_t size) :
            log_channel(channel),
            log_level(level),
            cache_name(name),
            limit_time(time),
            limit_size(size)
    {
        init();
    };

    void push(_Tp reocrd_entity)
    {
        struct timeval tv;
        gettimeofday(&tv, NULL);

        std::unique_lock <std::mutex> lk(m_dump_mtx);
        // check size & time
        if((record_buffer->size() >= limit_size) ||
           (!record_buffer->empty() && (std::get<0>(record_buffer->front()).tv_sec + limit_time)*1000000 + std::get<0>(record_buffer->front()).tv_usec < tv.tv_sec*1000000 + tv.tv_usec))
        {
            KUBOT_LOG(warning) << "cache_name = " << cache_name << ", oversize or timeout, record_buffer->size() = " << record_buffer->size();
            dump_buffer->splice(dump_buffer->end(), *record_buffer);
            record_buffer = std::make_shared<std::list<std::tuple<struct timeval,_Tp>>>();
            m_dump_cv.notify_all();
        }

        record_buffer->emplace_back(std::make_tuple(tv, reocrd_entity));
    }

    void dump(void)
    {
        if (record_buffer->empty())
            return;

        //KUBOT_LOG(info) << "cache_name = " << cache_name << ",record_buffer->size() = " << record_buffer->size() << ", dump_buffer->size() = " << dump_buffer->size();
        std::unique_lock <std::mutex> lk(m_dump_mtx);
        dump_buffer->splice(dump_buffer->end(), *record_buffer);
        m_dump_cv.notify_all();
    }

    void init(void)
    {
        record_buffer = std::make_shared<std::list<std::tuple<struct timeval,_Tp>>>();
        dump_buffer = std::make_shared<std::list<std::tuple<struct timeval,_Tp>>>();

        std::thread([this](){
            //bind_cpu(5);
            prctl(PR_SET_NAME, (std::string("dump_")+cache_name).c_str());

            while(true)
            {
                std::unique_lock<std::mutex> lk(m_dump_mtx);
                m_dump_cv.wait(lk, [this]{ return !dump_buffer->empty();});

                //KUBOT_LOG(info) << "cache_name = " << cache_name << ", dump_buffer->size() = " << dump_buffer->size();
                auto log_buffer = dump_buffer;
                dump_buffer = std::make_shared<std::list<std::tuple<struct timeval,_Tp>>>();
                lk.unlock();

                for(auto& record_entity : *log_buffer)
                {
                    KUBOT_CHANNEL_LOG(log_channel, log_level) << "[" << cache_name << "] "
                                                              << "[" << printf_timestamp(std::get<0>(record_entity)) << "] "
                                                              << std::get<1>(record_entity);
                }
            }
        }).detach();
    }
};

#endif //KUBOTROBOTICS_LOG_H
