#!/usr/bin/env python3
"""Update patterns.json to use PPAT8 format for ppat 400 and 401"""

import json

# Read the original patterns
with open('patterns.json', 'r') as f:
    data = json.load(f)

# The converted PPAT8 patterns
ppat8_patterns = {
    400: {
        "type": "ppat",
        "id": 400,
        "name": "Authentic4Color",
        "data": {
            "ppat8": {
                "palette": [
                    [0, 153, 153, 255],    # Teal
                    [153, 0, 153, 255],    # Purple
                    [0, 235, 255, 255],    # Light blue
                    [255, 153, 0, 255]     # Orange
                ],
                "indices": [
                    "0 0 0 0 0 0 0 0",
                    "3 1 3 1 3 1 3 1",
                    "2 0 2 0 2 0 2 0",
                    "3 1 3 1 3 1 3 1",
                    "2 0 2 0 2 0 2 0",
                    "3 1 3 1 3 1 3 1",
                    "2 0 2 0 2 0 2 0",
                    "3 1 3 1 3 1 3 1"
                ]
            }
        }
    },
    401: {
        "type": "ppat",
        "id": 401,
        "name": "ColorGradient",  # Change name to be more descriptive
        "data": {
            "ppat8": {
                "palette": [
                    [255, 255, 255, 255],  # White
                    [200, 200, 200, 255],  # Light gray
                    [150, 150, 150, 255],  # Medium gray
                    [100, 100, 100, 255],  # Dark gray
                    [50, 50, 50, 255],     # Very dark gray
                    [0, 0, 0, 255],        # Black
                    [255, 0, 0, 255],      # Red
                    [0, 0, 255, 255]       # Blue
                ],
                "indices": [
                    "0 0 1 1 2 2 3 3",
                    "0 0 1 1 2 2 3 3",
                    "1 1 2 2 3 3 4 4",
                    "1 1 2 2 3 3 4 4",
                    "2 2 3 3 4 4 5 5",
                    "2 2 3 3 4 4 5 5",
                    "3 3 4 4 5 5 0 0",
                    "3 3 4 4 5 5 0 0"
                ]
            }
        }
    }
}

# Update the patterns in the original data
for i, resource in enumerate(data['resources']):
    if resource['type'] == 'ppat' and resource['id'] in ppat8_patterns:
        data['resources'][i] = ppat8_patterns[resource['id']]

# Write back the updated patterns
with open('patterns.json', 'w') as f:
    json.dump(data, f, indent=2)

print("Updated patterns.json with PPAT8 format for ppat 400 and 401")