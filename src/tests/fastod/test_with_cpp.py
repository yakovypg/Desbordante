import argparse
from fastod_tester import *

def configure_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description='Test FASTOD algorithm')
    parser.add_argument('-f', '--first', type=str, required=True, help='Path to the first C++ algorithm implementation')
    parser.add_argument('-s', '--second', type=str, required=True, help='Path to the second C++ algorithm implementation')
    parser.add_argument('-c', '--comparer', type=str, required=True, help='Path to the algorithm results comparer')
    parser.add_argument('-d', '--datasets', required=True, nargs='+', help='Paths to the datasets')

    return parser

if __name__ == '__main__':
    parser = configure_parser()
    args = parser.parse_args()
    test_c_vs_c(args.first, args.second, args.comparer, args.datasets)