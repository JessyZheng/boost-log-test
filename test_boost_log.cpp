#include <iostream>
#include <sstream>
#include "log.h"
#include<string> 

int main(void)
{
    if (!KubotLogHelper::InitBoostLogEnvironment("boost-log.config", "master"))
        return false;
    KUBOT_LOG(info) << "test";

    uint8_t crc8_tmp[3] = {0xAB, 0xCD, 0x9c};


    std::stringstream s;
    for(int i=0; i< 3; ++i)
    {
        s << std::hex << (int)(crc8_tmp[i]);
        s << " ";
    }
    s << std::hex << 0xDE;
    KUBOT_LOG(info) << "[execution result crc8_tmp ] " << s.str();

    std::ostringstream oss;
    for(int i=0; i< 3; ++i)
    {
        oss << std::hex << crc8_tmp[i];
    }
    oss << std::hex << (uint8_t)(0xDE);
    KUBOT_LOG(info) << "[execution result crc8_tmp2 ] " << oss.str();
    return 0;
}