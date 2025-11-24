cmake_minimum_required(VERSION 3.16)
project(SmartEyeDropper LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# =========================================================
# 1. OS-specific OpenCV Path Configuration
# =========================================================
if(WIN32)
    # Windows: Set user-specific path (Keep your local path)
    set(OpenCV_DIR "C:/Users/cartc/Downloads/opencv/build")
else()
    # Linux: Usually installed in system paths, so manual path setting is often unnecessary.
    # (If needed, you can add set(OpenCV_DIR "/usr/local/share/opencv4") here)
endif()

# Find OpenCV package (Common)
find_package(OpenCV REQUIRED)

# =========================================================
# 2. Register Executable and Source Code
# =========================================================
add_executable(SmartEyeDropper
    src/main.cpp
    src/ImageProcessor.h
    src/ImageProcessor.cpp
    src/PaletteMatcher.h
    src/PaletteMatcher.cpp
)

# =========================================================
# 3. OS-specific Library Linking
# =========================================================
if(WIN32)
    # Windows: Link OpenCV + Comdlg32 (For file explorer dialog)
    target_link_libraries(SmartEyeDropper PRIVATE ${OpenCV_LIBS} Comdlg32)
else()
    # Linux: Link OpenCV only (File explorer logic handles differently or is excluded)
    target_link_libraries(SmartEyeDropper PRIVATE ${OpenCV_LIBS})
endif()