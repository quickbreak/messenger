cmake_minimum_required(VERSION 3.10)

project(CHATS LANGUAGES CXX)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

# include_directories(/opt/vcpkg/installed/x64-linux/include)
find_package(Boost REQUIRED COMPONENTS json)

set (SRC
    ./src/main.cpp
    ./src/web_socket_server.cpp
    ./src/web_socket_session.cpp
    ./src/bimap.cpp
    ./src/connector.cpp
)

add_executable(msg_server ${SRC})

target_link_libraries(msg_server Boost::json pqxx pq)

# set(BOOST_INCLUDE_DIRS /opt/vcpkg/installed/x64-linux/include)
# set(BOOST_LIB_DIR /opt/vcpkg/installed/x64-linux/lib)

# # Настройки для Boost.JSON
# target_include_directories(chat_server PRIVATE ${BOOST_INCLUDE_DIRS})
# target_link_directories(chat_server PRIVATE ${BOOST_LIB_DIR})
# target_link_libraries(chat_server PRIVATE boost_json)

# # Настройки для pqxx (через apt)
# target_link_libraries(chat_server PRIVATE pqxx pq)

# target_link_libraries(chat_server PRIVATE pqxx pq)

# target_link_directories(chat_server PRIVATE /opt/vcpkg/installed/x64-linux/lib)
# target_link_libraries(chat_server PRIVATE boost_json)
