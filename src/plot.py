import matplotlib.pyplot as plt

def plot_multiple_datasets(datasets, labels=None):
    """
    datasets: list of lists (each inner list is a dataset)
    labels: optional list of labels for each dataset
    """
    for i, data in enumerate(datasets):
        x = list(range(len(data)))
        
        if labels and i < len(labels):
            plt.plot(x, data, marker='o', label=labels[i])
        else:
            plt.plot(x, data, marker='o')

    plt.xlabel('Index')
    plt.ylabel('Value')
    plt.title('Multiple Broken Line Graphs')
    
    if labels:
        plt.legend()

    plt.grid()
    plt.show()


# Example usage
data1 = [0.061, 0.26, 0.60, 1.06, 1.65, 2.41, 3.26, 4.23, 5.19, 6.41]
data2 = [2, 4, 1, 6]
data3 = [3, 2, 4, 3]

plot_multiple_datasets(
    [data1, data2, data3],
    labels=['Dataset 1', 'Dataset 2', 'Dataset 3']
)