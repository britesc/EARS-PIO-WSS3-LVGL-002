import re
from datetime import datetime

def increment_build(header_file):
    # Read the current file
    with open(header_file, 'r') as f:
        content = f.read()
    
    # Find and increment EARS_APP_VERSION_PATCH
    build_pattern = r'#define\s+EARS_APP_VERSION_PATCH\s+(\d+)'
    match = re.search(build_pattern, content)
    
    if match:
        current_build = int(match.group(1))
        new_build = current_build + 1
        content = re.sub(build_pattern, f'#define EARS_APP_VERSION_PATCH {new_build}', content)
        print(f"Build number incremented: {current_build} -> {new_build}")
    else:
        print("EARS_APP_VERSION_PATCH not found, adding it")
        content += f'\n#define EARS_APP_VERSION_PATCH 1\n'
    
    # Update or add EARS_APP_BUILD_TIMESTAMP
    timestamp = datetime.now().strftime('%Y%m%d%H%M%S')
    timestamp_pattern = r'#define\s+EARS_APP_BUILD_TIMESTAMP\s+\d+'
    
    if re.search(timestamp_pattern, content):
        content = re.sub(timestamp_pattern, f'#define EARS_APP_BUILD_TIMESTAMP {timestamp}', content)
    else:
        # Add after EARS_APP_VERSION_PATCH if it exists
        content = re.sub(
            r'(#define\s+EARS_APP_VERSION_PATCH\s+\d+)',
            r'\1\n#define EARS_APP_BUILD_TIMESTAMP ' + timestamp,
            content
        )
    
    # Write back to file
    with open(header_file, 'w') as f:
        f.write(content)
    
    print(f"Timestamp updated: {timestamp}")

if __name__ == '__main__':
    increment_build('include/EARS_versionDef.h')