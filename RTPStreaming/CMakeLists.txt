cmake_minimum_required(VERSION 3.10)

# プロジェクト名とバージョンを設定
project(RTPStreaming VERSION 1.0)

# C++言語の標準を設定
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# GStreamerを探す
find_package(PkgConfig REQUIRED)
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0)

# 実行可能ファイルの生成とソースファイルのリスト
add_executable(main main.cpp)

# インクルードディレクトリとリンクするライブラリの設定
target_include_directories(main PRIVATE ${GSTREAMER_INCLUDE_DIRS})

# プラットフォームごとのライブラリパスとリンク設定
if(APPLE)
    # macOSの場合のライブラリパスとリンク設定
    target_link_directories(main PRIVATE /usr/local/lib)
elseif(UNIX)  # 他のUnix系OS（Linuxを含む）
    # Linuxの場合のライブラリパスとリンク設定
    target_link_directories(main PRIVATE /usr/lib/x86_64-linux-gnu)
endif()

# ライブラリのリンク
target_link_libraries(main PRIVATE ${GSTREAMER_LIBRARIES})

add_subdirectory(sender)
add_subdirectory(receiver)