#!/bin/bash
set -e

g++ main.cpp \
    third_party/imgui/imgui*.cpp \
    third_party/imgui/backends/imgui_impl_glfw.cpp \
    third_party/imgui/backends/imgui_impl_opengl3.cpp \
    -Ithird_party/imgui \
    -Ithird_party/imgui/backends \
    -lglfw -lGL -ldl -lpthread -lX11 -lXrandr -lXi \
    -o task_management_app

echo "Build complete! Run ./task_manager_app"
