# Smart Eye-Dropper (Team 7)

**Smart Eye-Dropper** is a C++ desktop application that extracts dominant colors from an image using the K-Means clustering algorithm and recommends a matching color palette based on a selected theme (Fashion, Interior, Design).

## Environment and Requirements

* **Language:** C++17 Standard or higher
* **Libraries:**
    * [OpenCV 4.x](https://opencv.org/) (Required)
    * [cvui](https://github.com/Dovyski/cvui) (Header-only GUI library, source included)
* **Build System:** CMake 3.16 or higher
* **Compiler:**
    * Windows: MSVC (Visual Studio 2019/2022)
    * Linux: GCC / G++

## Project Structure

```text
SmartEyeDropper/
├── CMakeLists.txt       # CMake build configuration
├── README.md            # Project documentation
└── src/
    ├── cvui.h           # GUI library header
    ├── main.cpp         # Main entry point and GUI logic
    ├── ImageProcessor.h # Image processing and K-Means logic declaration
    ├── ImageProcessor.cpp
    ├── PaletteMatcher.h # Theme-based palette matching logic declaration
    └── PaletteMatcher.cpp
```

## Installation and Build Guide (Linux)

This guide assumes an Ubuntu/Debian-based system.

### 1. Install Dependencies
Open a terminal and install the necessary build tools and the OpenCV library.

```bash
sudo apt update
sudo apt install build-essential cmake libopencv-dev
```

### 2. Build Project

Navigate to the project root directory and execute the following commands.
```bash
mkdir build
cd build
cmake ..
make
```

### 3. Execute Application
```bash
./SmartEyeDropper
```

## Installation and Build Guide (Windows)

This guide assumes the use of Visual Studio 2019 or 2022.

### 1. Prerequisites

* **Install OpenCV:**
  Download and install the Windows executable (.exe) from the official website.
  Example installation path: C:\opencv

* **Install CMake:**
  Download and install from the official website.

### 2. Build Configuration

1. Open the CMakeLists.txt file and verify that the OpenCV_DIR path matches your installation directory.
   ```bash
   # Example
   set(OpenCV_DIR "C:/opencv/build")
   ```

2. Open the project folder in Visual Studio (Open as CMake Project).

3. Select **Build -> Build All** from the top menu.

### 3. Runtime Configuration (DLL Error Resolution)

After building, if you encounter an error stating opencv_world4xx.dll was not found:

1. Navigate to the OpenCV installation directory: build\x64\vc16\bin.
2. Copy the opencv_world4xx.dll (Release mode) or opencv_world4xxd.dll (Debug mode) file.
3. Paste the file into the directory where the built executable (.exe) is located.

## Usage

Control the application using the GUI panel displayed upon launch.

### 1. Select Theme
* Choose one of **Fashion**, **Interior**, or **Design** from the left panel.
* The recommendation algorithm changes based on the selected theme.

### 2. Settings
* Adjust the **Clusters (K)** slider to set the number of dominant colors to extract from the image. (Default: 5)

### 3. Upload Image
* Click the **Open Image File** button.
* Select an image file (.jpg, .png, .bmp) to analyze.
* The analysis begins automatically once the image is loaded.

### 4. View Results & Re-analyze
* The original image and analysis results overlay are displayed on the right panel.
* **Extracted Colors:** Dominant colors extracted via K-Means clustering with their Hex codes.
* **Theme Recommendation:** The most suitable color palette recommended based on the theme database.
* To change the K value, adjust the slider and click the **Re-analyze (K)** button to immediately update the results.

---

## Troubleshooting

* **Compilation Error (Header not found):** Ensure that the cvui.h file exists in the src/ directory.
* **Windows Runtime Error:** Verify that the OpenCV DLL file is located in the same directory as the executable.
* **Linux CMake Error:** Ensure that the libopencv-dev package is correctly installed.
