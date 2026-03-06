@echo off
echo [1/4] Setting up Python Virtual Environment...
python -m venv .venv
call .venv\Scripts\activate

echo [2/4] Installing dependencies...
pip install -r requirements.txt

echo [3/4] Generating Test Laboratory...
python test_suite_creation.py

echo [4/4] Compiling C++ Payload (Release), Takes a moment...
cmake -S . -B build
cmake --build build --config Release

echo.
echo =====================================================
echo BUILD COMPLETE! 
echo Run the payload using: .\build\Release\Payload.exe
echo =====================================================
pause