import os
import argparse

from prettytable import PrettyTable

class DatasetInfo(object):
    def __init__(self, name: str, columns: int, rows: int, size_mb: float):
        self.name = name
        self.columns = columns
        self.rows = rows
        self.size_mb = size_mb
    
    def to_table_row(self) -> list[str]:
        return [self.name, self.columns, self.rows, self.size_mb]

def analyze_dataset(path: str, separator: str, has_header: bool) -> DatasetInfo:
    lines = None
    dataset_name = os.path.basename(path)

    with open(path, 'r') as f:
        lines = f.readlines()

    if len(lines) == 0:
        return DatasetInfo(dataset_name, 0, 0, 0)

    columns = lines[0].count(separator) + 1
    rows = (len(lines) - 1) if has_header else lines
    size = os.path.getsize(path) / (1024 * 1024.0)

    return DatasetInfo(dataset_name, columns, rows, '%.2f' % size)

def configure_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description='Test FASTOD algorithm')
    parser.add_argument('-s', '--separator', type=str, required=False, default=',', help='Dataset separator')
    parser.add_argument('--has-header', type=bool, required=False, default=True, help='Indicates whether the dataset has header')
    parser.add_argument('-d', '--datasets', required=True, nargs='+', help='Paths to the datasets')

    return parser

if __name__ == '__main__':
    parser = configure_parser()
    args = parser.parse_args()

    table = PrettyTable()
    table.field_names = ['Dataset', 'Columns', 'Rows', 'Size (MB)']

    for path in args.datasets:
        dataset_info = analyze_dataset(path, args.separator, args.has_header)
        table.add_row(dataset_info.to_table_row())
    
    print(table)
