import csv

# Read data in from file
with open("data1.csv") as f: 
    reader = csv.reader(f)
    next(reader)

    data = []
    for row in reader:
        data.append({
            "cond": [int(cell) for cell in row[:5]], 
            "out": [int(cell) for cell in row[5]], 
        })

# Write data to file
i = 0
f = open("Generated Files\data1_loader.txt", "a")
f.write("   data temp;\n")
for x in data:
    print(i, x.get("cond"), x.get("out"))
    cond_arr = x.get("cond")
    out_arr = x.get("out")
    f.write("   temp = (data){.variables = {"+ str(cond_arr[0]) + ", " + str(cond_arr[1]) + ", " + str(cond_arr[2]) 
            + ", " + str(cond_arr[3]) + ", "+ str(cond_arr[4]) + "}, .class = " + str(out_arr[0]) + "};\n")
    f.write("   data_set[" + str(i) + "] = temp;\n\n")
    i += 1
f.close()
