# Matrix-Multiplier
The purpose of this project was to develop a server and client program that carries out matrix multiplication. The master process splits the matrix multiplication into a number of pieces and sends these via sockets to a number of worker processes. The multiplication is then carried out in parallel improving speed. The workers return their respective answers which are combined by the master.

A potential improvement not employed would be to use pthreads to track each socket so that answers from each worker could be recorded in any order.
