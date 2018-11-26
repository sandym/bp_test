cmake_minimum_required( VERSION 3.10 )
project( bp_test )

set( CMAKE_CXX_STANDARD 11 )

# ./bootstrap.[bat | sh]
# ./b2 --prefix=... install

set( BOOST_ROOT ${CMAKE_CURRENT_LIST_DIR}/boost )
set( Boost_USE_STATIC_LIBS on )
find_package( Boost REQUIRED COMPONENTS thread system filesystem regex )

add_executable( bp_test main.cpp )

target_link_libraries( bp_test
Boost::thread
Boost::system
Boost::filesystem
Boost::regex
)

if( WIN32 )
target_compile_definitions( bp_test PRIVATE _WIN32_WINNT=0x0501 )
endif()
