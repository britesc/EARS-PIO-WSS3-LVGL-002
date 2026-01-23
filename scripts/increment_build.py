Import("env")

import re
from datetime import datetime

def increment_build(header_file):
    """Increment patch version and update timestamp in EARS_versionDef.h"""
    
    print("=" * 70)
    print("  BUILD VERSION INCREMENT SCRIPT")
    print("=" * 70)
    
    # Read the current file
    try:
        with open(header_file, 'r', encoding='utf-8') as f:
            content = f.read()
        print(f"✓ File read: {header_file} ({len(content)} bytes)")
    except FileNotFoundError:
        print(f"✗ ERROR: File not found: {header_file}")
        return
    except Exception as e:
        print(f"✗ ERROR: Could not read {header_file}: {e}")
        return
    
    # Find and increment EARS_APP_VERSION_PATCH
    patch_pattern = r'#define\s+EARS_APP_VERSION_PATCH\s+"(\d+)"'
    match = re.search(patch_pattern, content)
    
    if match:
        current_patch = int(match.group(1))
        new_patch = current_patch + 1
        content = re.sub(patch_pattern, f'#define EARS_APP_VERSION_PATCH "{new_patch}"', content)
        print(f"✓ Patch version: {current_patch} -> {new_patch}")
    else:
        print("✗ ERROR: EARS_APP_VERSION_PATCH not found")
        return
    
    # Update EARS_APP_BUILD_TIMESTAMP
    timestamp = datetime.now().strftime('%Y%m%d%H%M%S')
    timestamp_pattern = r'#define\s+EARS_APP_BUILD_TIMESTAMP\s+\d+'
    
    if re.search(timestamp_pattern, content):
        content = re.sub(timestamp_pattern, f'#define EARS_APP_BUILD_TIMESTAMP {timestamp}', content, count=1)
        print(f"✓ Timestamp: {timestamp}")
    else:
        print("✗ WARNING: EARS_APP_BUILD_TIMESTAMP not found")
    
    # Write back to file
    try:
        with open(header_file, 'w', encoding='utf-8') as f:
            f.write(content)
        print(f"✓ File updated: {header_file}")
    except Exception as e:
        print(f"✗ ERROR: Could not write to {header_file}: {e}")
    
    print("=" * 70)
    print("")

# Run the increment
increment_build('include/EARS_versionDef.h')