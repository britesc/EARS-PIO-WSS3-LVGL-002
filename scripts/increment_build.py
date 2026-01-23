import re
from datetime import datetime
import os

def increment_build(header_file):
    """Increment patch version and update timestamp in EARS_versionDef.h"""
    
    # Check if file exists
    if not os.path.exists(header_file):
        print(f"ERROR: File not found: {header_file}")
        print(f"Current directory: {os.getcwd()}")
        print(f"Looking for: {os.path.abspath(header_file)}")
        return
    
    print(f"Found file: {os.path.abspath(header_file)}")
    
    # Read the current file
    try:
        with open(header_file, 'r', encoding='utf-8') as f:
            content = f.read()
        print(f"File read successfully ({len(content)} bytes)")
    except Exception as e:
        print(f"ERROR: Could not read {header_file}: {e}")
        return
    
    # Find and increment EARS_APP_VERSION_PATCH
    patch_pattern = r'#define\s+EARS_APP_VERSION_PATCH\s+"(\d+)"'
    match = re.search(patch_pattern, content)
    
    if match:
        current_patch = int(match.group(1))
        new_patch = current_patch + 1
        old_line = match.group(0)
        new_line = f'#define EARS_APP_VERSION_PATCH "{new_patch}"'
        content = re.sub(patch_pattern, new_line, content)
        print(f"Patch version incremented: {current_patch} -> {new_patch}")
        print(f"  Old: {old_line}")
        print(f"  New: {new_line}")
    else:
        print("ERROR: EARS_APP_VERSION_PATCH not found in header file")
        print("Searched for pattern:", patch_pattern)
        # Show what's actually in the file
        for i, line in enumerate(content.split('\n'), 1):
            if 'PATCH' in line:
                print(f"  Line {i}: {line}")
        return
    
    # Update EARS_APP_BUILD_TIMESTAMP
    timestamp = datetime.now().strftime('%Y%m%d%H%M%S')
    timestamp_pattern = r'#define\s+EARS_APP_BUILD_TIMESTAMP\s+\d+'
    
    match = re.search(timestamp_pattern, content)
    if match:
        old_line = match.group(0)
        new_line = f'#define EARS_APP_BUILD_TIMESTAMP {timestamp}'
        content = re.sub(timestamp_pattern, new_line, content, count=1)
        print(f"Timestamp updated: {timestamp}")
        print(f"  Old: {old_line}")
        print(f"  New: {new_line}")
    else:
        print("WARNING: EARS_APP_BUILD_TIMESTAMP not found")
        print("Searched for pattern:", timestamp_pattern)
        # Show what's actually in the file
        for i, line in enumerate(content.split('\n'), 1):
            if 'TIMESTAMP' in line:
                print(f"  Line {i}: {line}")
    
    # Write back to file
    try:
        with open(header_file, 'w', encoding='utf-8') as f:
            f.write(content)
        print(f"Successfully updated {header_file}")
        print(f"Written {len(content)} bytes")
        print("")  # Blank line for readability
    except Exception as e:
        print(f"ERROR: Could not write to {header_file}: {e}")

if __name__ == '__main__':
    increment_build('include/EARS_versionDef.h')