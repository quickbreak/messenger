cmake_minimum_required(VERSION 3.10)

project(AUTH LANGUAGES CXX)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
# эта строка бесполезна, потому что я установил crow помимо способа через vcpkg ещё вручную
# (гайд с оф.сайта: git clone, cd Crow && mkdir bulid && cd build, cmake .., make install), 
# и теперь cmake сам находит его предположитель в /opt/vcpkg/include
# а libpqxx я установил с apt-get install -y libpqxx-dev
# include_directories(/opt/vcpkg/installed/x64-linux/include) 
# find_package(Crow)
# find_package(PkgConfig REQUIRED)
# pkg_check_modules(PQXX REQUIRED libpqxx)

# include_directories(${PQXX_INCLUDE_DIRS})
# link_directories(${PQXX_LIBRARY_DIRS})

include_directories(/auth_build/Crow/include)

set (SRC
    ./src/crow_http.cpp
    ./src/auth.cpp
)

add_executable (http_server ${SRC})

target_link_libraries(http_server PRIVATE pqxx pq)

# g++ -o http_server crow_http.cpp auth.cpp -lpqxx -lpq
