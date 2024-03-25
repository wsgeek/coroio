cmake_minimum_required(VERSION 3.22)
if (EXISTS ${CMAKE_BINARY_DIR}/local.cmake)
include(${CMAKE_BINARY_DIR}/local.cmake)
endif ()

project(netlib)
find_package(PkgConfig REQUIRED)
pkg_check_modules(CMOCKA REQUIRED cmocka)
if (LINUX)
pkg_check_modules(URING REQUIRED liburing)
endif ()
pkg_check_modules(OPENSSL openssl)
enable_testing()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(coroio
  coroio/socket.cpp
  coroio/sockutils.cpp
  coroio/poll.cpp
  coroio/select.cpp
  coroio/epoll.cpp
  coroio/uring.cpp
  coroio/kqueue.cpp
  coroio/resolver.cpp
  coroio/ssl.cpp
  )

target_include_directories(coroio PUBLIC ${URING_INCLUDE_DIRS} ${OPENSSL_INCLUDE_DIRS})
target_link_directories(coroio PUBLIC ${URING_LIBRARY_DIRS} ${OPENSSL_LIBRARY_DIRS})
target_link_libraries(coroio PUBLIC ${URING_LIBRARIES} ${OPENSSL_LIBRARIES})

target_include_directories(coroio PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})


macro(target name source)
  add_executable(${name} ${source})
  target_link_libraries(${name} coroio)
endmacro()

macro(ut name source)
  add_executable(ut_${name} ${source})
  target_include_directories(ut_${name} PRIVATE ${CMOCKA_INCLUDE_DIRS})
  target_link_directories(ut_${name} PRIVATE ${CMOCKA_LIBRARY_DIRS})
  target_link_libraries(ut_${name} PRIVATE coroio ${CMOCKA_LIBRARIES})

  add_test(NAME ${name} COMMAND ${CMAKE_CURRENT_BINARY_DIR}/ut_${name})
  set_tests_properties(${name} PROPERTIES ENVIRONMENT "CMOCKA_MESSAGE_OUTPUT=xml;CMOCKA_XML_FILE=${name}.xml")
endmacro()

target(coroio_timers examples/timers.cpp)
target(coroio_echotest examples/echotest.cpp)
target(coroio_echoserver examples/echoserver.cpp)
target(coroio_echoclient examples/echoclient.cpp)
target(coroio_sslechoclient examples/sslechoclient.cpp)
target(coroio_sslechoserver examples/sslechoserver.cpp)
target(coroio_resolver examples/resolver.cpp)
target(coroio_bench examples/bench.cpp)

ut(coroio_tests tests/tests.cpp)
