import csv
import tensorflow as tf
from sklearn.model_selection import train_test_split
from tensorflow.keras.callbacks import TensorBoard
import time
import numpy as np

# Read data in from file
with open("data3.csv") as f: 
    reader = csv.reader(f)
    next(reader)

    data = []
    for row in reader:
        data.append({
            "condition": [float(cell) for cell in row[:6]],
            "output": 1 if row[6] == "1" else 0 
        })

NAME = "Plot-{}".format(int(time.time()))

tensorboard = TensorBoard(log_dir='logs/{}'.format(NAME))

# Populate numpy arrays with the data
condition = [row["condition"] for row in data]
output = [row["output"] for row in data]
x = np.array(condition)
y = np.array(output)

# Create a sequential neural network
model = tf.keras.models.Sequential()

# Two hidden layers with 10 neurons, with Tanh activation function
model.add(tf.keras.layers.Dense(10, input_shape=(6,), activation="tanh")) # can choose Flatten or Dense
model.add(tf.keras.layers.Dense(10, input_shape=(6,), activation="tanh")) # 10 is number of neurons

# An output layer with 1 neuron, with sigmoid activation
model.add(tf.keras.layers.Dense(1, activation="sigmoid"))

# Train neural network
model.compile(
    optimizer="adam",
    loss="binary_crossentropy", 
    metrics=["accuracy"]
)

model.fit(x, y, epochs=1000, validation_split=0.33, callbacks=[tensorboard])
