cmake_minimum_required(VERSION 3.11)
project(notifyd CXX)

find_package(spdlog REQUIRED)

add_subdirectory(lib/httpxx)

set(SOURCES
	src/main.cc
)



add_executable(notifyd ${SOURCES})
target_link_libraries(notifyd PRIVATE spdlog::spdlog httpxx)

target_include_directories(notifyd
    PRIVATE 
        ${PROJECT_SOURCE_DIR}/include
	${PROJECT_SOURCE_DIR}/lib/httpxx
)
