#include <gtest/gtest.h>
#include <log.h>

TEST(DISABLED_TestLog, should_use_format_string)
{
    LOG_LOG("hello{} {}", "world", "bb");
}
