import argparse

def compare_files(path1: str, path2: str):
    file1_lines = None
    file2_lines = None

    with open(path1) as file1:
        file1_lines = [line.rstrip() for line in file1]

    with open(path2) as file2:
        file2_lines = [line.rstrip() for line in file2]

    is_equal = True

    for i in range(min(len(file1_lines), len(file2_lines))):
        line1 = file1_lines[i]
        line2 = file2_lines[i]

        if line1 != line2:
            is_equal = False
            print(f'Line {i + 1}: SOURCE="{line1}", NEW="{line2}"')

    if len(file1_lines) != len(file2_lines):
        print(f'DIFFERENT NUMBER OF LINES: SOURCE={len(file1_lines)}, NEW={len(file2_lines)}')
    elif is_equal:
        print('Results are equal')

def configure_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description='Compare two files line by line')
    parser.add_argument('-f', '--first', type=str, required=True, help='Path to the first file')
    parser.add_argument('-s', '--second', type=str, required=True, help='Path to the second file')

    return parser

if __name__ == '__main__':
    parser = configure_parser()
    args = parser.parse_args()

    compare_files(args.first, args.second)
