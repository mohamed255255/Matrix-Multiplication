# Matrix-Multiplication_ParallelProccessing

# Overview:

Write a matrix multiplication program. Matrices’ dimensions and values are taken as
an input. Multiply the two matrices together then print the result. You need to 
implement this program in two modes. 
First mode reads the input from the console, and the second one reads the input 
from a file.

# Requiremnts:

* you are required to use inter process communication to distribute the inputs rows for the allocated (X) processes
  in order to multiply the two input matrices and send the result back to the master 

* Read the number of rows and columns of the input matrices. They are written in the first line of the file as ”row=x col=y”. The following is an example of the format on an input file.  