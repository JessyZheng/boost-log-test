#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <chrono>
#include <stdexcept>
#include <sys/timeb.h>
#include <map>

#include <boost/filesystem.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/smart_ptr/make_shared_object.hpp>
#include <boost/format.hpp>
#include <boost/phoenix.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/attributes/attribute_name.hpp>
#include <boost/log/attributes/scoped_attribute.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/severity_channel_logger.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/utility/setup.hpp>
#include <boost/log/utility/value_ref.hpp>
#include <boost/log/utility/formatting_ostream.hpp>
#include <boost/log/utility/manipulators/add_value.hpp>
#include <boost/log/utility/setup/formatter_parser.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/from_stream.hpp>
#include <boost/log/utility/setup/formatter_parser.hpp>
#include <boost/log/utility/setup/filter_parser.hpp>

#include "log.h"

namespace logging = boost::log;
namespace attrs = logging::attributes;
namespace expr = logging::expressions;
namespace src = logging::sources;
namespace keywords = logging::keywords;
namespace sinks = logging::sinks;

using namespace logging::trivial;


/******************************** TimeFormatter ********************************/

const std::string   printf_timestamp(const std::string format)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    struct  tm      st_tm;
    char            st_string1[64];
    char            st_string2[64];

    localtime_r(&tv.tv_sec, &st_tm);
    strftime(st_string1, sizeof(st_string1), format.c_str(), &st_tm);
    sprintf(st_string2,
            ".%06ld",
            tv.tv_usec);

    return std::string(st_string1) + std::string(st_string2);
}

const std::string   printf_timestamp(struct timeval tv, const std::string format)
{
    struct  tm      st_tm;
    char            st_string1[64];
    char            st_string2[64];

    localtime_r(&tv.tv_sec, &st_tm);
    strftime(st_string1, sizeof(st_string1), format.c_str(), &st_tm);
    sprintf(st_string2,
            ".%06ld",
            tv.tv_usec);

    return std::string(st_string1) + std::string(st_string2);
}

const std::string   printf_timestamp(std::chrono::steady_clock::time_point tp, const std::string format)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    auto df = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - tp).count();
    {
        tv.tv_sec   -=  df/1000000;
        tv.tv_usec  -=  df%1000000;
    }
    if (tv.tv_usec < 0)
    {
        tv.tv_sec   -=  1;
        tv.tv_usec  +=  1000000;
    }

    struct  tm      st_tm;
    char            st_string1[64];
    char            st_string2[64];

    localtime_r(&tv.tv_sec, &st_tm);
    strftime(st_string1, sizeof(st_string1), format.c_str(), &st_tm);
    sprintf(st_string2,
            ".%06ld",
            tv.tv_usec);

    return std::string(st_string1) + std::string(st_string2);
}

const std::string   printf_timestamp(std::chrono::system_clock::time_point tp, const std::string format)
{
    struct timeval tv;

    auto df = std::chrono::duration_cast<std::chrono::microseconds>(tp.time_since_epoch()).count();
    {
        tv.tv_sec   =  df/1000000;
        tv.tv_usec  =  df%1000000;
    }
    if (tv.tv_usec < 0)
    {
        tv.tv_sec   -=  1;
        tv.tv_usec  +=  1000000;
    }

    struct  tm      st_tm;
    char            st_string1[64];
    char            st_string2[64];

    localtime_r(&tv.tv_sec, &st_tm);
    strftime(st_string1, sizeof(st_string1), format.c_str(), &st_tm);
    sprintf(st_string2,
            ".%06ld",
            tv.tv_usec);

    return std::string(st_string1) + std::string(st_string2);
}

const std::string   serial_timestamp(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    struct  tm      st_tm;
    char            st_string[64];

    localtime_r(&tv.tv_sec, &st_tm);
    sprintf(st_string,
            "%04d%02d%02d_%02d%02d%02d_%06ld",
            st_tm.tm_year + 1900,
            st_tm.tm_mon+1,
            st_tm.tm_mday,
            st_tm.tm_hour,
            st_tm.tm_min,
            st_tm.tm_sec,
            tv.tv_usec);

    return std::string(st_string);
}

const std::string   log_elapse(std::chrono::steady_clock::time_point start_time, std::chrono::steady_clock::time_point end_time)
{
    std::ostringstream os;
    os << std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();

    return os.str();
}



/******************************** Boost Log Implementation ********************************/

class kubot_severity_level_formatter
{
public:
    typedef void result_type;

public:
    explicit kubot_severity_level_formatter(std::string const &fmt) :
            useColor_(fmt == "color") {};

    void operator()(logging::formatting_ostream &strm, logging::value_ref<severity_level> const &value) const
    {
        if (!value)
            return;

        severity_level const &level = value.get();
        if (useColor_) {
            switch (level) {
                case trace:
                    strm << ANSI_CYAN << "TRAC" << ANSI_CLEAR;
                    break;
                case debug:
                    strm << ANSI_GREEN << "DBUG" << ANSI_CLEAR;
                    break;
                case info:
                    strm << ANSI_BLUE << "INFO" << ANSI_CLEAR;
                    break;
                case warning:
                    strm << ANSI_YELLOW << "WARN" << ANSI_CLEAR;
                    break;
                case error:
                    strm << ANSI_MAGENTA << "ERRR" << ANSI_CLEAR;
                    break;
                case fatal:
                    strm << ANSI_RED << "FATL" << ANSI_CLEAR;
                    break;
                default:
                    strm << (int) level;
            }
        } else {
            switch (level) {
                case trace:
                    strm << "TRAC";
                    break;
                case debug:
                    strm << "DBUG";
                    break;
                case info:
                    strm << "INFO";
                    break;
                case warning:
                    strm << "WARN";
                    break;
                case error:
                    strm << "ERRR";
                    break;
                case fatal:
                    strm << "FATL";
                    break;
                default:
                    strm << (int) level;
            }
        }
    }

private:
    bool useColor_;
};

class kubot_severity_level_formatter_factory : public logging::basic_formatter_factory<char, severity_level>
{
public:
    formatter_type  create_formatter(logging::attribute_name const &name, args_map const &args)
    {
        args_map::const_iterator it = args.find("format");
        if (it != args.end())
            return boost::phoenix::bind(kubot_severity_level_formatter(it->second), expr::stream,
                                        expr::attr<severity_level>(name));
        else
            return boost::phoenix::bind(kubot_severity_level_formatter("default"), expr::stream,
                                        expr::attr<severity_level>(name));
    }
};


class kubot_time_stamp_formatter_factory : public boost::log::basic_formatter_factory<char, boost::posix_time::ptime>
{
public:
    formatter_type create_formatter(boost::log::attribute_name const& name, args_map const& args)
    {
        args_map::const_iterator it = args.find("format");
        if (it != args.end())
            return boost::log::expressions::stream << boost::log::expressions::format_date_time<boost::posix_time::ptime>(boost::log::expressions::attr<boost::posix_time::ptime>(name), it->second);
        else
            return boost::log::expressions::stream << boost::log::expressions::attr<boost::posix_time::ptime>(name);
    }
};


/******************************** KubotLogHelper ********************************/
bool KubotLogHelper::init_done;
KubotLogHelper::log_impl    KubotLogHelper::log_impl_select = KubotLogHelper::log_impl::print_log;
std::string                 KubotLogHelper::default_channel = "kubot";
std::map<std::string, src::severity_channel_logger<severity_level, std::string>> KubotLogHelper::scl_map;
boost::shared_ptr<sinks::synchronous_sink<sinks::text_ostream_backend>> KubotLogHelper::console_sink;
logging::trivial::severity_level KubotLogHelper::console_log_level = logging::trivial::info;

src::severity_channel_logger<severity_level, std::string>& KubotLogHelper::GetLogger(std::string channel)
{
    auto it = scl_map.find(channel);
    if (it == scl_map.end())
    {
        scl_map.emplace(channel, src::severity_channel_logger<severity_level, std::string>(keywords::channel = channel));
    }

    return scl_map[channel];
};

bool KubotLogHelper::InitBoostLogEnvironment(const std::string config_file)
{
    return InitBoostLogEnvironment(config_file, "kubot");
}

bool KubotLogHelper::InitBoostLogEnvironment(const std::string config_file, const std::string channel)
{

    if (!boost::filesystem::exists("./log/"))
    {
        boost::filesystem::create_directory("./log/");
    }
    logging::add_common_attributes();

    logging::register_formatter_factory("Severity", boost::make_shared<kubot_severity_level_formatter_factory>());
    logging::register_formatter_factory("TimeStamp", boost::make_shared<kubot_time_stamp_formatter_factory>());
    logging::register_simple_filter_factory<severity_level, char>("Severity");
    logging::register_simple_filter_factory<std::string, char>("Channel");

    std::ifstream file(config_file);
    try
    {
        logging::init_from_stream(file);
    }
    catch (const std::exception& e)
    {
        std::cout << "init_logger is fail, read log config file fail. curse: " << e.what() << std::endl;
        exit(-2);
    }

    console_sink = logging::add_console_log();
    console_sink->set_filter(logging::trivial::severity >= console_log_level);

    auto console_format = logging::parse_formatter("[%TimeStamp(format=\"%Y-%b-%d %H:%M:%S.%f\")%] [%Severity(format=\"color\")%] [%Channel%] %Message%");
    console_sink->set_formatter(console_format);

    KubotLogHelper::log_impl_select = KubotLogHelper::log_impl::boost_log;
    KubotLogHelper::default_channel = channel;

    init_done = true;

    return true;
}

bool KubotLogHelper::InitPrintLogEnvironment()
{
    return InitPrintLogEnvironment("kubot");
}
bool KubotLogHelper::InitPrintLogEnvironment(const std::string channel)
{
    KubotLogHelper::log_impl_select = KubotLogHelper::log_impl::print_log;
    KubotLogHelper::default_channel = channel;

    return true;
}

int KubotLogHelper::SetConsoleSeverity(const std::string severity)
{
    if(!severity.compare("trace") || !severity.compare("Trace") || !severity.compare("TRACE")){
        console_sink->set_filter(logging::trivial::severity >= logging::trivial::trace);
    }else if(!severity.compare("debug") || !severity.compare("Debug") || !severity.compare("DEBUG")){
        console_sink->set_filter(logging::trivial::severity >= logging::trivial::debug);
    }else if(!severity.compare("info") || !severity.compare("Info") || !severity.compare("INFO")){
        console_sink->set_filter(logging::trivial::severity >= logging::trivial::info);
    }else if(!severity.compare("warning") || !severity.compare("Warning") || !severity.compare("WARNING")
             || !severity.compare("warn") || !severity.compare("Warn") || !severity.compare("WARN")){
        console_sink->set_filter(logging::trivial::severity >= logging::trivial::warning);
    }else if(!severity.compare("error") || !severity.compare("Error") || !severity.compare("ERROR")){
        console_sink->set_filter(logging::trivial::severity >= logging::trivial::error);
    }else if(!severity.compare("fatal") || !severity.compare("Fatal") || !severity.compare("FATAL")){
        console_sink->set_filter(logging::trivial::severity >= logging::trivial::fatal);
    }else{
        std::cout << "Unsupport severity[" << severity << "]" << std::endl;
        return -1;
    }

    std::cout << "Set console log severity to [" << severity << "]" << std::endl;

    return 0;
}