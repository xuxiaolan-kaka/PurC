include(PurCCommon)
include(target/PurC)
include(GoogleTest)

enable_testing()

# test_messages
PURC_EXECUTABLE_DECLARE(test_messages)

list(APPEND test_messages_PRIVATE_INCLUDE_DIRECTORIES
    ${PURC_DIR}/include
    ${PurC_DERIVED_SOURCES_DIR}
    ${PURC_DIR}
    ${CMAKE_BINARY_DIR}
    ${WTF_DIR}
)

PURC_EXECUTABLE(test_messages)

set(test_messages_SOURCES
    test_messages.cpp
)

set(test_messages_LIBRARIES
    PurC::PurC
    gtest_main
    gtest
    pthread
)

PURC_COMPUTE_SOURCES(test_messages)
PURC_FRAMEWORK(test_messages)
GTEST_DISCOVER_TESTS(test_messages DISCOVERY_TIMEOUT 10)

