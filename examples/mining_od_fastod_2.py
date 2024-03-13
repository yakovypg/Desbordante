import desbordante
import pandas
from tabulate import tabulate

TABLE = 'examples/datasets/position_distribution.csv'
TIME_LIMIT_SECONDS = 3

COLOR_CODES = {
    'bold_red': '\u001b[1;31m',
    'bold_green': '\033[1;32m',
    'bold_yellow': '\033[1;33m',
    'bold_blue': '\033[1;34m',
    'bold_magenta': '\033[1;35m',
    'bold_cyan': '\033[1;36m',
    'bold_white': '\033[1;37m',
    'red': '\u001b[31m',
    'green': '\033[32m',
    'yellow': '\033[33m',
    'blue': '\033[34m',
    'magenta': '\033[35m',
    'cyan': '\033[36m',
    'white': '\033[37m',
    'default': '\033[0m'
}

def make_text_colored(text, color):
    return f'{COLOR_CODES[color]}{text}{COLOR_CODES["default"]}'

def print_data_frame(data_frame, title = None):
    print_table(data_frame, 'keys', title)

def print_table(table, headers = None, title = None):
    if title is not None:
        print(title)
    
    print(tabulate(table, headers=headers, tablefmt='psql'))

def print_attribute_symbols(table):
    print('Attribute symbols:')
    
    counter = 1

    for column in table:
        print(f'{column} -- {counter}')
        counter += 1

def print_desc_ods_with_comments(desc_ods):
    print('descending ods:', len(desc_ods))

def print_asc_ods_with_comments(asc_ods, table):
    print('ascending ods:', len(asc_ods))

    for od in asc_ods:
        print(od)

    print()
    print(f'Dependency "{asc_ods[0]}" means that ordering the table')
    print('by attribute "percent" automatically entails ordering by')
    print('attribute "position". Moreover, this is observed regardless')
    print('of other attributes, since the dependency context is empty.')

    print()
    print('Let\'s sort it by attribute "percent".')

    table_sorted = table.sort_values('percent')

    print()
    print_data_frame(table_sorted, 'Sorted table:')

    print()
    print('We can see that this sort entails automatic ordering by')
    print('attribute "position".')

    print()
    print(f'Dependency "{asc_ods[1]}" is similar to the first and means')
    print('that ordering the table by attribute "position" automatically')
    print('entails ordering by attribute "percent". This can be seen in')
    print('the table above.')

    print()
    print('In other words, these dependencies indicate that the ordering of')
    print('percents entails an automatic ordering of the positions and')
    print('vice versa.')

def print_simple_ods_with_comments(simple_ods, table):
    print('simple ods:', len(simple_ods))

    for od in simple_ods:
        print(od)

    print()
    print(f'Dependency "{simple_ods[0]}" means that inside each equivalence')
    print('class from "percent" the constancy of the attribute "position"')
    print('can be traced.')

    percent_values = list(table['percent'])
    percent_classes = set([f'class [{i}] with {percent_values.count(i)} element{"" if percent_values.count(i) == 1 else "s"}'
                           for i in percent_values])

    print()
    print('We have 5 equivalence classes in "percent":')

    for c in percent_classes:
        print(c)

    print()
    print('This table shows the constancy of values from attribute "position"')
    print('within each equivalence class from "percent". For clarity, lines')
    print('containing different equivalence classes are colored differently.')

    table_headers = [i for i in table]
    table_rows = [list(str(i) for i in r) for r in table.values]

    table_rows[0] = [make_text_colored(i, 'bold_red') for i in table_rows[0]]
    table_rows[1] = [make_text_colored(i, 'green') for i in table_rows[1]]
    table_rows[2] = [make_text_colored(i, 'yellow') for i in table_rows[2]]
    table_rows[3] = [make_text_colored(i, 'blue') for i in table_rows[3]]
    table_rows[4] = [make_text_colored(i, 'magenta') for i in table_rows[4]]
    table_rows[5] = [make_text_colored(i, 'bold_red') for i in table_rows[5]]

    print()
    print_table(table_rows, table_headers)

if __name__ == '__main__':
    algo = desbordante.od.algorithms.Fastod()
    algo.load_data(table=(TABLE, ',', True))
    algo.execute(time_limit=TIME_LIMIT_SECONDS)

    asc_ods = algo.get_asc_ods()
    desc_ods = algo.get_desc_ods()
    simple_ods = algo.get_simple_ods()

    table = pandas.read_csv(TABLE)

    print_data_frame(table)
    print()
    print_attribute_symbols(table)
    print()
    print_desc_ods_with_comments(desc_ods)
    print()
    print_asc_ods_with_comments(asc_ods, table)
    print()
    print_simple_ods_with_comments(simple_ods, table)
