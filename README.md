# 🚀 ML-Powered File Prioritization Engine
### *High-Precision Data Exfiltration via Metadata Analysis*

![C++](https://img.shields.io/badge/Language-C%2B%2B17-blue.svg) ![Python](https://img.shields.io/badge/Research-Python%203.9-green.svg) ![License](https://img.shields.io/badge/License-MIT-yellow.svg)

## 🎯 Project Overview

In modern cyber operations, the challenge of data exfiltration is often defined by constraints: limited time windows, restrictive network bandwidth, and the constant threat of detection by behavioral-based security systems. Traditional methods that rely on keyword matching or content scanning are often slow and create a massive CPU footprint that triggers modern Endpoint Detection and Response (EDR) alerts.

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

- **Sub-Millisecond Speed:** The **C++ payload** delivers **sub-millisecond inference speeds per file**, ensuring minimal dwell time on a network.

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

- **Static Memory Allocation:**   
     Uses priority queues to maintain only the top $N$ files per focus, preventing memory spikes when scanning millions of files.

- **W-Character Support:**   
 Fully compatible with **UTF-16 filenames** (essential for diverse environments containing Hebrew, Russian, or specialized symbols).

- **Sub-Millisecond Dwell Time:**    
 By avoiding file I/O (reading content), the payload can scan an entire workstation in seconds, significantly reducing the risk of detection.

- **Standalone Execution:**   
 The payload is compiled into a single executable with no external dependencies, ensuring it can run on any Windows machine without leaving a trace of Python or C++ runtime libraries.

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

# 🚀 Quick Start Guide

This project is designed for **Windows** and requires the following tools to build and run:

1. **Python 3.9+**: Available from the [official Python website](https://www.python.org/downloads/).
2. **Visual Studio C++ Build Tools**: Required to compile the C++ payload using CMake.  
   * You must install the **"Desktop development with C++"** workload via the [*Build Tools for Visual Studio*](https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2026) Installer. This includes the necessary MSVC compiler and CMake.
   
You can run the project either through **Visual Studio Code** or entirely from the **terminal**.


## 💻 Option A — VS Code Workflow (Recommended):

If you use **Visual Studio Code**, setup is streamlined for both Python and C++ development.

### 1. Install Required Extensions:

From the VS Code Marketplace, install:

- **Python** (`ms-python.python`)
- **C/C++** (`ms-vscode.cpptools`)
- **CMake Tools** (`ms-vscode.cmake-tools`)
- **Jupyter** (optional, for notebook support)

You can install all the necessary VS Code extensions at once by running this command in your terminal:

```powershell
code --install-extension ms-python.python --install-extension ms-vscode.cpptools --install-extension ms-vscode.cmake-tools --install-extension ms-toolsai.jupyter
```


### 2. Clone the Repository:

Open the integrated VS Code terminal and run:

```powershell
git clone https://github.com/Crashededed/ML-Powered_File_Prioritization_Engine
cd ML-Powered_File_Prioritization_Engine
code .
```
> **Note:** This will open the project in a new VS Code window. Close the old one and use the terminal in the new window.
### 3. Set Up the Python Environment:

In the integrated VS Code terminal run:

```powershell
python -m venv .venv
.\.venv\Scripts\activate
pip install -r requirements.txt
```

> **Note:** If you encounter a PowerShell execution policy error, run the following command:
> ```powershell
> Set-ExecutionPolicy Unrestricted -Scope CurrentUser
> ```
> Alternatively, you can switch your VS Code terminal profile to **Command Prompt**.

After activation, set the interpreter in VS Code:

1. Open the **Command Palette** (`Ctrl+Shift+P`)
2. Run **Python: Select Interpreter**
3. Choose the `.venv` interpreter

### 4. Generate the Test Environment
Before running the payload, you must generate the test suite. This script builds a realistic, high-entropy filesystem structure using the metadata distributions identified in our research.

Create the test data directory:

```powershell
# Default - generates a ./test_data directory in the project root:
python test_suite_creation.py
# Or specify a custom path:
python test_suite_creation.py --path "C:\TestData"
```

> ⚠️ **Warning:** The script will clear any existing data in the target directory, make sure to specify a path that is safe to overwrite.

### 5. Build the C++ Payload

1. Open the **Command Palette** (`Ctrl+Shift+P`)
2. Run **CMake: Scan for Kits** 
3. Run **CMake: Select a Kit** and choose your installed compiler, MSVC from Visual Studio
4. Run **CMake: Select Variant** and choose **Release**
5. Run **CMake: Configure**
6. Run **CMake: Build**

> ⚠️ **Troubleshooting:** If MSVC does not appear in the list of available kits (a known VS Code bug with certain Windows locales) or if you encounter any configuration errors, skip the remaining steps here and follow **Option B — Pure Terminal Workflow** below.

## ⌨️ Option B — Pure Terminal Workflow (Windows):

Because standard Windows terminals (like **CMD** or **PowerShell**) typically do not know where the **MSVC C++ compiler** is located, you must use Microsoft's specialized developer terminal.

### 1. Open a Visual Studio Developer Terminal:

Search your Windows **Start Menu** for one of the following and open:

- **Developer Command Prompt for VS**
- **x64 Native Tools Command Prompt**
- **Developer PowerShell for VS**

> Do **not** use a regular CMD or PowerShell window.

### 2. Setup & Compile:

Navigate to your desired directory inside the Developer Prompt and run the following:

```powershell
git clone https://github.com/Crashededed/ML-Powered_File_Prioritization_Engine
cd ML-Powered_File_Prioritization_Engine

# Initialize the Python environment and install dependencies
python -m venv .venv
.\.venv\Scripts\activate
pip install -r requirements.txt

# Generate test dataset (default: ./test_data)
python test_suite_creation.py
# Or:
python test_suite_creation.py --path "C:\TestData"

# Build C++ payload
cmake -S . -B build
cmake --build build --config Release
```

## 🧠 Re-Training the Models (Optional)

If you wish to customize feature engineering, adjust class weights, or train on your own dataset, you can rerun the training pipeline.

The training module uses `master_training_dataset.csv` to fit Logistic Regression models for each target context and exports updated weights and biases to `include/ModelWeights.h` for the C++ payload.

You can interact with the training module in two ways:

### Option 1 - Via Jupyter Notebook:
For a deep dive into the data science process, open the Jupyter Notebook. This Notebook contains the training process, provides visuals and detailed F1/Recall metrics for each model.

1. Open `src/ModelTraining.ipynb` in VS Code.
2. Set the active kernel to your `.venv` environment.
3. Run cells sequentially to train models and export weights.

### Option 2 - Via Terminal:
If you only want to quickly retrain the models and regenerate the C++ header file, run this Python script from your terminal:

```powershell
python src/train_models.py
```

> **⚠️ Important:** After retraining, you must recompile the C++ payload (using **CMake: Build**) to include the updated weights in the executable.

## ▶️ Running the Payload:

Once compiled (via either method), you can run the standalone binary from any standard terminal. By default, it will scan the ./test_data directory and output the top 10 files for each target context (Finance, HR, IT). 

After building, run the binary from the project root. You can specify a different path or number of top files using command-line arguments:

```powershell
# Default: Scan ./test_data and show Top 10 results
.\build\Release\Payload.exe

# Custom: Scan a specific directory and show Top 5 results
.\build\Release\Payload.exe --path "C:\TestData" --top 5
```

> **Note:** Depending on your system's default CMake generator (Ninja or VS), the output path may differ. If you don't find the executable in `.\build\Release\`, check `.\build\` for a `Payload.exe` file.

## 💭 How to interpret the results:
The output will display the target context (e.g., HR) followed by a ranked list of files with their corresponding scores. Higher scores indicate a higher likelihood of being a sensitive file based on the metadata analysis.

```plaintext
ANALYZING TARGET: GENERAL       

--- TOP 10 FILES ---
Score: 0.9753 | .\test_data\Fin\Temp\Forecast_Mar2023.xlsx
Score: 0.9581 | .\test_data\Users\User_1\AppData\Local\Temp\f8a2c1.key
Score: 0.9465 | .\test_data\Users\nholmes\OneDrive - Corp\Forecasts\FY2025_R&D_Budget_v2.xlsx
Score: 0.9462 | .\test_data\HR\Recruiting\Candidates\Emily_Resume_2023.docx
Score: 0.9434 | .\test_data\Users\arthur13\Documents\Reporting\Expense_Accruals_2025.csv
...
```

It is a good idea to treat the scores relatively rather than absolutely, as the system is designed to prioritize files within the context of the target focus rather than provide a binary classification. 

**The Order is more important than the raw number.** The engine is designed to ensure that if an operator only has time to exfiltrate 10 files, those 10 are the most statistically significant assets on the drive.

## ❗ Checking whether the payload is standalone:
The payload is compiled with static runtime linking (/MT). This means that all necessary C++ runtime components are included within the executable itself, eliminating the need for external dependencies.

To verify the Zero-Dependency nature of the Payload, we can confirm that it does not rely on external C++ runtimes or Python DLLs.

On Windows, you can use the **Developer Command Prompt for VS** (or other variants mentioned in Option B) and run:

```powershell
#cd into the project root
dumpbin /DEPENDENTS .\build\Release\Payload.exe

#Output:
Dump of file .\build\Release\Payload.exe

File Type: EXECUTABLE IMAGE

  Image has the following dependencies:

    KERNEL32.dll

    Summary...
```

A truly standalone binary will show only *KERNEL32.dll* as a dependency, with no references to Python or C++ runtime libraries. *KERNEL32.dll* is a core Windows library that provides essential system functions and is always present on Windows systems. It does not indicate an external dependency.

## ⚠️ Research Context

This project is a **defensive research and academic exploration** of how metadata patterns can reveal sensitive data locations within large filesystems.  The techniques demonstrated here are intended for **cybersecurity research, data discovery, and digital forensics**, not unauthorized access.

## 👨‍💻 Author
**Ilan H. Rozinko** *Computer Science Student @ Ben-Gurion University* Specializing in Data Science.

