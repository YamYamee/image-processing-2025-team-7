mkdir -p build
cd build
cmake ..
cmake --build . -- -j
cp color_palette_extractor ../
