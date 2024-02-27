import desbordante

TABLE = 'examples/datasets/iris.csv'

algo = desbordante.od.algorithms.Default()
algo.load_data(table=(TABLE, ',', True))
algo.execute(max_time=60)
result = algo.get_asc_ods() + algo.get_desc_ods() + algo.get_simple_ods()
print('ODs:')
for od in result:
    print(od)