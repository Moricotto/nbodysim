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

    plt.xlabel('Number of Bodies')
    plt.ylabel('Time per Step')
    plt.title('Time per Step for Various Algorithms')
    
    if labels:
        plt.legend()

    plt.grid()
    plt.show()


# Example usage
data1 = [0.061, 0.26, 0.60, 1.06, 1.65, 2.41, 3.26, 4.23, 5.19, 6.41]
data2 = [0.0007887, 0.00147, 0.0023394, 0.003037, 0.003801, 0.005454, 0.0055342, 0.0066441, 0.006753, 0.0102076]

plot_multiple_datasets(
    [data1, data2],
    labels=['Naive algorithm on CPU', 'Barnes-Hut on CPU']
)