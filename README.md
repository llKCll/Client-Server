# Client-Server
Client lookup contents on server and choose files to download.

### Usage
ftpserver.py

1. Navigate to the directory where the file is contained.
2. Enter "python3 ftserver.py <HOST> <PORT>" of where you want the server to run.


ftpclient.c

1. Navigate to the directory where the file is found.
2. Enter "make ftclient" to create the executable. 
3. Enter "./ftclient <HOST> <PORT> <COMMAND>" if the command "-1" is used or
   "./ftclient <HOST> <PORT> <COMMAND> <FILE>" if "-g" is used.

Note: Commands "-1" or "-g" must be used.

Commands

-1: Display contents of the server.
-g: Request file to be copied.
