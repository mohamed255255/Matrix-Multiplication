#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include<stdbool.h>



int rank , size ,
row , col ,
row2 , col2 ; 
int **matrix , **matrix2 , **resultMatrix ;


int max(int a, int b) {
    return (a > b) ? a : b;
}

void allocate(int*** a, int row, int col) {
    *a = (int**)malloc(row * sizeof(int*));
    for (int i = 0; i < row; i++) {
        (*a)[i] = (int*)malloc(col * sizeof(int));
    }
}

void deallocate(int** arr , int row ){
   for (int i = 0; i < row; i++) {
            free(arr[i]);
            }
   free(arr);
} 

void reset_array(int*** a , int row , int col){
  for(int i = 0 ; i < row ; i++)
    for(int j = 0 ; j < col ; j++)
        (*a)[i][j] = 0 ;
  
}

void FileBehaviour(){
    char save[10000];
   
    
    FILE *file = fopen( "a.txt", "r" ); /// creating a file for the 1st matrix
    if (file == NULL)
    {
        printf("Can't open the file.\n");
        exit(-1);
    }
   
    fscanf(file, "%[^\n]", save); 
    if(  sscanf(save, "row=%d col=%d", &row , &col) != 2 ) 
    {
        printf("Invalid first line format");
        exit(1);
    }
     
    /// build matrix1 
    allocate(&matrix , row , col );

     ///reading matrix1 from the file.
    for(int i =0 ; i < row ; i++)
        for(int j = 0 ; j < col ; j++ )
            fscanf(file, "%d", &matrix[i][j]);  
   
   
    fclose(file);
    
   

    memset(save , 0 , sizeof(save)); // reset save array to read from new file 
    file = fopen( "b.txt", "r" );    //second file of the second matrix   
    if (file == NULL)
    {
        printf("Can't open the file.\n");
        exit(-1);
    }

    fscanf(file, "%[^\n]", save);
    if(  sscanf(save, "row=%d col=%d", &row2 , &col2 ) != 2 ) 
    {
        printf("Invalid first line format");
        exit(1);
    }
   
   /// build 2nd matrix
   allocate(&matrix2 , row2 , col2 );
   ///reading matrix2 from the file.
    for(int i =0 ; i < row2 ; i++)
        for(int j = 0 ; j < col2 ; j++ )
            fscanf(file, "%d", &matrix2[i][j]);
  
  
    fclose(file);
}


void ConsoleBehaviour(){
    printf("Please enter dimensions of the first matrix: ");

    scanf("%d%d",&row , &col);

    //building the first matrix.

    allocate(&matrix , row , col );

    //taking array elements as input.
    printf("Please enter its elements: \n");
  

     for (unsigned int i = 0; i < row; i++){
       for (unsigned int j = 0; j < col; j++)
         {
         scanf("%d",&matrix[i][j]);
        }   
     }

    printf("Please enter dimensions of the second matrix: ");
    scanf("%d%d" , &row2 , &col2);
    printf("\n") ;
    fflush(stdout);
    printf("Please enter its elements: \n");
    fflush(stdout);

    //building the second matrix.
    allocate(&matrix2 , row2 , col2 );
    
     /// input 2nd matrix    
   for(unsigned int i = 0 ; i < row2 ; i++){
      for(unsigned int j = 0 ; j < col2 ; j++){
        scanf("%d" , &matrix2[i][j] );
      }
   }
}
 
 
 
int main(int argc, char* argv[])
{
    
    MPI_Status status;
    MPI_Init(&argc, &argv);
  
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
        
   
    if (rank == 0) { // master process
        
        printf("To read dimensions and values from file press 1\n") ;
        printf("To read dimensions and values from console press 2\n") ;
        
    
        unsigned int option ;
        scanf("%d" , &option);
        
        
        
        if(option == 1){    
           FileBehaviour() ;     /// read from file
           }
        
        else{   
            ConsoleBehaviour();  /// read from console
            }
        
        
        if(col != row2){
          printf("----We cant multiply as column of matrix1  and row of matrix2 are not equal----\n");
          exit(-1);
         } 

       int part_row = size - 1 ; /// take part of the total rows and send it to the available processes
                                 /// this logic is important in case of no sufficient processes for the total rows
                                 /// part of the rows will be computed at the available processes (cores) and the rest is computed by the master process
                             

       
       for (unsigned int i = 1 ; i < size ; i++){
       
            MPI_Send( &row , 1 , MPI_INT, i ,0, MPI_COMM_WORLD); 
            MPI_Send( &col , 1 , MPI_INT, i ,0, MPI_COMM_WORLD); 
            MPI_Send( &part_row , 1 , MPI_INT, i ,1, MPI_COMM_WORLD); 
        }

     
       //send matrix1 row by row to the available proccesses
        for (unsigned int i = 1; i < size ; i++) 
        {
            for (unsigned int j = 0; j < row ; j++)
            {
                MPI_Send(matrix[j] ,  col , MPI_INT, i, 4, MPI_COMM_WORLD);    
            }
        }

      
        for (unsigned int i = 1; i < size; i++)
        {
            MPI_Send( &row2 , 1 , MPI_INT, i ,1, MPI_COMM_WORLD);                           
            MPI_Send( &col2 , 1 , MPI_INT, i ,1, MPI_COMM_WORLD);  
        }

    
        ///send matrix2 row by row to the available proccesses
        for (unsigned int i = 1; i < size; i++) 
        {
            for (unsigned int j = 0; j < row2 ; j++)
            {
                MPI_Send(matrix2[j], col2 , MPI_INT, i, 2, MPI_COMM_WORLD);  
            
            }
        }
        

       /// build a result matrix at master process to recieve the computations from each row
        allocate(&resultMatrix , row , col2 );
        /// to prevent garbage
        reset_array(&resultMatrix , row , col2) ;
        //recieving the resultedMatrix
         for (unsigned int i = 1; i < size; i++){
          for (unsigned int  j = 0; j < row ; j++){   
            MPI_Recv(resultMatrix[j]  , col2 , MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);  
          }
        }
        
        /// in case we can not distribute all the rows to all cores and we have more rows than the core number
        /// the master process will continue the unsent rows and multiply
        if(size <= row || size <= row2){
              for(int i = size-1  ; i < row  ; i++){
                  for( int j = 0 ; j < col2 ;j++){
                     for(int z = 0 ; z < col; z++){
                       resultMatrix[i][j] +=  matrix[i][z] * matrix2[z][j] ;  
                     } 
                 } 
            }
       }
       
    
         ///successfully received and printing the answer       
         
         printf("\nResult Matrix is (%d x %d) :" , row , col2);
         printf("\n");
         for(int i = 0 ; i < row ; i++){
            for(int j = 0 ; j < col2 ; j++){
               printf("%d  " , resultMatrix[i][j]);
          }
          printf("\n");
        } 
   
    }
   
   /// slave processes
    else{
      int part_row ;
      MPI_Recv(&part_row, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      
      //now each other process receives the values of rows and columns of M1.
        MPI_Recv(&row, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&col, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);



      //now each other process receives the values of rows and columns of transM2.
        MPI_Recv(&row2, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&col2, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);


      
       
        //building 2d array to recieve matrix 1.
        allocate(&matrix , row , col );

        //reciving matrix1
        for (unsigned int  j = 0; j < row; j++){    
            MPI_Recv(matrix[j]  ,  col  , MPI_INT, 0, 4, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
 
        }
        

        //building matrix2
        allocate(&matrix2 , row2 , col2 );
        
        for (unsigned int  j = 0 ; j < row2; j++){    
            MPI_Recv(matrix2[j]  , col2, MPI_INT, 0, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
        
       
       
          //building resultMatrix to hold answers of the multiplication process .
          allocate(&resultMatrix , row , col2 );
            
          /// reset to avoid garabage
          reset_array(&resultMatrix , row , col2) ;
    
        
         /// multiply matrix1 with matrix2  
          for(int i = 0 ; i < part_row ;i++){
             for( int j = 0 ; j < col2 ;j++){
                for(int z = 0 ; z < col; z++){
                  resultMatrix[i][j] +=  matrix[i][z] * matrix2[z][j] ;  
                } 
             } 
          }
        
    
        
        //// send results to the master 
        for (unsigned int i = 1; i < size; i++){ 
             for (unsigned int j = 0; j < row ; j++){
                MPI_Send(resultMatrix[j], col2 , MPI_INT, 0, 0, MPI_COMM_WORLD); 
               }
            } 
         
       /// free the 2d arrays
         deallocate(matrix,row) ;
         deallocate(matrix2,row2) ;
         deallocate(resultMatrix,row) ;
         
    }
     
        
  
    // cleans up all MPI state before exit of process
    MPI_Finalize();
  
    return 0;
}
