# By: Kevin Conner

from socket import *
from time import sleep
import os, sys

# Validate args. Print a error message and exit the program if invalid. 
def argCheck(args):
    # 2 args must be used.
    if len(args) != 3:
        print("2 parameters: <HOST> <PORT>.")
        exit()

    # Ensure port number is only integers
    elif not args[2].isdigit():
        print("Please use only enter integers without spaces for the port number.")
        exit()

    else:
        return

# Send the name of the files that are stored on the server to the client to be displayed.
def displayDir(connect):
    # Files in directory.
    dirItems = os.listdir("./")

    print("Directory contents requested on port {}.".format(cPort))
    print("Sending directory contents to {}: {}.".format(cHost, cPort))

    # Send the name of the files in the directory with a \n appended except for the last file.
    for item in dirItems:
        if item == dirItems[-1]:
            msg = item
            connect.send(msg.encode())
        else:
            msg = item + "\n"
            connect.send(msg.encode())

# Sends a requested file to be downloaded by the client.
def sendFile(f, connect, port):
    # Full file path of the file.
    fpath = os.getcwd() + "/" + file

    print("File {} requested on port {}.".format(file, port))
    
    # If the file exists in the current directory.
    if os.path.isfile(fpath):

        print("Sending {} to {}: {}.".format(file, cHost, port))

        # Open the file and send the content.
        with open(file) as f:

            content = f.readlines()

            for line in content:
                conn.send(line.encode())
    
    # File doesn't exist in the current directory. Send error message to client.
    else:
        conn.send("FILE NOT FOUND".encode())
        print("{} not found. Sending error message to {}: {}.".format(file, cHost, port))

# Return true if command is valid, false otherwise
def valCommand(clientCmd, connect):

    if clientCmd == "-ls":
        return
    
    elif clientCmd == "-dl":
        return

    print("{} is an invalid command. Sending error message to {}: {}.".format(command, cHost, cPort))
    connect.send("INVALID COMMAND".encode())
    exit(1)

if __name__ == "__main__":
    # Validate args.
    argCheck(sys.argv)

    host = sys.argv[1]
    port = sys.argv[2]
    socket = socket(AF_INET, SOCK_STREAM)
    socket.bind((host, int(port)))

    # Listen for a client. 
    socket.listen(1)
    print("Server open on", str(port) + ".")
    conn, addr = socket.accept()

    # Receive request from the client.
    data = conn.recv(1024).decode()

    # Parse clients request and append to paramsLst.
    paramsLst = [ s.strip() for s in data.splitlines() ]

    cArgs = paramsLst[0]
    command = paramsLst[1]
    cHost = paramsLst[2]
    cPort = conn.getsockname()[1]

    print("Connection with {} established.".format(cHost))

    # Validate command. Send error message to client if invalid and exit.
    valCommand(command, conn)
        
    # Command to send contents of directory to client.
    if command == "-ls":
        displayDir(conn)

    # Command to send requested file to client.       
    elif command == "-dl":
        # File requested by client
        file = paramsLst[3]
        sendFile(file, conn, cPort)
           
    # Close connection. 
    conn.close()
    print("Connection with {} closed.".format(cHost))




