#!/usr/bin/env python3
import json
import re
from pathlib import Path
from collections import OrderedDict


def infer_type(value):
    value = value.strip()
    
    color_pattern = r'^\s*\d+\s*,\s*\d+\s*,\s*\d+\s*(,\s*\d+)?\s*$'
    if re.match(color_pattern, value):
        return "color"
    
    try:
        if '.' in value:
            float(value)
            return "float"
        else:
            int(value)
            return "int"
    except ValueError:
        pass
    
    if value.lower() in ['true', 'false', 'yes', 'no', '1', '0']:
        return "bool"
    
    if value.startswith('res/') or '/' in value or '\\' in value:
        return "path"
    
    return "string"


def generate_schema_from_ini(ini_path, schema_path):
    schema = OrderedDict()
    current_section = None
    
    with open(ini_path, 'r') as f:
        for line in f:
            line = line.strip()
            
            if line.startswith('[') and line.endswith(']'):
                current_section = line[1:-1]
                schema[current_section] = OrderedDict()
            elif '=' in line and current_section:
                key, value = line.split('=', 1)
                key = key.strip()
                value = value.strip()
                
                key_type = infer_type(value)
                schema[current_section][key] = key_type
    
    with open(schema_path, 'w') as f:
        json.dump(schema, f, indent=2)
    
    print(f"Schema generated successfully at {schema_path}")
    print(f"Sections found: {list(schema.keys())}")


if __name__ == "__main__":
    script_dir = Path(__file__).parent
    default_ini = script_dir.parent.parent / "res" / "theme" / "default.ini"
    schema_file = script_dir / "data" / "schema.json"
    
    generate_schema_from_ini(default_ini, schema_file)
