// Manuel Stephan Homework 1, etch a sketch ... 
#include <stdio.h>

// global definitions 
const int MATDIM1 = 8; // y dimension
const int MATDIM2 = 8; // x dimension



int main(int argc, char **argv)
{
 int i,j,xpos=0,ypos=0,running=1;
 char c,dummy;
 int matrix[MATDIM1][MATDIM2];
// init the matrix
for( i=0; i< MATDIM1; i++)
 for( j=0; j< MATDIM2; j++)
 { 
  matrix[i][j]=0;
 } 

 while(running)
 {
	c = getchar(); 
        switch (c) {
        case 'f':
		if(xpos < (MATDIM2-1)) // do not write over borders of matrix 
	 	xpos++; 
		
	break;
	case 's': 
		if(xpos > 0) // do not write over borders of matrix 
	 	xpos--; 
        break;
	case 'e': 
		if(ypos > 0) // do not write over borders of matrix 
	 	ypos--; 
        break;
	case 'd': 
		if(ypos < (MATDIM1-1)) // do not write over borders of matrix 
	 	ypos++; 
        break;
	case 'q' :
	running = 0;
	break;	
	} 

	 matrix[ypos][xpos]=1;

	 for( i=0; i< MATDIM1; i++)
	 for( j=0; j< MATDIM2; j++)
	 { 
	  printf("%i  ",matrix[i][j]);
	  if(j==(MATDIM2-1))
	  printf("\n");
	 } 
 dummy = getchar(); // hack to avoid /n in the buffer :)
 }
  
  

  return 0;
}

