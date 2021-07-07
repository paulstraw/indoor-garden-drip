envFile = open(".env", "r")

for envLine in envFile.readlines():
    [key, val] = envLine.split("=")
    print(f"-{key}='\"{val.strip()}\"'")
