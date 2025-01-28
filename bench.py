import subprocess

print("running benchmarks...")
cmd ='make bench'

results = {}
for i in range(10):
    proc = subprocess.Popen(cmd,shell=True,stdout=subprocess.PIPE)
    for line in proc.stdout:
        line = line.decode('utf-8')
        if 'HM_Bench' in line and 'ns' in line:
            _,latter = line.split('.')
            function, time_str = latter.split(' ')
            time = float(time_str[1:-4])
            if function not in results.keys():
                results[function] = []
            results[function].append(time)
print("done")

print("results:")
for function in results.keys():
    avg = sum(results[function])/len(results[function])
    avg_ms = avg / 1000000.0
    print(f'{function:16}: {avg_ms:>8.2f} ms')
