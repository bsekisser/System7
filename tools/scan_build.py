#!/usr/bin/env python3
"""Build scanner for System 7.1 compilation issues"""

import subprocess
import json
import re
import sys
from collections import defaultdict
from pathlib import Path

def scan_build(project_dir):
    """Scan build errors and categorize them"""

    # Clean and build
    subprocess.run(['make', 'clean'], cwd=project_dir, capture_output=True)
    result = subprocess.run(['make'], cwd=project_dir, capture_output=True, text=True)

    errors = defaultdict(lambda: defaultdict(list))

    # Parse errors
    for line in result.stderr.split('\n'):
        # Unknown type errors
        if m := re.match(r'(.+?):(\d+):(\d+): error: unknown type name [\'"]?(\w+)[\'"]?', line):
            file, line_no, col, type_name = m.groups()
            errors['unknown_types'][type_name].append({
                'file': file,
                'line': int(line_no),
                'col': int(col)
            })

        # Undeclared identifier errors
        elif m := re.match(r'(.+?):(\d+):(\d+): error: [\'"]?(\w+)[\'"]? undeclared', line):
            file, line_no, col, identifier = m.groups()
            errors['undeclared'][identifier].append({
                'file': file,
                'line': int(line_no),
                'col': int(col)
            })

        # Redefinition errors
        elif m := re.match(r'(.+?):(\d+):(\d+): error: redefinition of [\'"]?(\w+)[\'"]?', line):
            file, line_no, col, symbol = m.groups()
            errors['redefinitions'][symbol].append({
                'file': file,
                'line': int(line_no),
                'col': int(col)
            })

        # Missing member errors
        elif m := re.match(r'(.+?):(\d+):(\d+): error: .* has no member named [\'"]?(\w+)[\'"]?', line):
            file, line_no, col, member = m.groups()
            errors['missing_members'][member].append({
                'file': file,
                'line': int(line_no),
                'col': int(col)
            })

        # Implicit declaration errors
        elif m := re.match(r'(.+?):(\d+):(\d+): error: implicit declaration of function [\'"]?(\w+)[\'"]?', line):
            file, line_no, col, func = m.groups()
            errors['implicit_functions'][func].append({
                'file': file,
                'line': int(line_no),
                'col': int(col)
            })

    # Generate report
    report = {
        'total_errors': sum(len(locs) for category in errors.values() for locs in category.values()),
        'categories': {}
    }

    for category, items in errors.items():
        report['categories'][category] = {
            'count': len(items),
            'total_occurrences': sum(len(locs) for locs in items.values()),
            'items': dict(items)
        }

    # Save report
    with open(project_dir / 'build_report.json', 'w') as f:
        json.dump(report, f, indent=2)

    # Print summary
    print(f"Total errors: {report['total_errors']}")
    for category, data in report['categories'].items():
        print(f"\n{category}: {data['count']} unique, {data['total_occurrences']} total")
        for item, locs in sorted(data['items'].items())[:10]:
            print(f"  - {item}: {len(locs)} occurrences")

    return report

if __name__ == '__main__':
    project_dir = Path('/home/k/iteration2')
    scan_build(project_dir)