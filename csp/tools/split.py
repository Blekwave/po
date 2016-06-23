"""
split.py - Cutting Stock Problem input splitter
-----------------------------------------------

This script takes a cutting stock problem (CSP) file with multiple
test cases as an input and outputs many files, each one with a single
test case, in a format which can be run by the CSP solver implemented
in this work.

Usage:
    python3 split.py input_file
    python3 split.py input_file -o output_dir/ (optional)
"""


#!/usr/bin/env python3

import os
import argparse


def parse_args():
    """Parse command-line arguments for this script."""
    p = argparse.ArgumentParser()
    a = p.add_argument
    a("input_file", help="Input file with multiple test cases.")
    a("-o", "--output_dir", default="",
      help="Output directory, relative to input file's dir "
           "(default: input file name)")
    return p.parse_args()


def split_file(input_file, out_dir):
    with open(input_file) as in_file:
        out_file = None
        in_iter = iter(in_file)
        for line in in_iter:
            if line.startswith("'"):
                # First line of case: single-quoted title
                if out_file:
                    out_file.close()
                out_filename = line[:-1].strip("'")
                out_file = open(os.path.join(out_dir, out_filename), "w")

                # Followed by number of restrictions and stock width
                restrictions = next(in_iter)
                width = next(in_iter)
                # Which must be inverted, to comply with the default format
                out_file.write(width)
                out_file.write(restrictions)
            else:
                out_file.write(line)
        out_file.close()


def main():
    args = parse_args()

    if args.output_dir:
        out_dir_name = args.output_dir
    else:
        out_dir_name = os.path.basename(args.input_file) + "_split"

    out_dir = os.path.join(os.path.dirname(args.input_file), out_dir_name)

    if not os.path.exists(out_dir):
        os.makedirs(out_dir)

    print("Input file:", args.input_file)
    print("Output directory:", out_dir)

    split_file(args.input_file, out_dir)


if __name__ == "__main__":
    main()
