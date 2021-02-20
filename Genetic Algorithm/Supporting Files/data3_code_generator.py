import csv

# Read data in from file
with open("data3.csv") as f: 
    reader = csv.reader(f)
    next(reader)

    data = []
    for row in reader:

        rules = []
        for x in range(6):
            rule = 1 if float(row[x]) >= 0.5 else 0
            rules.append(rule)

        data.append({
            "cond": rules, 
            "out": [int(cell) for cell in row[6]], 
        })

# Write data to file
i = 0
f = open("Generated Files\data3_loader.txt", "a")
f.write("   data temp;\n")
for x in data:
    print(i, x.get("cond"), x.get("out"))
    cond_arr = x.get("cond")
    out_arr = x.get("out")
    f.write("   temp = (data){.variables = {"+ str(cond_arr[0]) + ", " + str(cond_arr[1]) + ", " + str(cond_arr[2]) 
            + ", " + str(cond_arr[3]) + ", "+ str(cond_arr[4]) + ", "+ str(cond_arr[5]) +"}, .class = " + str(out_arr[0]) + "};\n")
    f.write("   data_set[" + str(i) + "] = temp;\n\n")
    i += 1
f.close()
