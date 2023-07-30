import time

start = time.time()
total = 0
for i in range(10000): total += i
print(total, (time.time() - start) * 1000, "ms")