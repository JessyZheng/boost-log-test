#include <iostream>
#include "log.h"

int main(void)
{
    if (!KubotLogHelper::InitBoostLogEnvironment("boost-log.config", "master"))
        return false;
    KUBOT_LOG(info) << "test";
    KUBOT_LOG(info) << "test";
    KUBOT_LOG(info) << "test";
    KUBOT_LOG(info) << "test";
    return 0;
}