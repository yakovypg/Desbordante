'''
valgrind --tool=massif ./Desbordante_run [DATASET_PATH] [ALG_OUTPUT_FILE_PATH]
ms_print massif.out > [PATH_TO_MSPRINT_OUTPUT_FILE]
python3 clear_msprint_output.py -i [PATH_TO_MSPRINT_OUTPUT_FILE] -o [OUTPUT_PATH]
'''

import argparse

def clear_output(input_path: str, output_path: str, start_line: int) -> None:
    with open(input_path, 'r') as reader:
        with open(output_path, 'w') as writer:
            line_num = 0

            for line in reader:
                line_num += 1

                if line_num < start_line:
                    writer.write(line)
                    continue

                stripped_line = line.strip()
                is_table = line.startswith('--')
                is_important = line.startswith(' ') and not stripped_line.startswith('-') and not stripped_line.startswith('|')

                if (is_important or is_table):
                    writer.write(line)

def configure_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description='Clean ms_print output file')
    parser.add_argument('-i', '--input', type=str, required=True, help='Path to the ms_print output file')
    parser.add_argument('-o', '--output', type=str, required=True, help='Path to the output file')
    parser.add_argument('-s', '--start', type=int, required=False, default=35, help='Start line')

    return parser

if __name__ == '__main__':
    parser = configure_parser()
    args = parser.parse_args()
    clear_output(args.input, args.output, args.start)
