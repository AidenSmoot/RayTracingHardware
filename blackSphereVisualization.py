width = 1024
height = 768
wfile = open("blackspherevisualization.txt", "w")
rfile = open("blackdot.txt", "r")
list = []

for line in rfile:
    list.append(line.split())

index = 0
hVal = list[index][0]
hVal = int(hVal)
wVal = list[index][1]
wVal = int(wVal)
print(hVal, wVal)
print(list[1][0], list[1][1])
print(list[0:10])

for i in range(height):
    for j in range(width):
        if i == hVal and j == wVal:
            wfile.write("'")
            if index != len(list)-1:
                index += 1
                hVal = list[index][0]
                hVal = int(hVal)
                wVal = list[index][1]
                wVal = int(wVal)
        else:
            wfile.write(" ")
    wfile.write("\n")

# b.close()
# a.close()