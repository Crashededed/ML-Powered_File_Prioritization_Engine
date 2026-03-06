# 🚀 ML-Powered File Prioritization Engine
### *High-Precision Data Exfiltration via Metadata Analysis*

![C++](https://img.shields.io/badge/Language-C%2B%2B17-blue.svg) ![Python](https://img.shields.io/badge/Research-Python%203.9-green.svg) ![License](https://img.shields.io/badge/License-MIT-yellow.svg)

## 🎯 Project Overview

In modern cyber operations, the challenge of data exfiltration is often defined by constraints: limited time windows, restrictive network bandwidth, and the constant threat of detection by behavioral-based security systems. Traditional methods that rely on keyword matching or deep-content scanning are often too slow and create a massive CPU footprint that triggers modern Endpoint Detection and Response (EDR) alerts.

This project introduces a sophisticated alternative: **A metadata-centric machine learning engine designed to rank and prioritize High-Value Targets (HVTs)** with zero file-content interaction, designed for maximum stealth and precision.

By shifting the focus from file content to file metadata and directory structure, our system identifies high-value sensitive files while remaining entirely lightweight and fast. Using machine learning to analyze filenames, folder hierarchies, and modification histories, the engine predicts file value with high accuracy without ever needing to open a single document.

---

## 🛠️ Key Technical Pillars 

- #### Multi-Target Classification
    The engine is built for flexibility, featuring specialized models for **Finance**, **HR**, and **IT** targets to ensure the most relevant assets are surfaced immediately.

- #### Zero-Dependency Portability
    The entire inference engine is compiled into a lightweight C++ binary with no external requirements - no Python environment, PyTorch libraries, or external DLLs are needed for deployment. This ensures a negligible forensic footprint and allows the payload to perform real-time scoring using only the pre-trained weights.

- #### Trained on Real-World Data
    The models were developed using a high-entropy dataset constructed from authentic corporate filesystem snapshots, public US government documents (GovDocs), and diverse GitHub repositories. By training on these real-world naming conventions and directory hierarchies, the system learns to distinguish between mundane system files and genuine high-value assets with high precision.

## 🏗️ Project Architecture

- #### Python Training Module
    Employs feature engineering and **Logistic Regression** to learn the distinction between critical signals and system "noise" from a custom-built dataset.
    This module produces the weights and biases for each model, which are then exported directly into a C++ header file.

- #### C++ Payload
    A lightweight, standalone binary that traverses target drives and performs real-time inference. It extracts metadata on the fly to generate probability scores for each file, enabling the immediate ranking and prioritization of sensitive assets. It requires **zero external libraries or Python environments**, simulating a real-world, self-contained deployment.

---

## 📊 Performance & Results

The system successfully bridges the gap between theoretical machine learning and operational cybersecurity requirements:

- **High Recall:** Achieved detection rates ranging from **0.82 (IT)** to **0.89 (Finance)**, even in highly imbalanced environments where sensitive files are statistically rare.

- **Sub-Millisecond Speed:** The **C++ payload** delivers **sub-millisecond inference speeds per file**, ensuring minimal dwell time on a target system.

- **Stealth-First Design:** Maintains a **negligible CPU footprint** and effectively filters out system noise, making it a viable and stealthy alternative to traditional content-scanning methods.

### Snippet from the Payload - Ranking according to the HR model:

```
ANALYZING TARGET: HR

--- TOP 3 FILES ---
Score: 0.9896 | .\test_data\HR\Recruiting\Candidates\Employee_Benefits_2023.xlsx
Score: 0.9684 | .\test_data\HR\Recruiting\Candidates\Onboarding_Checklist_Product.xlsx
Score: 0.9514 | .\test_data\HR\Recruiting\Candidates\Interview_Notes_John_Johnson.docx
```

## 🧠 Technical Deep Dive: Feature Engineering

This section explains the "secret sauce" that allows the model to identify sensitive files without looking at their contents:

#### 1. Structural Text Hashing (Quadgrams)

To "read" file paths without a lookup table, the system breaks strings into overlapping **4-character chunks called quadgrams**. These are processed using the **MurmurHash3** algorithm and mapped to a fixed **2048-feature vector**. This allows the model to mathematically recognize sensitive patterns like **budg** (from *budget*) or **payr** (from *payroll*) regardless of the surrounding text.

#### 2. Temporal Decay Logic

Time is a primary indicator of relevance. We implement a **hyperbolic decay function** to calculate a "Recency Score".
This causes a **"half-life" effect** where a file exactly one year old receives a score of **0.5**. The model learns to prioritize fresh, active data over outdated noise.

#### 3. Metadata Heuristics

The engine analyzes several secondary signals to refine its skeptical baseline:

- **Path Depth:** Identifies how deep a file is nested; human-created data tends to be shallower than system-generated junk.
- **Log-Scaled Size:** We use `log1p` of the file size to prevent massive files from skewing the model's weights while maintaining relative scale.
- **Target-Specific Extensions:** Binary indicators flag formats like `.xlsx` for Finance or `.pem` for IT.

## ⚙️ Payload Features - Performance-Oriented Engineering:

- **Static Memory Allocation** - Uses priority queues to maintain only the top $N$ files per focus, preventing memory spikes when scanning millions of files.

- **W-Character Support** - Fully compatible with **UTF-16 filenames** (essential for diverse environments containing Hebrew, Russian, or specialized symbols).

- **Sub-Millisecond Dwell Time**  - By avoiding file I/O (reading content), the payload can scan an entire workstation in seconds, significantly reducing the window for EDR detection.

## 📂 Repository Structure
```
├── test_suite_creation.py      # Script to rebuild the test environment
├── master_training_dataset.csv # The labeled dataset used for training 
├── CMakeLists.txt              # Build configuration for the C++ payload
├── requirements.txt            # Python dependencies for the training module
├── src/
│   ├── main.cpp                # Entry point and performance benchmarking 
│   ├── ModelScorer.cpp         # ML inference and feature engineering
│   ├── FileScanner.cpp         # Recursive filesystem traversal logic 
│   └── ModelTraining.ipynb     # End-to-end ML training and weight export pipeline 
└── include/
    ├── ModelWeights.h          # Exported model weights and bias constants 
    ├── ModelScorer.h           # Headers for the scoring engine
    └── FileScanner.h           # Headers for the scanner
```

## 🚀 Quick Start Guide

This project requires **Python 3.9+** (for research and lab generation) and **CMake** (for payload compilation).

> **Note:** If you have Visual Studio installed with "Desktop development with C++," you likely already have CMake.  
>  Check by running `cmake --version` in your terminal.
> 
---
## ⚡ Option A: Quick Setup (Windows)
If you are on Windows and have Python and Visual Studio (C++ Build Tools) installed, you can initialize the entire environment, generate the test directory, and compile the payload by running:

```PowerShell
.\quick_setup.bat
```
---
## 🛠️ Option B: Manual Setup

### 1. Environment Initialization

**Bash**
```bash
# Clone the repository
git clone https://github.com/your-username/CyberSecurityMLproject
cd CyberSecurityMLproject

# Create a virtual environment and install dependencies
python -m venv .venv

# Windows:
.venv\Scripts\activate

# Linux/WSL:
source .venv/bin/activate

pip install -r requirements.txt
```

### 2. Training vs. Research
- **For Research**: Open *src/ModelTraining.ipynb* in VS Code/Jupyter and select the "Python (CyberML)" kernel to view charts and training metrics.

- **For Production**: To re-train models and update include/ModelWeights.h quickly, run the script:

```Bash
python src/train_models.py
```

### 3. Build the Test Environment

Before running the payload, you must generate the test suite. This script builds a realistic, high-entropy filesystem structure using the metadata distributions identified in our research.

By default, this creates a ./test_data directory containing ~3,000 files. You can customize the output location by specifying the --path argument:

```bash
# Default:
python test_suite_creation.py

# Custom: Generate the test environment in a specific directory
python test_suite_creation.py --path "C:\TestData"
```

### 4. Compile the Payload:

The C++ payload is designed for **zero-dependency execution**. We utilize **CMake** to generate an optimized **Release binary**, which ensures maximum inference speed and removes debug overhead:

```bash
cmake -S . -B build
cmake --build build --config Release
```

## Running the Payload:

After finishing the setup, Run the standalone binary to begin the prioritized scan of the generated test data.

By default, it will scan the `./test_data` directory and output the top 10 files for each target context (Finance, HR, IT). You can specify a different path or number of top files using command-line arguments:


>**Note**: Ensure you are running the command from the project root.

```bash
# Default: Scans ./test_data and shows the Top 10 results
.\build\Release\Payload.exe

# Custom: Scan a specific directory and show the Top 5 results
.\build\Release\Payload.exe --path "C:\TestData" --top 5```

## ⚠️ Research Context

This project is a **defensive research and academic exploration** of how metadata patterns can reveal sensitive data locations within large filesystems.  
The techniques demonstrated here are intended for **cybersecurity research, data discovery, and digital forensics**, not unauthorized access.

## 👨‍💻 Author
**Ilan H. Rozinko** *Computer Science Student @ Ben-Gurion University* Specializing in Data Science.

