cmake_minimum_required(VERSION 3.20)
project(MicroExpressionEngineWasm)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source file path
set(SOURCES backend/analyzer.cpp)

# Define the WebAssembly target executable
add_executable(analyzer ${SOURCES})

# Emscripten-specific linker flags for WASM generation
target_link_options(analyzer PRIVATE
    "SHELL:--bind"                     # Enable emscripten/bind support for C++ to JS/TS bindings
    "SHELL:-s WASM=1"                  # Compile to WebAssembly format
    "SHELL:-s ALLOW_MEMORY_GROWTH=1"   # Allow dynamic memory allocation growth
    "SHELL:-s MODULARIZE=1"            # Wrap output in a factory function (e.g., createMicroExpressionModule)
    "SHELL:-s EXPORT_ES6=1"            # Export as an ES6 module compatible with modern frontend tooling
    "SHELL:-s ENVIRONMENT=web,worker"  # Target web browsers and web workers
    "SHELL:-O3"                        # High optimization level for performance
)

# Output configuration (generates analyzer.js and analyzer.wasm in the build directory)
set_target_properties(analyzer PROPERTIES
    OUTPUT_NAME "analyzer"
    SUFFIX ".js"
)