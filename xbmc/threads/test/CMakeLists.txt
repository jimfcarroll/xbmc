set(SOURCES TestConcurrencyMechanisms.cpp
            TestEvent.cpp
            TestExecutor.cpp
            TestSharedSection.cpp)

set(HEADERS TestHelpers.h)

core_add_test_library(threads_test)
