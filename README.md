# Micro Expression Engine 🔬🧠

> **High-performance real-time facial expression and micro-expression analysis platform, powered by native C++ compiled to WebAssembly (WASM) and Angular 22.**

---

## 🌟 About the Project

**MicroExpression Engine** is a modern web-native research and R&D platform designed for the automated localization, tracking and quantitative evaluation of human facial expressions based on the **Facial Action Coding System (FACS)**.

This project is a modern reimagining and technological evolution of a [master's thesis](https://psychophysiology.pages.dev/project-details/masters-thesis) originally developed at **Bauman Moscow State Technical University (BMSTU)**. By bridging high-performance systems programming with modern web architecture, the engine eliminates subjective observer bias and manual coding bottlenecks in behavioral science, psychology and affective computing.

Because the core mathematical and computer vision algorithms run via **WebAssembly (WASM)** directly in the browser, the application requires **zero backend servers**, guarantees absolute user privacy (video streams never leave the client session) and delivers lightning-fast processing speeds.

---

## 🚀 Key Features

- **WASM-Powered Performance:** Heavy matrix operations and facial landmark/geometry calculations run natively in the browser via C++ compiled with Emscripten.

- **FACS Quantitative Mapping:** Translates subtle facial muscle movements into precise, objective numerical metrics and action unit intensity scores.

- **Real-Time Visual Overlay:** Utilizes HTML5 Canvas to render active mimic zones, facial meshes and emotional timelines dynamically over live webcam or video feeds.

- **Modern Angular Architecture:** Built using **Angular (Signals)** within an enterprise-grade standalone structure, ensuring reactive, predictable and high-performance state management.

- **Client-Side Privacy:** All video processing happens locally on the user's device—no external server storage or cloud streaming required.

---

## 🛠️ Tech Stack

- **Core & Processing:** C++20, OpenCV / Custom Math Matrix Library

- **Compilation & Bridge:** Emscripten (C++ to WebAssembly, `emscripten/bind`)

- **Frontend:** Angular, Signals, TypeScript, HTML5 Canvas, SCSS

- **Tooling:** CMake, Vite, npm

---

## 🏗️ Architecture Overview

```text
facs(micro-expression-engine)/
├── backend/               # C++ Computer Vision & Geometric Metric Engine
│   ├── include/           # Header files (Facial zones, FACS calculators)
│   ├── src/               # Core algorithms (matrix operations, landmark mapping)
│   └── bindings.cpp       # Emscripten bindings (C++ to JS/WASM bridge)
├── frontend/              # Angular (Signals) + TypeScript + Canvas UI
│   ├── src/
│   │ ├── app/
│   │ │ ├── core/          # WASM loader service & state management
│   │ │ └── features/      # Camera feed, canvas overlay and real-time charts
│   │ └── styles/
│   └── package.json
├── CMakeLists.txt         # Emscripten build configuration
└── README.md

```

---

## 🚦 Getting Started & Building from Source

### Prerequisites

- **Emscripten SDK (emsdk)** installed and configured in your path.
- **Node.js** (v18+) & **npm**
- **CMake** (v3.20+)

### 1. Build the C++ Core to WebAssembly

```bash
# Create build directory for WASM target
mkdir build && cd build
emcmake cmake ..
emmake make
# This generates the compiled .wasm and glue .js files for the frontend

```

### 2. Run the Angular Frontend

```bash
cd frontend
npm install
npm start

```

Open your browser and navigate to `http://localhost:4200` to start analyzing micro-expressions in real time.

---

## 📜 Academic Attribution & License

- **Scientific Basis:** Built upon computational paradigms for quantitative behavioral analysis, facial landmark tracking and pattern classification (PCA/HMM methodologies).

- **License:** Distributed under the MIT License. See `LICENSE` for more information.
