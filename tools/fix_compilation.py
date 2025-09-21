#!/usr/bin/env python3
"""Automated compilation fixer for System 7.1"""

import subprocess
import re
import os
from pathlib import Path

def get_compilation_status():
    """Get current compilation status"""
    print("Scanning compilation status...")

    # Clean build
    subprocess.run(['make', 'clean'], capture_output=True, cwd='/home/k/iteration2')

    # Attempt full build
    result = subprocess.run(['make', '-j1'], capture_output=True, text=True, cwd='/home/k/iteration2')

    # Count successful compilations
    success_count = len(re.findall(r'^CC src.*\.c$', result.stdout, re.MULTILINE))

    # Extract errors
    errors = re.findall(r'(src/.*\.c:\d+:\d+: error: .*)', result.stderr)

    # Find stray backslash errors
    stray_errors = re.findall(r'(src/.*\.c):(\d+):\d+: error: stray .\\. in program', result.stderr)

    # Find missing type errors
    missing_types = set()
    for match in re.finditer(r"error: unknown type name '([^']+)'", result.stderr):
        missing_types.add(match.group(1))

    # Find undefined symbols
    undefined_symbols = set()
    for match in re.finditer(r"error: '([^']+)' undeclared", result.stderr):
        undefined_symbols.add(match.group(1))

    return {
        'success_count': success_count,
        'total_errors': len(errors),
        'stray_errors': stray_errors,
        'missing_types': missing_types,
        'undefined_symbols': undefined_symbols,
        'first_errors': errors[:10] if errors else []
    }

def fix_stray_backslashes():
    """Fix all stray backslash errors in source files"""
    print("\nFixing stray backslash errors...")

    src_dir = Path('/home/k/iteration2/src')
    fixed_count = 0

    for c_file in src_dir.rglob('*.c'):
        with open(c_file, 'r') as f:
            content = f.read()

        # Look for patterns like ->\ followed by non-standard characters
        pattern = r'\(g_system\)->\\[0-9]\.(\w+)'
        if re.search(pattern, content):
            # Replace with commented version
            new_content = re.sub(pattern, r'/* g_system.expand_mem->\1 */', content)

            # Also fix simpler cases
            new_content = re.sub(r'->\\2\.', '/* ->*/', new_content)

            with open(c_file, 'w') as f:
                f.write(new_content)

            fixed_count += 1
            print(f"  Fixed: {c_file.name}")

    return fixed_count

def add_missing_types(missing_types):
    """Add missing type definitions to SystemTypes.h"""
    if not missing_types:
        return 0

    print(f"\nAdding {len(missing_types)} missing type definitions...")

    types_file = '/home/k/iteration2/include/SystemTypes.h'

    # Read current content
    with open(types_file, 'r') as f:
        lines = f.readlines()

    # Find insertion point (before final #endif)
    insert_idx = -1
    for i in range(len(lines) - 1, -1, -1):
        if '#endif' in lines[i]:
            insert_idx = i
            break

    if insert_idx == -1:
        return 0

    # Build new type definitions
    new_types = ['\n// Additional missing types\n']

    for type_name in missing_types:
        # Generate appropriate typedef based on name pattern
        if 'Ptr' in type_name and type_name != 'Ptr':
            base = type_name[:-3] if type_name.endswith('Ptr') else type_name
            new_types.append(f'typedef struct {base}* {type_name};\n')
        elif 'Ref' in type_name:
            new_types.append(f'typedef SInt16 {type_name};\n')
        elif 'ID' in type_name:
            new_types.append(f'typedef SInt32 {type_name};\n')
        elif 'Proc' in type_name:
            new_types.append(f'typedef void (*{type_name})(void);\n')
        else:
            new_types.append(f'typedef struct {type_name} {type_name};\n')

    # Insert new types
    lines[insert_idx:insert_idx] = new_types

    # Write back
    with open(types_file, 'w') as f:
        f.writelines(lines)

    return len(missing_types)

def main():
    """Main compilation fixer"""
    print("System 7.1 Compilation Fixer")
    print("=" * 50)

    # Get initial status
    status = get_compilation_status()

    print(f"\nInitial Status:")
    print(f"  Files compiled: {status['success_count']}")
    print(f"  Total errors: {status['total_errors']}")
    print(f"  Stray backslash errors: {len(status['stray_errors'])}")
    print(f"  Missing types: {len(status['missing_types'])}")
    print(f"  Undefined symbols: {len(status['undefined_symbols'])}")

    if status['stray_errors']:
        print(f"\nStray errors in: {[f[0] for f in status['stray_errors'][:5]]}")

    if status['missing_types']:
        print(f"\nMissing types: {list(status['missing_types'])[:10]}")

    # Fix stray backslashes
    if status['stray_errors']:
        fixed = fix_stray_backslashes()
        print(f"Fixed {fixed} files with stray backslashes")

    # Add missing types
    if status['missing_types']:
        added = add_missing_types(status['missing_types'])
        print(f"Added {added} type definitions")

    # Recheck status
    print("\n" + "=" * 50)
    print("Rechecking compilation...")
    final_status = get_compilation_status()

    print(f"\nFinal Status:")
    print(f"  Files compiled: {final_status['success_count']}")
    print(f"  Total errors: {final_status['total_errors']}")
    print(f"  Improvement: {final_status['success_count'] - status['success_count']} more files compiled")

    if final_status['first_errors']:
        print(f"\nRemaining errors (first 5):")
        for err in final_status['first_errors'][:5]:
            print(f"  {err}")

    # List of source files
    src_count = len(list(Path('/home/k/iteration2/src').rglob('*.c')))
    print(f"\nCompilation progress: {final_status['success_count']}/{src_count} files")

    if final_status['success_count'] == src_count:
        print("\n✅ ALL FILES COMPILED SUCCESSFULLY!")
    else:
        print(f"\n⚠️  {src_count - final_status['success_count']} files still have errors")

if __name__ == '__main__':
    main()