import os
import time
import argparse
import subprocess

def create_results_directory() -> str:
    output_dir = 'results_mem'

    if not os.path.exists(output_dir):
        os.mkdir(output_dir)

    return output_dir


def test_memory(dataset_path: str,
                algorith_path: str,
                output_mark: str,
                clear_msprint_output: str,
                max_from_msprint_output: str) -> None:
    output_dir = create_results_directory()
    dataset_name = os.path.basename(dataset_path)

    if output_mark is None:
        output_mark = int(time.time())

    algorith_output_path = '/dev/null'
    valgrind_output_path = os.path.join(output_dir, f'{dataset_name}_{output_mark}_valgrind.txt')
    massif_output_path = os.path.join(output_dir, f'{dataset_name}_{output_mark}_massif.txt')
    msprint_output_path = os.path.join(output_dir, f'{dataset_name}_{output_mark}_msprint.txt')
    msprint_cleaned_output_path = os.path.join(output_dir, f'{dataset_name}_{output_mark}_msprint_cleaned.txt')
    max_result_path = os.path.join(output_dir, f'{dataset_name}_{output_mark}_max_res.txt')

    subprocess.run(['valgrind', f'--log-file={valgrind_output_path}', '--tool=massif', algorith_path, dataset_path, algorith_output_path], stdout=subprocess.DEVNULL)

    massif_result = subprocess.run(['find', '-name', 'massif.out.*'], stdout=subprocess.PIPE)
    massif_result_path = massif_result.stdout.decode('utf-8').rstrip()

    subprocess.run(['mv', massif_result_path, massif_output_path])

    with open(msprint_output_path, 'w') as msprint_file:
        subprocess.run(['ms_print', massif_output_path], stdout=msprint_file)

    subprocess.run(['python3', clear_msprint_output, '-i', msprint_output_path, '-o', msprint_cleaned_output_path])
    subprocess.run(['python3', max_from_msprint_output, '-i', msprint_cleaned_output_path, '-o', max_result_path])


def configure_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description='Clean ms_print output file')

    parser.add_argument('-d', '--datasets', required=True, nargs='+', help='Paths to the datasets')
    parser.add_argument('-o', '--output_mark', required=False, default=None, help='Output files mark')

    parser.add_argument('-a', '--algorithm', type=str, required=False,
                        default='../../../build/target/Desbordante_run', help='Path to the algorithm executable file')

    parser.add_argument('-c', '--clear_msprint_output', type=str, required=False,
                        default='./clear_msprint_output.py', help='Path to the clear_msprint_output.py')

    parser.add_argument('-m', '--max_from_msprint_output', type=str, required=False,
                        default='./max_from_msprint_output.py', help='Path to the max_from_msprint_output.py')

    return parser

if __name__ == '__main__':
    parser = configure_parser()
    args = parser.parse_args()

    for dataset in args.datasets:
        test_memory(dataset,
                    args.algorithm,
                    args.output_mark,
                    args.clear_msprint_output,
                    args.max_from_msprint_output)

        dataset_name = os.path.basename(dataset)
        print(f'Tested {dataset_name}')
