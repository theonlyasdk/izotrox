#!/usr/bin/env python3
import sys
import os
import glob
from pathlib import Path

class ThemeEditor:
    def __init__(self, theme_path):
        self.theme_path = theme_path
        self.sections = {}
        self.load()
    
    def load(self):
        if not os.path.exists(self.theme_path):
            return
        
        current_section = None
        with open(self.theme_path, 'r') as f:
            for line in f:
                line = line.rstrip('\n')
                stripped = line.strip()
                
                if stripped.startswith('[') and stripped.endswith(']'):
                    current_section = stripped[1:-1]
                    if current_section not in self.sections:
                        self.sections[current_section] = []
                elif stripped and not stripped.startswith('#') and not stripped.startswith(';'):
                    if current_section:
                        self.sections[current_section].append(line)
                elif current_section:
                    self.sections[current_section].append(line)
    
    def add_section(self, section_name):
        if section_name not in self.sections:
            self.sections[section_name] = []
    
    def remove_section(self, section_name):
        if section_name in self.sections:
            del self.sections[section_name]
    
    def add_key(self, section_name, key, value):
        if section_name not in self.sections:
            self.sections[section_name] = []
        
        for i, line in enumerate(self.sections[section_name]):
            if '=' in line:
                current_key = line.split('=')[0].strip()
                if current_key == key:
                    self.sections[section_name][i] = f"{key} = {value}"
                    return
        
        self.sections[section_name].append(f"{key} = {value}")
    
    def remove_key(self, section_name, key):
        if section_name not in self.sections:
            return
        
        self.sections[section_name] = [
            line for line in self.sections[section_name]
            if '=' not in line or line.split('=')[0].strip() != key
        ]
    
    def save(self):
        with open(self.theme_path, 'w') as f:
            for i, (section, lines) in enumerate(self.sections.items()):
                f.write(f"[{section}]\n")
                for line in lines:
                    f.write(f"{line}\n")
                if i < len(self.sections) - 1:
                    f.write("\n")

def main():
    if len(sys.argv) < 2:
        print("Usage: theme_editor.py <command> [args...]")
        print("\nCommands:")
        print("  update <file|all> section add <section_name>")
        print("  update <file|all> section remove <section_name>")
        print("  update <file|all> key add <section> <key> <value>")
        print("  update <file|all> key remove <section> <key>")
        sys.exit(1)
    
    cmd = sys.argv[1]
    
    if cmd == "update":
        if len(sys.argv) < 3:
            print("Error: update requires target (file path or 'all')")
            sys.exit(1)
        
        target = sys.argv[2]
        
        if target == "all":
            theme_dir = Path(__file__).parent.parent / "res" / "theme"
            theme_files = list(theme_dir.glob("*.ini"))
        else:
            theme_files = [Path(target)]
        
        if len(sys.argv) < 4:
            print("Error: update requires object type (section/key)")
            sys.exit(1)
        
        obj_type = sys.argv[3]
        
        if obj_type == "section":
            if len(sys.argv) < 6:
                print("Error: section requires action (add/remove) and name")
                sys.exit(1)
            
            action = sys.argv[4]
            section_name = sys.argv[5]
            
            for theme_file in theme_files:
                editor = ThemeEditor(str(theme_file))
                if action == "add":
                    editor.add_section(section_name)
                elif action == "remove":
                    editor.remove_section(section_name)
                editor.save()
                print(f"Updated: {theme_file}")
        
        elif obj_type == "key":
            if len(sys.argv) < 7:
                print("Error: key requires action, section, key name, and value")
                sys.exit(1)
            
            action = sys.argv[4]
            section = sys.argv[5]
            key = sys.argv[6]
            
            for theme_file in theme_files:
                editor = ThemeEditor(str(theme_file))
                if action == "add":
                    if len(sys.argv) < 8:
                        print("Error: key add requires value")
                        sys.exit(1)
                    value = sys.argv[7]
                    editor.add_key(section, key, value)
                elif action == "remove":
                    editor.remove_key(section, key)
                editor.save()
                print(f"Updated: {theme_file}")

if __name__ == "__main__":
    main()
