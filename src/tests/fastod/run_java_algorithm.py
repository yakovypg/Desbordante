import argparse
from fastod_tester import *

def configure_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description='Test FASTOD algorithm')
    parser.add_argument('-i', '--input', type=str, required=True, help='Path to the bin directory containing Java algorithm implementation')
    parser.add_argument('-d', '--datasets', required=True, nargs='+', help='Paths to the datasets')
    parser.add_argument('-p', '--package', type=str, required=True, help='Name of the Java algorithm implementation class')
    parser.add_argument('--heap', type=str, required=False, default='12g', help='Maximum java heap size')

    return parser

if __name__ == '__main__':
    parser = configure_parser()
    args = parser.parse_args()
    test_java_without_second_alg(args.input, args.package, args.datasets, args.heap)