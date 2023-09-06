import os
import argparse
import subprocess

from prettytable import PrettyTable

class AlgorithmResult(object):
    def __init__(self, time: float, od: int, fd: int, ocd: int, result_path: str):
        self.time = time
        self.od = od
        self.fd = fd
        self.ocd = ocd
        self.result_path = result_path

    def equal_to(self, other, comparer_path: str) -> bool:
        return (self.od == other.od
            and self.fd == other.fd
            and self.ocd == other.ocd
            and is_algorithm_results_equal(self.result_path, other.result_path, comparer_path))

    def to_table_row(self, algorithm_name: str, acc_factor: float) -> list[str]:
        time_str = '%.6f' % self.time
        acc_factor_str = '%.2fx' % acc_factor

        return [algorithm_name, time_str, acc_factor_str, str(self.od), str(self.fd), str(self.ocd)]

    def get_table_header(self) -> list[str]:
        return ['Algorithm', 'Time', 'Acc factor', 'OD', 'FD', 'OCD']

class AlgorithmResultsSummary(object):
    def __init__(self, first_alg_name: str, second_alg_name: str):
        self.first_alg_name = first_alg_name
        self.second_alg_name = second_alg_name

        self.table = PrettyTable()
        self.table.field_names = ['Dataset', first_alg_name, second_alg_name, 'Acc factor']
    
    def add_result(self, dataset_name: str,
                   first_alg_result: AlgorithmResult,
                   second_alg_result: AlgorithmResult,
                   is_first_alg_current: bool = True) -> None:
        first_time_str = '%.6f' % first_alg_result.time
        second_time_str = '%.6f' % second_alg_result.time
        
        acc_factor = (second_alg_result.time / first_alg_result.time
            if is_first_alg_current
            else first_alg_result.time / second_alg_result.time)
        
        acc_factor_str = '%.2fx' % acc_factor
        
        self.table.add_row([dataset_name, first_time_str, second_time_str, acc_factor_str])
    
    def print_summary(self) -> None:
        print(self.table)

def create_results_directory() -> str:
    output_dir = 'results'

    if not os.path.exists(output_dir):
        os.mkdir(output_dir)

    return output_dir

def is_algorithm_results_equal(first_result_path: str, second_result_path: str, comparer_path: str) -> bool:
    info_prefix = 'INFO: '

    result = subprocess.run(['bash', comparer_path, first_result_path, second_result_path], stdout=subprocess.PIPE)
    output = result.stdout.decode('utf-8')

    lines = output.split('\n')
    equality_data = list(filter(lambda t: t.startswith(info_prefix), lines))[0].replace(info_prefix, '')

    return equality_data == 'Equal'

def parse_algorithm_output(output: str) -> AlgorithmResult:
    result_prefix = 'RESULT: '
    time_prefix = 'Time='
    od_prefix = 'OD='
    fd_prefix = 'FD='
    ocd_prefix = 'OCD='

    lines = output.split('\n')
    result_line = list(filter(lambda t: t.startswith(result_prefix), lines))[0].replace(result_prefix, '')

    data = result_line.split(', ')
    time_data = list(filter(lambda t: t.startswith(time_prefix), data))[0].replace(time_prefix, '').replace(',', '.')
    od_data = list(filter(lambda t: t.startswith(od_prefix), data))[0].replace(od_prefix, '')
    fd_data = list(filter(lambda t: t.startswith(fd_prefix), data))[0].replace(fd_prefix, '')
    ocd_data = list(filter(lambda t: t.startswith(ocd_prefix), data))[0].replace(ocd_prefix, '')

    return AlgorithmResult(float(time_data), int(od_data), int(fd_data), int(ocd_data), None)

def execute_algorithm(dataset_path: str, algorithm_execute_args: list[str], algorithm_name: str, output_dir: str = './') -> AlgorithmResult:
    dataset_name = os.path.basename(dataset_path)
    output_file_path = os.path.join(output_dir, f'res_{algorithm_name}_{dataset_name}.txt')

    args = [dataset_path, output_file_path]
    result = subprocess.run(algorithm_execute_args + args, stdout=subprocess.PIPE)
    output = result.stdout.decode('utf-8')

    algorith_result = parse_algorithm_output(output)
    algorith_result.result_path = output_file_path

    return algorith_result

def test_algorithms(c_impl_path: str, java_impl_path: str, java_impl_class, comparer_path: str, datasets: list[str]) -> None:
    output_dir = create_results_directory()

    passed_tests = 0
    failed_tests = 0

    c_impl_start = [c_impl_path]
    java_impl_start = ['java', '-classpath', java_impl_path, java_impl_class]

    c_impl_name = 'C++'
    java_impl_name = 'Java'

    summary = AlgorithmResultsSummary(c_impl_name, java_impl_name)

    for dataset in datasets:
        c_impl_res = execute_algorithm(dataset, c_impl_start, c_impl_name, output_dir)
        java_impl_res = execute_algorithm(dataset, java_impl_start, java_impl_name, output_dir)
        compare_res = c_impl_res.equal_to(java_impl_res, comparer_path)

        c_impl_acc_factor = java_impl_res.time / c_impl_res.time
        java_impl_acc_factor = c_impl_res.time / java_impl_res.time

        dataset_name = os.path.basename(dataset)
        print(f'Dataset: {dataset_name}')
        print(f'Passed: {compare_res}\n')

        if compare_res:
            passed_tests += 1
        else:
            failed_tests += 1
        
        summary.add_result(dataset_name, c_impl_res, java_impl_res)

        curr_dataset_table = PrettyTable()
        curr_dataset_table.field_names = c_impl_res.get_table_header()
        curr_dataset_table.add_row(c_impl_res.to_table_row(c_impl_name, c_impl_acc_factor))
        curr_dataset_table.add_row(java_impl_res.to_table_row(java_impl_name, java_impl_acc_factor))

        print(curr_dataset_table)
        print()

    summary.print_summary()

    print()
    print('[Summary]')
    print(f'Passed test: {passed_tests}')
    print(f'Failed test: {failed_tests}')

def configure_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description='Test FASTOD algorithm')
    parser.add_argument('-c', '--c', type=str, required=True, help='Path to the C++ algorithm implementation')
    parser.add_argument('-j', '--java', type=str, required=True, help='Path to the bin directory containing Java algorithm implementation')
    parser.add_argument('-p', '--package', type=str, required=True, help='Name of the Java algorithm implementation class')
    parser.add_argument('-C', '--comparer', type=str, required=True, help='Path to the algorithm results comparer')
    parser.add_argument('-d', '--datasets', required=True, nargs='+', help='Paths to the datasets')

    return parser

if __name__ == '__main__':
    parser = configure_parser()
    args = parser.parse_args()
    test_algorithms(args.c, args.java, args.package, args.comparer, args.datasets)
