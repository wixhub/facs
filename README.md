# Micro Expression Engine 🔬🧠

> C++20 -> WebAssembly -> Angular 22

A high-performance, real-time facial telemetry and micro-expression analysis web application. The core detection engine is written in **C++20**, compiled to **WebAssembly (WASM)** via **Emscripten** and **Ninja**, and integrated into a modern **Angular** frontend using Signals and reactive components.

![Engine Status](https://img.shields.io/badge/Engine-C%2B%2B20%20%2F%20WASM-blue)
![Frontend](https://img.shields.io/badge/Frontend-Angular-red)
![License](https://img.shields.io/badge/License-MIT-green)

## 🌟 About the Project

**MicroExpression Engine** is a modern web-native research and R&D platform designed for the automated localization, tracking and quantitative evaluation of human facial expressions based on the **Facial Action Coding System (FACS)**.

This project is a modern reimagining and technological evolution of a [master's thesis](https://psychophysiology.pages.dev/project-details/masters-thesis) originally developed at **Bauman Moscow State Technical University (BMSTU)**. By bridging high-performance systems programming with modern web architecture, the engine eliminates subjective observer bias and manual coding bottlenecks in behavioral science, psychology and affective computing.

Because the core mathematical and computer vision algorithms run via **WebAssembly (WASM)** directly in the browser, the application requires **zero backend servers**, guarantees absolute user privacy (video streams never leave the client session) and delivers lightning-fast processing speeds.

---

## 🚀 Key Features

- **High-Performance WASM Engine:** Heavy matrix operations and facial landmark/geometry calculations run natively in the browser via C++20 compiled with Emscripten and Ninja, ensuring zero server-side latency.

- **Full FACS Telemetry Dashboard:** Measures 10 distinct Facial Action Units (AU1, AU2, AU4, AU5, AU6, AU9, AU12, AU15, AU17, AU25) with normalized intensities ranging from `0.0` to `1.0`.

- **Emotion & Valence Classification:** Evaluates dominant emotions and positive/negative valence scoring on-the-fly.

- **Dual Source Support:** Switch seamlessly between a pre-recorded demo video and a live webcam stream.

- **Modern Angular Architecture:** Built using **Angular (Signals)** within a standalone structure, ensuring reactive, predictable, and high-performance state management.

- **Client-Side Privacy:** All video processing happens locally on the user's device—no external server storage or cloud streaming required.

---

## 🛠️ Tech Stack

- **Core & Processing:** C++20, OpenCV / Custom Math Matrix Library

- **Compilation & Bridge:** Emscripten (C++ to WebAssembly, `emscripten/bind`), CMake, Ninja

- **Frontend:** Angular, Signals, TypeScript, HTML5 Canvas, SCSS

- **Computer Vision Pipeline**: Browser MediaPipe / Landmark detection feeding raw geometric coordinates into the C++ WASM engine.

- **Tooling:** CMake, Vite, npm

---

## 🏗️ Architecture Overview

```text
facs(micro-expression-engine)/
├── backend/               # C++ Computer Vision & Geometric Metric Engine
│   └── analyser.cpp       # C++ core source files and Emscripten bindings
├── frontend/              # Angular (Signals) + TypeScript + Canvas UI
│   ├── src/
│   │ ├── app/
│   │ │ ├── core/          # WASM loader service & state management
│   │ │ └── features/      # Camera feed, canvas overlay and real-time charts
│   │ └── styles.scss
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

## 📄 Demo Video Source

The default sample video used for testing is provided courtesy of [Pexels](https://www.pexels.com/) under the Pexels Free License.

## 📜 Academic Attribution & License

- **Scientific Basis:** Built upon computational paradigms for quantitative behavioral analysis, facial landmark tracking and pattern classification (PCA/HMM methodologies).

- **License:** Distributed under the MIT License. See `LICENSE` for more information.
