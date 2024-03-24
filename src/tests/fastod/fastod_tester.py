import os
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

class ShortAlgorithmResultSummary(object):
    def __init__(self, alg_name: str):
        self.alg_name = alg_name
        self.table = PrettyTable()
        self.table.field_names = ['Dataset', alg_name]
    
    def add_result(self, dataset_name: str, alg_result: AlgorithmResult) -> None:
        time_str = '%.6f' % alg_result.time
        self.table.add_row([dataset_name, time_str])
    
    def print_summary(self) -> None:
        print(self.table)

INSERT_DATASET_LABEL = '__DATASET__'

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

def extract_algorithm_dependencies(output: str) -> list[str]:
    lines = output.split('\n')
    return list(filter(lambda t: t.startswith('{') and ('}' in t), lines))

def parse_algorithm_result(output: str) -> AlgorithmResult:
    result_prefix = 'RESULT: '
    time_prefix = 'Time='
    od_prefix = 'OD='
    fd_prefix = 'FD='
    ocd_prefix = 'OCD='

    lines = output.split('\n')
    result_lines = list(filter(lambda t: t.startswith(result_prefix), lines))

    if len(result_lines) == 0:      
        raise Exception(f'No result line found.')

    result_line = result_lines[0].replace(result_prefix, '')
    data = result_line.split(', ')

    time_data = list(filter(lambda t: t.startswith(time_prefix), data))[0].replace(time_prefix, '').replace(',', '.')
    od_data = list(filter(lambda t: t.startswith(od_prefix), data))[0].replace(od_prefix, '')
    fd_data = list(filter(lambda t: t.startswith(fd_prefix), data))[0].replace(fd_prefix, '')
    ocd_data = list(filter(lambda t: t.startswith(ocd_prefix), data))[0].replace(ocd_prefix, '')

    return AlgorithmResult(float(time_data), int(od_data), int(fd_data), int(ocd_data), None)

def execute_algorithm(dataset_path: str,
                      algorithm_execute_args: list[str],
                      algorithm_name: str,
                      output_dir: str = './') -> AlgorithmResult:
    dataset_name = os.path.basename(dataset_path)
    output_file_path = os.path.join(output_dir, f'res_{algorithm_name}_{dataset_name}.txt')

    algorithm_execute_args = [i.replace(INSERT_DATASET_LABEL, dataset_path) for i in algorithm_execute_args]
    
    result = subprocess.run(algorithm_execute_args, stdout=subprocess.PIPE)
    output = result.stdout.decode('utf-8')

    algorithm_dependencies = list(map(lambda t: t + '\n',extract_algorithm_dependencies(output)))

    with open(output_file_path, 'w') as output_file:
        output_file.writelines(algorithm_dependencies)

    algorith_result = parse_algorithm_result(output)
    algorith_result.result_path = output_file_path

    return algorith_result

def test_algorithm(impl_name: str, impl_start: list[str], datasets: list[str]) -> None:
    output_dir = create_results_directory()
    summary = ShortAlgorithmResultSummary(impl_name)

    for dataset in datasets:
        impl_res = None

        try:
            impl_res = execute_algorithm(dataset, impl_start, impl_name, output_dir)
        except Exception as ex:
            print(f'Failed to get result from {impl_name} algorithm implementation. Reason:')
            print(str(ex))
            print()

        dataset_name = os.path.basename(dataset)
        print(f'Dataset: {dataset_name}')
        
        summary.add_result(dataset_name, impl_res)

        curr_dataset_table = PrettyTable()
        curr_dataset_table.field_names = impl_res.get_table_header()
        curr_dataset_table.add_row(impl_res.to_table_row(impl_name, 0))

        print(curr_dataset_table)
        print()

    summary.print_summary()

def test_java_without_second_alg(java_impl_path: str,
                                 java_impl_class: str,
                                 datasets: list[str],
                                 java_max_heap_size: str = '12g') -> None:
    java_impl_name = 'Java'
    java_impl_start = ['java', f'-Xmx{java_max_heap_size}', '-classpath', java_impl_path, java_impl_class, INSERT_DATASET_LABEL]

    test_algorithm(java_impl_name, java_impl_start, datasets)

def test_c_without_second_alg(c_impl_path: str, datasets: list[str]) -> None:
    c_impl_name = 'C++'
    c_impl_start = ['python3', c_impl_path, '--task=od', f'--table={INSERT_DATASET_LABEL}', ',', 'True']

    test_algorithm(c_impl_name, c_impl_start, datasets)

def test_algorithms(first_impl_name: str,
                    first_impl_start: list[str],
                    second_impl_name: str,
                    second_impl_start: list[str],
                    comparer_path: str,
                    datasets: list[str]) -> None:   
    output_dir = create_results_directory()
    summary = AlgorithmResultsSummary(first_impl_name, second_impl_name)

    passed_tests = 0
    failed_tests = 0

    for dataset in datasets:
        first_impl_res = None
        second_impl_res = None

        compare_res = False
        first_impl_acc_factor = 0
        second_impl_acc_factor = 0

        try:
            first_impl_res = execute_algorithm(dataset, first_impl_start, first_impl_name, output_dir)
        except Exception as ex:
            print(f'Failed to get result from {first_impl_name} algorithm implementation. Reason:')
            print(str(ex))
            print()
        
        try:
            second_impl_res = execute_algorithm(dataset, second_impl_start, second_impl_name, output_dir)
        except Exception as ex:
            print(f'Failed to get result from {second_impl_name} algorithm implementation. Reason:')
            print(str(ex))
            print()
        
        if (first_impl_res is not None) and (second_impl_res is not None):
            compare_res = first_impl_res.equal_to(second_impl_res, comparer_path)   
            first_impl_acc_factor = second_impl_res.time / first_impl_res.time
            second_impl_acc_factor = first_impl_res.time / second_impl_res.time
        
        if first_impl_res is None:
            first_impl_res = AlgorithmResult(float('inf'), 0, 0, 0, '')
        
        if second_impl_res is None:
            second_impl_res = AlgorithmResult(float('inf'), 0, 0, 0, '')

        dataset_name = os.path.basename(dataset)
        print(f'Dataset: {dataset_name}')
        print(f'Passed: {compare_res}\n')

        if compare_res:
            passed_tests += 1
        else:
            failed_tests += 1
        
        summary.add_result(dataset_name, first_impl_res, second_impl_res)

        curr_dataset_table = PrettyTable()
        curr_dataset_table.field_names = first_impl_res.get_table_header()
        curr_dataset_table.add_row(first_impl_res.to_table_row(first_impl_name, first_impl_acc_factor))
        curr_dataset_table.add_row(second_impl_res.to_table_row(second_impl_name, second_impl_acc_factor))

        print(curr_dataset_table)
        print()

    summary.print_summary()

    print()
    print('[Summary]')
    print(f'Passed test: {passed_tests}')
    print(f'Failed test: {failed_tests}')

def test_c_vs_c(first_c_impl_path: str,
                second_c_impl_path: str,
                comparer_path: str, 
                datasets: list[str]) -> None:
    first_c_impl_name = 'C++_v1'
    first_c_impl_start = ['python3', first_c_impl_path, '--task=od', f'--table={INSERT_DATASET_LABEL}', ',', 'True']

    second_c_impl_name = 'C++_v2'
    second_c_impl_start = ['python3', second_c_impl_path, '--task=od', f'--table={INSERT_DATASET_LABEL}', ',', 'True']

    test_algorithms(
        first_c_impl_name,
        first_c_impl_start,
        second_c_impl_name,
        second_c_impl_start,
        comparer_path,
        datasets)

def test_c_vs_java(c_impl_path: str, 
                   java_impl_path: str, 
                   java_impl_class: str, 
                   comparer_path: str, 
                   datasets: list[str],
                   java_max_heap_size: str = '12g') -> None:
    c_impl_name = 'C++'
    c_impl_start = ['python3', c_impl_path, '--task=od', f'--table={INSERT_DATASET_LABEL}', ',', 'True']

    java_impl_name = 'Java'
    java_impl_start = ['java', f'-Xmx{java_max_heap_size}', '-classpath', java_impl_path, java_impl_class, INSERT_DATASET_LABEL]

    test_algorithms(
        c_impl_name,
        c_impl_start,
        java_impl_name,
        java_impl_start,
        comparer_path,
        datasets)
