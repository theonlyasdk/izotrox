#!/usr/bin/env python3
import sys
from .cli import run_cli
from .gui import run_gui

def main():
    if len(sys.argv) < 2:
        # We run the GUI if we haven't got any args
        try:
            run_gui()
        except Exception as e:
            print(f"Error launching GUI: {e}")
            sys.exit(1)
    else:
        run_cli(sys.argv[1:])

if __name__ == "__main__":
    main()
