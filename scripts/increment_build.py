import re
from datetime import datetime

def increment_build(header_file):
    """Increment build number and update timestamp in EARS_versionDef.h"""
    
    # Read the current file
    try:
        with open(header_file, 'r', encoding='utf-8') as f:
            content = f.read()
    except FileNotFoundError:
        print(f"ERROR: Could not find {header_file}")
        return
    
    # Find and increment EARS_APP_BUILD_NUMBER
    build_pattern = r'#define\s+EARS_APP_BUILD_NUMBER\s+(\d+)'
    match = re.search(build_pattern, content)
    
    if match:
        current_build = int(match.group(1))
        new_build = current_build + 1
        content = re.sub(build_pattern, f'#define EARS_APP_BUILD_NUMBER {new_build}', content)
        print(f"Build number incremented: {current_build} -> {new_build}")
    else:
        # Add BUILD_NUMBER after BUILD_TIMESTAMP line
        print("EARS_APP_BUILD_NUMBER not found, adding it")
        timestamp_line = r'(#define\s+EARS_APP_BUILD_TIMESTAMP\s+\d+)'
        if re.search(timestamp_line, content):
            content = re.sub(
                timestamp_line,
                r'\1\n#define EARS_APP_BUILD_NUMBER 1',
                content
            )
        else:
            # If timestamp also doesn't exist, add both after PATCH
            patch_line = r'(#define\s+EARS_APP_VERSION_PATCH\s+"[^"]*")'
            content = re.sub(
                patch_line,
                r'\1\n\n#define EARS_APP_BUILD_TIMESTAMP 0\n#define EARS_APP_BUILD_NUMBER 1',
                content
            )
        new_build = 1
    
    # Update EARS_APP_BUILD_TIMESTAMP
    timestamp = datetime.now().strftime('%Y%m%d%H%M%S')
    timestamp_pattern = r'(#define\s+EARS_APP_BUILD_TIMESTAMP\s+)\d+'
    
    if re.search(timestamp_pattern, content):
        content = re.sub(timestamp_pattern, r'\g<1>' + timestamp, content)
        print(f"Timestamp updated: {timestamp}")
    else:
        print("WARNING: EARS_APP_BUILD_TIMESTAMP not found")
    
    # Write back to file
    try:
        with open(header_file, 'w', encoding='utf-8') as f:
            f.write(content)
        print(f"Successfully updated {header_file}")
    except Exception as e:
        print(f"ERROR: Could not write to {header_file}: {e}")

if __name__ == '__main__':
    increment_build('include/EARS_versionDef.h')