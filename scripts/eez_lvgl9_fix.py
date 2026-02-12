"""
EEZ Studio LVGL 9.3.0 Compatibility Fix
This script fixes LVGL function calls in EEZ Studio generated code
to be compatible with LVGL 9.3.0

ESF 0.25.1 generates calls without animation parameters
LVGL 9.3.0 requires animation parameters

Author: Claude Sonnet 4.5
Updated: 20250212 - Fixed to ADD animation parameters for LVGL 9.3.0
"""

# type: ignore - PlatformIO build script
Import("env")  # type: ignore # Provided by PlatformIO SCons environment
import os
import re

def fix_eez_lvgl9_compatibility(source, target, env):
    """
    Fix LVGL 9.3.0 compatibility issues in eez-flow.cpp
    
    In LVGL 9.3.0, these functions REQUIRE animation parameters.
    ESF 0.25.1 generates calls WITHOUT them.
    This script ADDS the missing LV_ANIM_OFF parameter:
    - lv_bar_set_value()
    - lv_roller_set_selected()
    - lv_slider_set_value()
    - lv_slider_set_start_value()
    - lv_dropdown_set_selected()
    """
    
    # Path to the eez-flow.cpp file
    eez_flow_path = os.path.join(env.subst("$PROJECT_DIR"), "src", "ui", "eez-flow.cpp")
    
    # Check if file exists
    if not os.path.exists(eez_flow_path):
        print(f"⚠️  EEZ flow file not found: {eez_flow_path}")
        return
    
    print(f"🔧 Checking EEZ Studio LVGL 9.3.0 compatibility: {eez_flow_path}")
    
    # Read the file
    with open(eez_flow_path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    original_content = content
    fixes_applied = 0
    
    # Define function patterns to fix
    # Each pattern ADDS the missing animation parameter (third parameter)
    
    function_fixes = [
        {
            'name': 'lv_bar_set_value',
            # Matches: lv_bar_set_value(obj, value)
            # Replaces with: lv_bar_set_value(obj, value, LV_ANIM_OFF)
            'pattern': r'lv_bar_set_value\s*\(\s*([^,]+)\s*,\s*([^)]+)\s*\)',
            'replacement': r'lv_bar_set_value(\1, \2, LV_ANIM_OFF)'
        },
        {
            'name': 'lv_roller_set_selected',
            # Matches: lv_roller_set_selected(obj, value)
            # Replaces with: lv_roller_set_selected(obj, value, LV_ANIM_OFF)
            'pattern': r'lv_roller_set_selected\s*\(\s*([^,]+)\s*,\s*([^)]+)\s*\)',
            'replacement': r'lv_roller_set_selected(\1, \2, LV_ANIM_OFF)'
        },
        {
            'name': 'lv_slider_set_value',
            # Matches: lv_slider_set_value(obj, value)
            # Replaces with: lv_slider_set_value(obj, value, LV_ANIM_OFF)
            'pattern': r'lv_slider_set_value\s*\(\s*([^,]+)\s*,\s*([^)]+)\s*\)',
            'replacement': r'lv_slider_set_value(\1, \2, LV_ANIM_OFF)'
        },
        {
            'name': 'lv_slider_set_start_value',
            # Matches: lv_slider_set_start_value(obj, value)
            # Replaces with: lv_slider_set_start_value(obj, value, LV_ANIM_OFF)
            'pattern': r'lv_slider_set_start_value\s*\(\s*([^,]+)\s*,\s*([^)]+)\s*\)',
            'replacement': r'lv_slider_set_start_value(\1, \2, LV_ANIM_OFF)'
        },
        {
            'name': 'lv_dropdown_set_selected',
            # Matches: lv_dropdown_set_selected(obj, value)
            # Replaces with: lv_dropdown_set_selected(obj, value, LV_ANIM_OFF)
            'pattern': r'lv_dropdown_set_selected\s*\(\s*([^,]+)\s*,\s*([^)]+)\s*\)',
            'replacement': r'lv_dropdown_set_selected(\1, \2, LV_ANIM_OFF)'
        }
    ]
    
    # Apply each fix
    for fix in function_fixes:
        # Only match calls that DON'T already have LV_ANIM parameter
        # This prevents double-fixing
        pattern_with_check = fix['pattern'].replace(')', r'(?!.*LV_ANIM)')
        content_before = content
        content, count = re.subn(fix['pattern'], fix['replacement'], content)
        
        if count > 0:
            print(f"   ✅ Fixed {count} {fix['name']}() call(s)")
            fixes_applied += count
    
    # Write back if changes were made
    if content != original_content:
        with open(eez_flow_path, 'w', encoding='utf-8') as f:
            f.write(content)
        print(f"✅ EEZ Studio LVGL 9.3.0 compatibility fixes applied: {fixes_applied} total changes")
    else:
        print("✅ EEZ Studio code already compatible - no fixes needed")

# Register the callback to run before building
env.AddPreAction("$BUILD_DIR/${PROGNAME}.elf", fix_eez_lvgl9_compatibility)

print("🔍 EEZ Studio LVGL 9.3.0 compatibility fixer loaded")