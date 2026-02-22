#!/usr/bin/env python3
"""
ESF Image Path Fixer for LVGL Filesystem
Adds 'S:' drive prefix to image paths in ESF-generated images.c
Runs automatically after ESF build
"""

import re
import sys

def fix_image_paths(filepath):
    """Add S: prefix to image paths in images.c"""
    
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            content = f.read()
        
        # Pattern: { "name", "/path/to/image.bin" }
        # Replace: { "name", "S:/path/to/image.bin" }
        pattern = r'(\{\s*"[^"]+",\s*)"(/[^"]+)"'
        replacement = r'\1"S:\2"'
        
        fixed_content = re.sub(pattern, replacement, content)
        
        # Count fixes made
        fixes = len(re.findall(pattern, content))
        
        if fixes > 0:
            with open(filepath, 'w', encoding='utf-8') as f:
                f.write(fixed_content)
            print(f"✅ Fixed {fixes} image path(s) in {filepath}")
            return True
        else:
            print(f"ℹ️  No image paths found in {filepath}")
            return False
            
    except FileNotFoundError:
        print(f"❌ File not found: {filepath}")
        return False
    except Exception as e:
        print(f"❌ Error: {e}")
        return False

if __name__ == "__main__":
    # Default path
    images_c_path = "src/ui/images.c"
    
    # Allow override via command line
    if len(sys.argv) > 1:
        images_c_path = sys.argv[1]
    
    print(f"ESF Image Path Fixer - Processing {images_c_path}")
    success = fix_image_paths(images_c_path)
    
    sys.exit(0 if success else 1)