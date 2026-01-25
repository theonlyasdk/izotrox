#!/usr/bin/env python3
import sys
import os

# Add parent directory to path for package import
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from theme_editor.__main__ import main

if __name__ == "__main__":
    main()
