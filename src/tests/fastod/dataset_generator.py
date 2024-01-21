import os
import random
import argparse

class DatasetGenerator(object):
    def __init__(self,
                 rows_num: int,
                 columns_num: int,
                 min_value: int,
                 max_value: int,
                 add_header: bool,
                 separator: str):
        if rows_num <= 0:
            raise RuntimeError("Number of rows must be greater than zero.")

        if columns_num <= 0:
            raise RuntimeError("Number of rows must be greater than zero.")
        
        if max_value < min_value:
            (min_value, max_value) = (max_value, min_value)
        
        self.rows_num = rows_num
        self.columns_num = columns_num
        self.min_value = min_value
        self.max_value = max_value
        self.add_header = add_header
        self.separator = separator
    
    def write_header(self, file) -> None:
        header = [f'c{i}' for i in range(1, self.columns_num + 1)]
        self.write_list(file, header)
    
    def write_list(self, file: str, lst: list[object]) -> None:
        for v in lst:
            if type(v) == type(1.4):
                print(type(v))
        str_lst = [str(i) for i in lst]
        self.write_str(file, self.separator.join(str_lst))
    
    def write_str(self, file, string: str) -> None:
        file.write(string + os.linesep)
    
    def mix_rows(self, matrix: list[list[int]]) -> None:
        rows_num = len(matrix)
        
        for i in range(rows_num):
            swap_index = random.randint(0, rows_num - 1)     

            if swap_index == i:
                continue

            tmp = matrix[i]
            matrix[i] = matrix[swap_index]
            matrix[swap_index] = tmp
    
    def generate_start_value(self) -> int:
        return random.randint(self.min_value, self.max_value)
    
    def generate_next_value(self, prev_value: int, step_coef: int = 1) -> int:
        next_value = prev_value + random.randint(0, 100) * step_coef

        if next_value > self.max_value:
            next_value = self.max_value
        elif next_value < self.min_value:
            next_value = self.min_value
        
        return next_value
    
    def generate_ordered_list(self, length: int) -> list[int]:
        if length <= 0:
            raise RuntimeError("Length of the list must be greater than zero.")
        
        start_value = self.generate_start_value()
        step_coef = int(-1 ** start_value)

        column = [start_value]
        prev_value = start_value

        for _ in range(1, length):
            curr_value = self.generate_next_value(prev_value, step_coef)
            
            column.append(curr_value)
            prev_value = curr_value
        
        return column

    def generate_const_list(self, length: int) -> list[int]:
        if length <= 0:
            raise RuntimeError("Length of the list must be greater than zero.")
        
        const_value = self.generate_start_value()

        return [const_value for _ in range(length)]

    def generate_range_based_list(self, length: int, ranges_num: int) -> list[int]:
        if length <= 0:
            raise RuntimeError("Length of the list must be greater than zero.")
        
        if ranges_num > length:
            raise RuntimeError("Number of ranges is greater than the length of the list.")
        
        column = []
        range_size = length // ranges_num

        for _ in range(ranges_num):
            column += self.generate_const_list(range_size)
        
        if len(column) != length:
            column += self.generate_const_list(length - len(column))
        
        return column

    def generate_random_list(self, length: int) -> list[int]:
        if length <= 0:
            raise RuntimeError("Length of the list must be greater than zero.")
        
        return [random.randint(self.min_value, self.max_value) for _ in range(length)]
    
    def generate_chaotic(self, output_path: str) -> None:
        with open(output_path, 'w') as file:
            if self.add_header:
                self.write_header(file)
            
            for _ in range(self.rows_num):
                row = self.generate_random_list(self.columns_num)
                self.write_list(file, row)
    
    def generate_range_based(self, output_path: str, range_based_columns_num: int = -1) -> None:
        with open(output_path, 'w') as file:
            if self.add_header:
                self.write_header(file)
            
            if range_based_columns_num < 0:
                range_based_columns_num = random.randint(1, self.columns_num)
            elif range_based_columns_num > self.columns_num:
                range_based_columns_num = self.columns_num
            
            remaining_columns_num = self.columns_num - range_based_columns_num
            ranges_num = self.rows_num // 200

            if ranges_num <= 1:
                ranges_num = 2

            range_based_columns = [self.generate_range_based_list(self.rows_num, ranges_num) for _ in range(range_based_columns_num)]
            random_columns = [self.generate_random_list(self.rows_num) for _ in range(remaining_columns_num)]
            matrix = range_based_columns + random_columns

            self.mix_rows(matrix)

            for i in range(self.rows_num):
                row = [matrix[j][i] for j in range(self.columns_num)]
                self.write_list(file, row)
    
    def generate(self, output_path: str, ordered_columns_num: int = -1) -> None:
        with open(output_path, 'w') as file:
            if self.add_header:
                self.write_header(file)
            
            if ordered_columns_num < 0:
                ordered_columns_num = random.randint(1, self.columns_num)
            elif ordered_columns_num > self.columns_num:
                ordered_columns_num = self.columns_num
            
            remaining_columns_num = self.columns_num - ordered_columns_num

            ordered_columns = [self.generate_ordered_list(self.rows_num) for _ in range(ordered_columns_num)]
            random_columns = [self.generate_random_list(self.rows_num) for _ in range(remaining_columns_num)]
            matrix = ordered_columns + random_columns

            self.mix_rows(matrix)

            for i in range(self.rows_num):
                row = [matrix[j][i] for j in range(self.columns_num)]
                self.write_list(file, row)

def configure_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description='Generate dataset for FASTOD algorithm')
    parser.add_argument('-r', '--rows', type=int, required=True, help='Number of rows in the dataset')
    parser.add_argument('-c', '--columns', type=int, required=True, help='Number of columns in the dataset')
    parser.add_argument('-o', '--output', type=str, required=True, help='Path to the output dataset')
    parser.add_argument('-s', '--separator', type=str, required=False, default=',', help='Dataset separator')
    parser.add_argument('--orderedcols', type=int, required=False, default=-1, help='Number of ordered columns in the dataset')
    parser.add_argument('--rangebasedcols', type=int, required=False, default=-1, help='Number of range based columns in the dataset')
    parser.add_argument('--min', type=int, required=False, default=-2**30, help='Minimum dataset value')
    parser.add_argument('--max', type=int, required=False, default=2**30, help='Maximum dataset value')
    parser.add_argument('--header', type=bool, required=False, default=True, help='Need to add a header to the dataset')

    return parser

if __name__ == '__main__':
    parser = configure_parser()
    args = parser.parse_args()

    generator = DatasetGenerator(args.rows,
                                 args.columns,
                                 args.min,
                                 args.max,
                                 args.header,
                                 args.separator)
    
    # generator.generate_chaotic(args.output)
    # generator.generate(args.output, args.orderedcols)
    generator.generate_range_based(args.output, args.rangebasedcols)
