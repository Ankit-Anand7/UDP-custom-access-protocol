Program 2: Client using customized protocol on top of UDP protocol for requesting identification from server for access permission to the cellular network(Communication between one client and one server).



Steps to compile and run the program:

  i. Open terminal and change directory using command cd to the folder in which the programs are.
 ii. Compile server program by using command gcc -o "xyz" server2.c
iii. Compile client program by using command gcc -o "xyz" client2.c
 iv. Run server program first by using command ./"xyz" port_number
  v. Run client program after that by using command ./"xyz" localhost port_number


Notes:

   i. Make sure gcc compiler is installed in your system.
  ii. If you are using clang, use clang commands instead of gcc.
 iii. Be very precise to run server program first and not the client program.
  iv. Server program expects 2 command-line arguments, ie, executable file name and port number.
   v. Client program expects 3 command-line arguments, ie, executable file name, host name and port number.
  vi. I am going to demonstrate on my system, so both client and server will be on the same system, so hostname will be localhost.
 vii. Make sure to run both the programs on same port number, otherwise there will be no response.
viii. Data packet headers and trailers are intended to be used in both client and server and thus is defined in both files to avoid compilation errors.
  ix. Make sure to keep both the input files in the same folder/directory, not even in subdirectory.
   x. Do not change the name or extension of input files otherwise a segmentation error will pop.
  xi. To demonstrate ack_timer, just enter different port numbers in client and server programs.
 xii. Do not change name of log file. It is used in my program to store Response packet from server. I could have printed it on the terminal, but as a good practice I have stored in a separate file.
