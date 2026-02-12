#!/usr/bin/env python3
import os
import json
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
        if not key or not section_name:
            print("Error: Empty key or section name")
            return
        if '=' in key:
            print(f"Error: Key '{key}' cannot contain '='")
            return

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
    
    def rename_section(self, old_name, new_name):
        if old_name not in self.sections:
            return False
        if new_name in self.sections:
            return False
        self.sections[new_name] = self.sections.pop(old_name)
        return True
    
    def rename_key(self, section_name, old_key, new_key):
        if section_name not in self.sections:
            return False
        if not old_key or not new_key or '=' in new_key:
            return False
        
        for i, line in enumerate(self.sections[section_name]):
            if '=' in line:
                current_key = line.split('=')[0].strip()
                if current_key == old_key:
                    value = line.split('=', 1)[1].strip()
                    self.sections[section_name][i] = f"{new_key} = {value}"
                    return True
        return False
    
    def move_key(self, source_section, key, target_section):
        if source_section not in self.sections or target_section not in self.sections:
            return False
        
        key_value = None
        for line in self.sections[source_section]:
            if '=' in line:
                current_key = line.split('=')[0].strip()
                if current_key == key:
                    key_value = line.split('=', 1)[1].strip()
                    break
        
        if key_value is None:
            return False
        
        self.remove_key(source_section, key)
        self.add_key(target_section, key, key_value)
        return True
    
    def cleanup(self):
        for section in self.sections:
            self.sections[section] = sorted([line for line in self.sections[section] if line.strip()])

    def save(self):
        temp_path = f"{self.theme_path}.tmp"
        try:
            self.cleanup()
            
            with open(temp_path, 'w') as f:
                sorted_sections = sorted(self.sections.keys())
                for i, section in enumerate(sorted_sections):
                    f.write(f"[{section}]\n")
                    lines = self.sections[section]
                    for line in lines:
                        f.write(f"{line}\n")
                    if i < len(sorted_sections) - 1:
                        f.write("\n")
            
            if os.path.getsize(temp_path) == 0 and self.sections:
                 print(f"Error: Generated empty file for {self.theme_path}, aborting save.")
                 if os.path.exists(temp_path):
                     os.remove(temp_path)
                 return

            os.replace(temp_path, self.theme_path)
        except Exception as e:
            print(f"Failed to save {self.theme_path}: {e}")
            if os.path.exists(temp_path):
                os.remove(temp_path)


def get_theme_dir():
    return Path(__file__).parent.parent.parent / "res" / "themes"

def load_enums():
    enum_file = Path(__file__).parent / "data" / "enums.json"
    if enum_file.exists():
        with open(enum_file, 'r') as f:
            return json.load(f)
    return {}
