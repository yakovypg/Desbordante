import desbordante

TABLE = 'examples/datasets/iris.csv'
TIME_LIMIT = 3

algo = desbordante.od.algorithms.Fastod()
algo.load_data(table=(TABLE, ',', True))
algo.execute(time_limit=TIME_LIMIT)
result = algo.get_asc_ods() + algo.get_desc_ods() + algo.get_simple_ods()
print('ODs:')
for od in result:
    print(od)