#!/usr/bin/env python3
"""
System 7 Integration Test Runner

Automated test execution and result parsing for Phase 1 integration tests.
- Builds kernel with INTEGRATION_TESTS=1
- Runs QEMU and captures serial output
- Parses test results from log output
- Reports pass/fail status
- Can be integrated into CI/CD pipelines

Usage:
    python3 run_integration_tests.py                 # Run all tests
    python3 run_integration_tests.py --timeout 60    # Custom timeout
    python3 run_integration_tests.py --output junit  # JUnit XML output
"""

import sys
import os
import subprocess
import re
import argparse
import time
from pathlib import Path
from typing import List, Dict, Tuple
from datetime import datetime

class TestResult:
    def __init__(self, name: str, passed: bool, reason: str = ""):
        self.name = name
        self.passed = passed
        self.reason = reason
        self.timestamp = datetime.now()

    def __str__(self):
        status = "✓ PASS" if self.passed else "✗ FAIL"
        return f"{status}: {self.name}" + (f" ({self.reason})" if self.reason else "")

class TestRunner:
    def __init__(self, project_root: str, timeout: int = 120, verbose: bool = False):
        self.project_root = Path(project_root)
        self.timeout = timeout
        self.verbose = verbose
        self.results: List[TestResult] = []
        self.qemu_output = ""
        self.build_success = False

    def log(self, msg: str, level: str = "INFO"):
        """Log message with timestamp"""
        timestamp = datetime.now().strftime("%H:%M:%S")
        prefix = {
            "INFO": "[*]",
            "PASS": "[+]",
            "FAIL": "[!]",
            "WARN": "[~]",
            "DEBUG": "[D]"
        }.get(level, "[?]")
        print(f"{prefix} [{timestamp}] {msg}")

    def build_kernel(self) -> bool:
        """Build kernel and ISO with integration tests enabled"""
        self.log("Building kernel with INTEGRATION_TESTS=1...", "INFO")

        build_cmd = [
            "make", "-C", str(self.project_root),
            "INTEGRATION_TESTS=1", "clean", "iso"
        ]

        try:
            result = subprocess.run(
                build_cmd,
                capture_output=True,
                text=True,
                timeout=600,
                cwd=str(self.project_root)
            )

            if result.returncode != 0:
                self.log("Build failed!", "FAIL")
                if self.verbose:
                    self.log("Build stderr:", "DEBUG")
                    print(result.stderr)
                return False

            self.log("Build successful", "PASS")
            self.build_success = True
            return True

        except subprocess.TimeoutExpired:
            self.log("Build timeout after 600 seconds", "FAIL")
            return False
        except Exception as e:
            self.log(f"Build error: {e}", "FAIL")
            return False

    def run_tests(self) -> bool:
        """Run QEMU with serial output capture"""
        self.log("Starting QEMU for integration testing...", "INFO")

        iso_path = self.project_root / "system71.iso"
        if not iso_path.exists():
            self.log(f"ISO not found: {iso_path}", "FAIL")
            return False

        qemu_cmd = [
            "qemu-system-i386",
            "-cdrom", str(iso_path),
            "-m", "512",
            "-serial", "stdio",
            "-nographic",
            "-monitor", "none"
        ]

        try:
            result = subprocess.run(
                qemu_cmd,
                capture_output=True,
                text=True,
                timeout=self.timeout,
                cwd=str(self.project_root)
            )

            self.qemu_output = result.stdout + result.stderr
            self.log(f"QEMU execution completed (timeout: {self.timeout}s)", "INFO")

            if self.verbose:
                self.log("QEMU output (first 2000 chars):", "DEBUG")
                print(self.qemu_output[:2000])

            return True

        except subprocess.TimeoutExpired:
            self.log(f"QEMU timeout after {self.timeout} seconds", "WARN")
            return True  # Still parse whatever output we got
        except Exception as e:
            self.log(f"QEMU execution error: {e}", "FAIL")
            return False

    def parse_test_results(self) -> bool:
        """Parse test results from QEMU output"""
        self.log("Parsing test results...", "INFO")

        # Look for test result markers
        pass_pattern = r"✓ PASS: (.+?)(?:\s*\(|$)"
        fail_pattern = r"✗ FAIL: (.+?)(?:\s*\(|$)"

        pass_matches = re.findall(pass_pattern, self.qemu_output)
        fail_matches = re.findall(fail_pattern, self.qemu_output)

        # Also look for summary line
        summary_pattern = r"Total tests: (\d+)\s+Passed:\s+(\d+)\s+Failed:\s+(\d+)"
        summary_match = re.search(summary_pattern, self.qemu_output)

        # Record all tests
        for test_name in pass_matches:
            self.results.append(TestResult(test_name.strip(), True))

        for test_name in fail_matches:
            self.results.append(TestResult(test_name.strip(), False))

        # If no tests found, check if QEMU ran
        if not self.results:
            if "INTEGRATION TEST SUITE" in self.qemu_output:
                self.log("Tests ran but no results parsed", "WARN")
                return True
            else:
                self.log("Integration tests did not run", "FAIL")
                return False

        self.log(f"Parsed {len(self.results)} test results", "INFO")
        return True

    def print_results(self):
        """Print human-readable test results"""
        print("\n" + "="*60)
        print("INTEGRATION TEST RESULTS")
        print("="*60)

        passed = sum(1 for r in self.results if r.passed)
        failed = sum(1 for r in self.results if r.passed is False)
        total = len(self.results)

        for result in self.results:
            status = "✓" if result.passed else "✗"
            print(f"  {status} {result.name}")
            if result.reason and not result.passed:
                print(f"      Reason: {result.reason}")

        print("\n" + "-"*60)
        print(f"Total:  {total}")
        print(f"Passed: {passed}")
        print(f"Failed: {failed}")
        print("="*60 + "\n")

        if failed == 0 and total > 0:
            self.log("ALL TESTS PASSED!", "PASS")
            return True
        elif failed > 0:
            self.log(f"{failed} test(s) failed", "FAIL")
            return False
        else:
            self.log("No tests were run", "WARN")
            return False

    def generate_junit_report(self, output_file: str):
        """Generate JUnit XML report for CI/CD integration"""
        self.log(f"Generating JUnit report: {output_file}", "INFO")

        xml = '<?xml version="1.0" encoding="UTF-8"?>\n'
        xml += f'<testsuites name="System7-IntegrationTests" tests="{len(self.results)}">\n'
        xml += f'  <testsuite name="Phase1-IntegrationTests" tests="{len(self.results)}">\n'

        for result in self.results:
            xml += f'    <testcase name="{result.name}" time="0">\n'
            if not result.passed:
                xml += f'      <failure message="{result.reason}"/>\n'
            xml += '    </testcase>\n'

        xml += '  </testsuite>\n'
        xml += '</testsuites>\n'

        try:
            with open(output_file, 'w') as f:
                f.write(xml)
            self.log(f"JUnit report written: {output_file}", "PASS")
        except Exception as e:
            self.log(f"Failed to write JUnit report: {e}", "FAIL")

    def run(self) -> bool:
        """Execute full test pipeline"""
        self.log("Starting System 7 Integration Test Suite", "INFO")
        print()

        # Build kernel
        if not self.build_kernel():
            return False

        # Run tests
        if not self.run_tests():
            return False

        # Parse results
        if not self.parse_test_results():
            return False

        # Print results
        success = self.print_results()

        return success

def main():
    parser = argparse.ArgumentParser(
        description="System 7 Integration Test Runner",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  %(prog)s                              # Run tests with defaults
  %(prog)s --timeout 60                 # Use 60 second timeout
  %(prog)s --output junit.xml           # Generate JUnit report
  %(prog)s --verbose --no-build         # Verbose output, reuse existing kernel
        """
    )

    parser.add_argument("--timeout", type=int, default=120,
                      help="QEMU execution timeout in seconds (default: 120)")
    parser.add_argument("--output", type=str, metavar="FILE",
                      help="Generate JUnit XML report to FILE")
    parser.add_argument("--verbose", action="store_true",
                      help="Enable verbose output")
    parser.add_argument("--no-build", action="store_true",
                      help="Skip kernel build, use existing kernel")
    parser.add_argument("--project-root", type=str, default="/home/k/iteration2",
                      help="Project root directory")

    args = parser.parse_args()

    # Find project root if needed
    project_root = args.project_root
    if not Path(project_root).exists():
        print(f"Error: Project root not found: {project_root}", file=sys.stderr)
        return 1

    # Create test runner
    runner = TestRunner(
        project_root=project_root,
        timeout=args.timeout,
        verbose=args.verbose
    )

    # Skip build if requested
    if args.no_build:
        runner.build_success = True
    else:
        if not runner.build_kernel():
            return 1

    # Run tests
    if not runner.run_tests():
        return 1

    # Parse results
    if not runner.parse_test_results():
        return 1

    # Print results
    success = runner.print_results()

    # Generate report if requested
    if args.output:
        runner.generate_junit_report(args.output)

    return 0 if success else 1

if __name__ == "__main__":
    sys.exit(main())
