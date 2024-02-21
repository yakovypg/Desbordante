import argparse
from prettytable import PrettyTable

class ResultInfo(object):
    def __init__(self,
                 time: int,
                 total_memory: int,
                 useful_heap: int,
                 extra_heap: int,
                 time_index: int,
                 total_memory_index: int,
                 useful_heap_index: int,
                 extra_heap_index: int):
        self.time = time
        self.total_memory = total_memory
        self.useful_heap = useful_heap
        self.extra_heap = extra_heap
        self.time_index = time_index
        self.total_memory_index = total_memory_index
        self.useful_heap_index = useful_heap_index
        self.extra_heap_index = extra_heap_index

    def __str__(self) -> str:
        table = PrettyTable()
        table.field_names = ['time(i)', 'total(B)', 'useful-heap(B)', 'extra-heap(B)']

        time = f'{self.time} [{self.time_index}]'
        total_memory = f'{self.total_memory} [{self.total_memory_index}]'
        useful_heap = f'{self.useful_heap} [{self.useful_heap_index}]'
        extra_heap = f'{self.extra_heap} [{self.extra_heap_index}]'

        table.add_row([time, total_memory, useful_heap, extra_heap])
        return str(table)

def find_max_result(input_path: str, output_path: str) -> None:
    time_list = []
    total_memory_list = []
    useful_heap_list = []
    extra_heap_list = []

    with open(input_path, 'r') as reader:
        snapshot_index = 0

        for line in reader:
            data = list(map(lambda t: t.replace(',', ''), line.split()))

            if (len(data) == 6 and data[0] == str(snapshot_index)):
                snapshot_index += 1
                time_list.append(int(data[1]))
                total_memory_list.append(int(data[2]))
                useful_heap_list.append(int(data[3]))
                extra_heap_list.append(int(data[4]))

    max_time = max(time_list)
    max_total_memory = max(total_memory_list)
    max_useful_heap = max(useful_heap_list)
    max_extra_heap = max(extra_heap_list)

    max_time_index = time_list.index(max_time)
    max_total_memory_index = total_memory_list.index(max_total_memory)
    max_useful_heap_index = useful_heap_list.index(max_useful_heap)
    max_extra_heap_index = extra_heap_list.index(max_extra_heap)

    result = ResultInfo(max_time,
                        max_total_memory,
                        max_useful_heap,
                        max_extra_heap,
                        max_time_index,
                        max_total_memory_index,
                        max_useful_heap_index,
                        max_extra_heap_index)

    result_str = str(result)

    if output_path is None:
        print(result_str)
    else:
        with open(output_path, 'w') as writer:
            writer.write(result_str + '\n')


def configure_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description='Clean ms_print output file')
    parser.add_argument('-i', '--input', type=str, required=True, help='Path to the ms_print output file')
    parser.add_argument('-o', '--output', type=str, required=False, default=None, help='Path to the output file')

    return parser

if __name__ == '__main__':
    parser = configure_parser()
    args = parser.parse_args()
    find_max_result(args.input, args.output)
