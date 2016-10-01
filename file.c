#include "baseinclude.h"
#include <g++/iostream.h>
#include <stdlib.h>
#include <math.h>
#include <g++/fstream.h>

extern char *inputFile;
int object_no;

void getObjectNo(Widget parent, char *clientdata, XtPointer calldata)
{
  /*Local definitions:*/

  const int length = 100;
  char buffer[length];
 
  ifstream inputfile(inputFile); 
  
   /*go to the beginning of file*/ 
  inputfile.seekg(0);

  inputfile.getline(buffer, length, '\n');
  inputfile.getline(buffer, length, '\n');
  object_no = atoi(buffer);
  inputfile.close();
}

void getAllVariables(double fmass[], double fposition[][3], double fvelocity[][3], int fobject_no, char *ffilename) {
  //local definitions:
  const int length=100;
  char buffer[length];
 
  ifstream inputfile(ffilename); 
  inputfile.seekg(0); //go to the beginning of file
  
  //read the needed values from the file:
  
  //mass:
  inputfile.getline(buffer, length, '\n');
  inputfile.getline(buffer, length, '\n');
    for (int i=0; i < fobject_no; i++) {
      inputfile.getline(buffer, length, '\n');
      inputfile.getline(buffer, length, '\n');
      fmass[i]=atof(buffer);
    }
  //position:
   inputfile.getline(buffer, length, '\n');
   for (int i=0; i < fobject_no; i++) {
     for (int j=0; j<3; j++) {
       inputfile.getline(buffer, length, '\n');
       fposition[i][j] = atof(buffer);
     }
     inputfile.getline(buffer, length, '\n');
   }
  //velocity:
  for (int i=0; i < fobject_no; i++) {
     for (int j=0; j<3; j++) {
       inputfile.getline(buffer, length, '\n');
       fvelocity[i][j] = atof(buffer);
     }
     inputfile.getline(buffer, length, '\n');
   }	
inputfile.close(); 
return;    
}





