#!/usr/bin/env python3
"""Analyze malloc/free/calloc/realloc usage across the codebase."""

import os
import re
from pathlib import Path
from collections import defaultdict

# Exclude patterns
EXCLUDE_PATTERNS = ['test', 'third-party', 'third_party', 'vendor', 'external']
EXCLUDE_EXTENSIONS = ['.md', '.txt', '.json', '.xml']

def should_exclude(filepath):
    """Check if file should be excluded."""
    filepath_str = str(filepath).lower()

    # Check for excluded patterns
    for pattern in EXCLUDE_PATTERNS:
        if pattern in filepath_str:
            return True

    # Check extension
    if filepath.suffix in EXCLUDE_EXTENSIONS:
        return True

    return False

def count_pattern(content, pattern):
    """Count occurrences of a pattern in content."""
    regex = re.compile(r'\b' + pattern + r'\s*\(')
    return len(regex.findall(content))

def categorize_file(filepath):
    """Categorize file by subsystem based on path."""
    path_parts = filepath.parts

    # Look for subsystem in path
    for part in path_parts:
        if 'SpeechManager' in part:
            return 'SpeechManager'
        elif 'SoundManager' in part:
            return 'SoundManager'
        elif 'AppleEventManager' in part:
            return 'AppleEventManager'
        elif 'FileMgr' in part or 'FileManager' in part or part == 'FS':
            return 'FileMgr'
        elif 'Finder' in part:
            return 'Finder'
        elif 'DialogManager' in part:
            return 'DialogManager'
        elif 'WindowManager' in part:
            return 'WindowManager'
        elif 'MenuManager' in part or 'Menu' in part:
            return 'MenuManager'
        elif 'EventManager' in part:
            return 'EventManager'
        elif 'MemoryMgr' in part or 'MemoryManager' in part:
            return 'MemoryMgr'
        elif 'ResourceMgr' in part or 'ResourceManager' in part:
            return 'ResourceMgr'
        elif 'ExtensionManager' in part:
            return 'ExtensionManager'
        elif 'DeskManager' in part:
            return 'DeskManager'
        elif 'ControlManager' in part:
            return 'ControlManager'
        elif 'TextEdit' in part:
            return 'TextEdit'
        elif 'FontManager' in part:
            return 'FontManager'
        elif 'ScrapManager' in part:
            return 'ScrapManager'
        elif 'StandardFile' in part:
            return 'StandardFile'
        elif 'DeviceManager' in part:
            return 'DeviceManager'
        elif 'QuickDraw' in part:
            return 'QuickDraw'
        elif 'Chooser' in part:
            return 'Chooser'
        elif 'Apps' in part:
            return 'Apps'

    # Check filename patterns for HFS files
    filename = filepath.name
    if filename.startswith('HFS_') or filename.startswith('hfs_'):
        return 'FileMgr'

    # Default to Core System
    return 'CoreSystem'

def main():
    root_dir = Path('/home/k/iteration2')

    # Data structure: subsystem -> file -> {malloc, free, calloc, realloc, total}
    results = defaultdict(lambda: {})

    # Find all C and header files
    for ext in ['*.c', '*.h']:
        for filepath in root_dir.rglob(ext):
            if should_exclude(filepath):
                continue

            try:
                with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
                    content = f.read()

                malloc_count = count_pattern(content, 'malloc')
                free_count = count_pattern(content, 'free')
                calloc_count = count_pattern(content, 'calloc')
                realloc_count = count_pattern(content, 'realloc')
                total = malloc_count + free_count + calloc_count + realloc_count

                if total > 0:
                    subsystem = categorize_file(filepath)
                    results[subsystem][str(filepath)] = {
                        'malloc': malloc_count,
                        'free': free_count,
                        'calloc': calloc_count,
                        'realloc': realloc_count,
                        'total': total
                    }
            except Exception as e:
                print(f"Error reading {filepath}: {e}")

    # Print results organized by subsystem
    print("=" * 100)
    print("MALLOC/FREE/CALLOC/REALLOC USAGE ANALYSIS")
    print("=" * 100)
    print()

    # Calculate totals per subsystem
    subsystem_totals = {}
    for subsystem, files in results.items():
        subsystem_total = sum(f['total'] for f in files.values())
        subsystem_totals[subsystem] = {
            'total': subsystem_total,
            'malloc': sum(f['malloc'] for f in files.values()),
            'free': sum(f['free'] for f in files.values()),
            'calloc': sum(f['calloc'] for f in files.values()),
            'realloc': sum(f['realloc'] for f in files.values()),
            'files': len(files)
        }

    # Sort subsystems by total count (descending)
    sorted_subsystems = sorted(subsystem_totals.items(), key=lambda x: x[1]['total'], reverse=True)

    grand_total_malloc = 0
    grand_total_free = 0
    grand_total_calloc = 0
    grand_total_realloc = 0
    grand_total_all = 0

    for subsystem, totals in sorted_subsystems:
        print(f"\n{'=' * 100}")
        print(f"SUBSYSTEM: {subsystem}")
        print(f"{'=' * 100}")
        print(f"Files: {totals['files']} | malloc: {totals['malloc']} | free: {totals['free']} | "
              f"calloc: {totals['calloc']} | realloc: {totals['realloc']} | TOTAL: {totals['total']}")
        print(f"{'-' * 100}")

        # Sort files by total count
        sorted_files = sorted(results[subsystem].items(), key=lambda x: x[1]['total'], reverse=True)

        for filepath, counts in sorted_files:
            print(f"  {filepath}")
            print(f"    malloc: {counts['malloc']:3d} | free: {counts['free']:3d} | "
                  f"calloc: {counts['calloc']:3d} | realloc: {counts['realloc']:3d} | TOTAL: {counts['total']:3d}")

        grand_total_malloc += totals['malloc']
        grand_total_free += totals['free']
        grand_total_calloc += totals['calloc']
        grand_total_realloc += totals['realloc']
        grand_total_all += totals['total']

    # Print grand totals
    print(f"\n{'=' * 100}")
    print(f"GRAND TOTALS")
    print(f"{'=' * 100}")
    print(f"Total malloc calls:  {grand_total_malloc}")
    print(f"Total free calls:    {grand_total_free}")
    print(f"Total calloc calls:  {grand_total_calloc}")
    print(f"Total realloc calls: {grand_total_realloc}")
    print(f"TOTAL ALL CALLS:     {grand_total_all}")
    print(f"{'=' * 100}")

    # Priority recommendation
    print(f"\n{'=' * 100}")
    print(f"PRIORITY RECOMMENDATIONS (by subsystem total)")
    print(f"{'=' * 100}")
    for i, (subsystem, totals) in enumerate(sorted_subsystems, 1):
        print(f"{i:2d}. {subsystem:25s} - {totals['total']:3d} calls across {totals['files']:2d} files")

if __name__ == '__main__':
    main()
