#!/usr/bin/env python3
import sys
from pathlib import Path
from .core import ThemeEditor, get_theme_dir

def print_usage():
    print("Usage: theme_editor <command> [args...]")
    print("\nCommands:")
    print("  gui")
    print("  list")
    print("  show <theme_name>")
    print("  update <file|all> section add <section_name>")
    print("  update <file|all> section remove <section_name>")
    print("  update <file|all> key add <section> <key> <value>")
    print("  update <file|all> key remove <section> <key>")

def cmd_list():
    theme_dir = get_theme_dir()
    if theme_dir.exists():
        print("Available themes:")
        for theme_file in sorted(theme_dir.glob("*.ini")):
            print(f"  {theme_file.stem}")
    else:
        print(f"Theme directory not found: {theme_dir}")

def cmd_show(theme_name):
    if not theme_name.endswith('.ini'):
        theme_name += '.ini'
    theme_path = get_theme_dir() / theme_name
    if theme_path.exists():
        print(f"--- Content of {theme_name} ---")
        with open(theme_path, 'r') as f:
            print(f.read())
    else:
        print(f"Error: Theme '{theme_name}' not found at {theme_path}")

def cmd_update(args):
    if len(args) < 1:
        print("Error: update requires target (file path or 'all')")
        sys.exit(1)
    
    target = args[0]
    
    if target == "all":
        theme_dir = get_theme_dir()
        theme_files = list(theme_dir.glob("*.ini"))
    else:
        theme_files = [Path(target)]
    
    if len(args) < 2:
        print("Error: update requires object type (section/key)")
        sys.exit(1)
    
    obj_type = args[1]
    
    if obj_type == "section":
        if len(args) < 4:
            print("Error: section requires action (add/remove) and name")
            sys.exit(1)
        
        action = args[2]
        section_name = args[3]
        
        for theme_file in theme_files:
            editor = ThemeEditor(str(theme_file))
            if action == "add":
                editor.add_section(section_name)
            elif action == "remove":
                editor.remove_section(section_name)
            editor.save()
            print(f"Updated: {theme_file}")
    
    elif obj_type == "key":
        if len(args) < 5:
            print("Error: key requires action, section, key name")
            sys.exit(1)
        
        action = args[2]
        section = args[3]
        key = args[4]
        
        for theme_file in theme_files:
            editor = ThemeEditor(str(theme_file))
            if action == "add":
                if len(args) < 6:
                    print("Error: key add requires value")
                    sys.exit(1)
                value = args[5]
                editor.add_key(section, key, value)
            elif action == "remove":
                editor.remove_key(section, key)
            editor.save()
            print(f"Updated: {theme_file}")

def run_cli(args):
    if len(args) < 1:
        print_usage()
        sys.exit(1)
    
    cmd = args[0]
    
    if cmd == "list":
        cmd_list()
    elif cmd == "show":
        if len(args) < 2:
            print("Error: show requires theme name")
            sys.exit(1)
        cmd_show(args[1])
    elif cmd == "update":
        cmd_update(args[1:])
    elif cmd == "gui":
        from .gui import run_gui
        run_gui()
    else:
        print(f"Unknown command: {cmd}")
        print_usage()
