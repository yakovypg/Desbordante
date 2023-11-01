import argparse
from fastod_tester import *

def configure_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description='Test FASTOD algorithm')
    parser.add_argument('-i', '--input', type=str, required=True, help='Path to the C++ algorithm implementation')
    parser.add_argument('-d', '--datasets', required=True, nargs='+', help='Paths to the datasets')

    return parser

if __name__ == '__main__':
    parser = configure_parser()
    args = parser.parse_args()
    test_c_without_second_alg(args.input, args.datasets)