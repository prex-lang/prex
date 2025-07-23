clang++-17 -std=c++23 \
    $(find . -name '*.cpp') \
    `llvm-config-18 --cxxflags --ldflags --system-libs --libs core mc support target` \
    -o bin/prex
