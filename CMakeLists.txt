cmake_minimum_required(VERSION 3.28)
project(h1_malware)

set(CMAKE_CXX_STANDARD 14)

add_executable(h1_malware
        .idea/.gitignore
        .idea/h1_malware.iml
        .idea/modules.xml
        .idea/vcs.xml
        .idea/workspace.xml
        README.md
        self_injection.cpp)
