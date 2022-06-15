import os

def main(argv):
    n = int(argv[1])
    for i in range(n):
        os.system("./client client 2")

if __name__ == "__main__":
    main(os.sys.argv)