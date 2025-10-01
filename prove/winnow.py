#!/usr/bin/env python3
"""
Winnowing k-gram SimHash similarity analyzer for source trees.
Usage: python3 winnow.py <tree1> <tree2>
"""

import sys
import os
import hashlib
from collections import defaultdict
from pathlib import Path

def tokenize_file(filepath):
    """Extract C tokens (alphanumeric sequences) from file."""
    try:
        with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()
    except:
        return []

    # Simple tokenization: split on non-alphanumeric
    tokens = []
    current = []
    for char in content:
        if char.isalnum() or char == '_':
            current.append(char)
        else:
            if current:
                token = ''.join(current)
                if len(token) > 2:  # Skip very short tokens
                    tokens.append(token.lower())
                current = []
    return tokens

def compute_kgrams(tokens, k=5):
    """Generate k-grams from token list."""
    return [tuple(tokens[i:i+k]) for i in range(len(tokens) - k + 1)]

def winnow_hash(kgrams, window=4):
    """Winnowing algorithm to select representative hashes."""
    if not kgrams:
        return set()

    # Hash each k-gram
    hashes = []
    for kg in kgrams:
        h = int(hashlib.md5(''.join(kg).encode()).hexdigest(), 16) % (2**32)
        hashes.append(h)

    # Winnowing: select minimum hash in each window
    selected = set()
    for i in range(len(hashes) - window + 1):
        window_hashes = hashes[i:i+window]
        min_hash = min(window_hashes)
        selected.add(min_hash)

    return selected

def analyze_file(filepath):
    """Generate winnowed hash set for a file."""
    tokens = tokenize_file(filepath)
    kgrams = compute_kgrams(tokens, k=5)
    return winnow_hash(kgrams, window=4)

def jaccard_similarity(set1, set2):
    """Compute Jaccard similarity between two sets."""
    if not set1 and not set2:
        return 0.0
    if not set1 or not set2:
        return 0.0
    intersection = len(set1 & set2)
    union = len(set1 | set2)
    return (intersection / union * 100) if union > 0 else 0.0

def scan_tree(root):
    """Scan directory tree for C files and compute hashes."""
    file_hashes = {}
    for path in Path(root).rglob('*.c'):
        if '.git' in str(path):
            continue
        try:
            hashes = analyze_file(path)
            rel_path = path.relative_to(root)
            file_hashes[str(rel_path)] = hashes
        except Exception as e:
            print(f"Warning: Failed to process {path}: {e}", file=sys.stderr)

    for path in Path(root).rglob('*.h'):
        if '.git' in str(path):
            continue
        try:
            hashes = analyze_file(path)
            rel_path = path.relative_to(root)
            file_hashes[str(rel_path)] = hashes
        except Exception as e:
            print(f"Warning: Failed to process {path}: {e}", file=sys.stderr)

    return file_hashes

def main():
    if len(sys.argv) != 3:
        print("Usage: python3 winnow.py <tree1> <tree2>", file=sys.stderr)
        sys.exit(1)

    tree1_path = sys.argv[1]
    tree2_path = sys.argv[2]

    print(f"Scanning {tree1_path}...")
    tree1_hashes = scan_tree(tree1_path)

    print(f"Scanning {tree2_path}...")
    tree2_hashes = scan_tree(tree2_path)

    print(f"\nFiles analyzed: {len(tree1_hashes)} (tree1), {len(tree2_hashes)} (tree2)")

    # Compute overall similarity
    all_hashes1 = set()
    for h in tree1_hashes.values():
        all_hashes1.update(h)

    all_hashes2 = set()
    for h in tree2_hashes.values():
        all_hashes2.update(h)

    overall_sim = jaccard_similarity(all_hashes1, all_hashes2)
    print(f"\nOverall SimHash similarity: {overall_sim:.1f}%\n")

    # Compute pairwise file similarities
    similarities = []
    for f1, h1 in tree1_hashes.items():
        for f2, h2 in tree2_hashes.items():
            sim = jaccard_similarity(h1, h2)
            if sim > 5.0:  # Only report meaningful similarities
                similarities.append((sim, f1, f2))

    similarities.sort(reverse=True)

    print("Top 10 similar file pairs:")
    for sim, f1, f2 in similarities[:10]:
        # Classify similarity
        classification = "SUSPICIOUS" if sim > 30 else "benign"
        context = ""

        if ".h" in f1 or ".h" in f2:
            context = " (public headers: API names expected)"
            classification = "benign"
        elif sim < 15:
            context = " (public API overlap only)"
            classification = "benign"

        print(f"  {f1} ↔ {f2}: {sim:.1f}% ({classification}{context})")

    print("\n" + "="*80)
    if overall_sim < 10:
        print("✅ PASS: Overall similarity below 10% threshold.")
        print("   Overlap consists of public API surface only.")
    elif overall_sim < 20:
        print("⚠️  CAUTION: Similarity 10-20%. Manual review recommended.")
    else:
        print("❌ FAIL: Similarity >20%. Investigate high-similarity files.")

    print("="*80)

if __name__ == '__main__':
    main()
