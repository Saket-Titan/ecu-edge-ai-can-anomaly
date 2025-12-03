import numpy
import tensorflow as tf
from tensorflow.keras import layers, models


input_bytes = 8;
bottleneck_bytes = 4;

def read_file (filename):
  # print("Reading file: " + filename)

  final_data = []

  f = open(filename, "r")

  for line in f:
    if "Data: [" in line:
      data = line.split("[")[3].split(']')[0]

      hex_data = data.split()

      number_list = []
      for hex_number in hex_data:
        data_value = int(hex_number, 16)
        number_list.append(data_value)

      while len(number_list) < input_bytes:
        number_list.append(0)

      final_data.append(number_list)

  f.close()
  return final_data


normal_list = read_file("normal.txt")
attack_list = read_file("attack.txt")

train_data = numpy.array(normal_list)/255.0
test_data = numpy.array(attack_list)/255.0

# print("Data is Ready.")

model = models.Sequential()
model.add(layers.Input(shape=(8,)))
model.add(layers.Dense(4, activation='relu'))
model.add(layers.Dense(8, activation='sigmoid'))

model.compile(optimizer='adam', loss='mae')

# print("Starting Training...")
model.fit(train_data, train_data, epochs=50,verbose=0)
# print("Training finished.")

predictions = model.predict(train_data)

all_errors = []

for real,pred in zip(train_data, predictions):
  #print(real,pred)
  diff = numpy.abs(real-pred)
  row_error = numpy.mean(diff)
  all_errors.append(row_error)

#print(all_errors)

all_errors_np = numpy.array(all_errors)

# print(all_errors_np)

average_error = numpy.mean(all_errors_np)
std_dev = numpy.std(all_errors_np)

# print(average_error +  std_dev)

limit = average_error + (2 * std_dev)

# print("Error Limit: " + str(limit))

attack_predictions = model.predict(test_data)

attack_caught = 0

for real, pred in zip(test_data, attack_predictions):
  diff = numpy.abs(real-pred)
  error = numpy.mean(diff)

  if(error > limit):
    attack_caught += 1

# print("Attacks Caught: " + str(attack_caught))

layer_count = 0

for layer in model.layers:
  weights = layer.get_weights()

  if len(weights)>0:

    w  = weights[0]
    b  = weights[1]

    print("//layer" + str(layer_count) + "Weights")
    print("const float layer_"+ str(layer_count) + "_w[] = {")
    w.flat = w.flatten()
    for val in w.flat:
      print(str(val) + "f, ",end="")
    print("};")


    print("//layer" + str(layer_count) + "Biases")
    print("const float layer_"+ str(layer_count) + "_b[] = {")
    b.flat = b.flatten()
    for val in b.flat:
      print(str(val) + "f, ",end="")
    print("};")

    layer_count += 1
  
  print("TFLite Model (model_data.h)")

converter = tf.lite.TFLiteConverter.from_keras_model(model)
tflite_model = converter.convert()

# print(tflite_model)

f = open("model_data.h","w")
f.write("#ifndef MODEL_DATA_H\n")
f.write("#define MODEL_DATA_H\n")
f.write("#include <stdlign.h>\n\n")

f.write("const unsigned char g_model[] __attribute__((aligned(16))) = {\n")

hex_line = ""
byte_count = 0

for byte_val in tflite_model:
  hex_line += "0x{:02x}, ".format(byte_val)
  byte_count += 1


  if byte_count % 12 == 0:
    f.write(hex_line + "\n")
    hex_line = ""
if len(hex_line) > 0:
  f.write(hex_line + "\n")

f.write("};\n\n")

f.write("const int g_model_len = " + str(len(tflite_model)) + ";\n")
f.write("#endif\n")

f.close()

