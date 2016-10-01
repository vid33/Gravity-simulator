#include "baseinclude.h"
#include <g++/iostream.h>
#include <stdlib.h>
#include <math.h>
#include <g++/fstream.h>

#define G  6.672e-11

double centerOfMass(double fmass[], double fposition[][3], int fobject_no) {
 //Definitions:
  double massTot=0;
  double massWeigh[3] = {0, 0, 0};
  double centerOfMass[3];
  //Finds the total mass:
  for (int i=0; i<fobject_no; i++) {
     (massTot)=((massTot)+(fmass[i]));
  }
  //Finds the 'weighted' mass in all directions;
  for (int i=0; i<fobject_no; i++) {
    for (int j=0; j<3; j++) {
      (massWeigh[j])=(massWeigh[j])+((fposition[i][j])*(fmass[i]));
    }
  }
  //Finds the centre of mass in user coordinate system:
  for (int j=0; j<3; j++) {
    (centerOfMass[j])=((massWeigh[j])/(massTot));
  }
  //Moves every object to c.o.m. coordinate system:
  for (int i=0; i<fobject_no; i++) {
    for (int j=0; j<3; j++) {
      (fposition[i][j])=(fposition[i][j])-(centerOfMass[j]);
    }
  }
  return massTot;
}

void zeroMomentum(double fvelocity[][3], double fmass[0], double fmassTot, int fobject_no) {
//Definitions:
  double momentum[fobject_no][3];
  double momentumTot[3]={0, 0, 0};
  double comVelocity[3];
  //Finds the momentum of each object:
  for (int i=0; i<fobject_no; i++) { 
    for (int j=0; j<3; j++) {
      (momentum[i][j])=(fvelocity[i][j])*(fmass[i]);
    }
  }
  //Finds the total momentum of the system;
  for (int i=0; i<fobject_no; i++) {
    for (int j=0; j<3; j++) {
      (momentumTot[j])=(momentumTot[j])+(momentum[i][j]);
    }
  }
  //Finds the velocity of the center of mass
  for (int j=0; j<3; j++) {
    comVelocity[j]=(momentumTot[j])/(fmassTot);
  }
  //Sets the c.o.m. velocity to 0
  for (int i=0; i<fobject_no; i++) {
    for (int j=0; j<3; j++) {
      fvelocity[i][j]=(fvelocity[i][j])-(comVelocity[j]);
    }
  }
  return;
}
  
void  model(double fforce[][3], double facceleration[][3], double fposition[][3], double fvelocity[][3], double fmass[], double &ftime, double fdeltaTime, int fstepNo, int fobject_no) {
  //declarations of local variables...:
  double forceNorm;
  double radius, radiusSquared, radiusSquared1, radiusSquared2, radiusSquared3;
  double directionX, directionY, directionZ;
  double forceX, forceY, forceZ;
  cout.precision(10); //sets cout precision to ten decimals
  /* vidfile.precision(5); //sets vidfile precision to five decimals */
			  /*cout.setf(ios::scientific, ios::floatfield);*/ // scientific not. in cout
/*vidfile.setf(ios::scientific, ios::floatfield);*/ // scientific not. in file 
  //THE Loop
    for (int j=0; j<fobject_no; j++) {
      fforce[j][0]=0;
      fforce[j][1]=0;
      fforce[j][2]=0;
      for (int k=0; k<fobject_no; k++) {
        directionX=((fposition[j][0])-(fposition[k][0]));
	directionY=((fposition[j][1])-(fposition[k][1]));
	directionZ=((fposition[j][2])-(fposition[k][2]));
	radiusSquared1=pow(directionX, 2);
	radiusSquared2=pow(directionY, 2);
	radiusSquared3=pow(directionZ, 2);
	radiusSquared=radiusSquared1 + radiusSquared2 + radiusSquared3;
	radius=sqrt(radiusSquared1 + radiusSquared2 + radiusSquared3);
	if (radiusSquared > 0) { //to avoid same object
	    forceNorm=(G * fmass[j] * fmass[k] )/(radiusSquared);
	    //cout << forceNorm << "\n";
	    forceX = (directionX/radius) * forceNorm; 
	    forceY = (directionY/radius) * forceNorm; 
	    forceZ = (directionZ/radius) * forceNorm;
	    fforce[j][0] = fforce[j][0] + forceX;
	    fforce[j][1] = fforce[j][1] + forceY;
	    fforce[j][2] = fforce[j][2] + forceZ;
	  }  
      }
      /*-1 is for correct direction*/
      facceleration[j][0]=((fforce[j][0])/fmass[j])*(-1);
      facceleration[j][1]=((fforce[j][1])/fmass[j])*(-1);
      facceleration[j][2]=((fforce[j][2])/fmass[j])*(-1);
    }
    /*vidfile << "\n";*/
    for (int i=0; i < fobject_no; i++){
      fposition[i][0] = fposition[i][0] + facceleration[i][0]*(fdeltaTime/2) + fvelocity[i][0]*fdeltaTime;
      fposition[i][1] = fposition[i][1] + facceleration[i][1]*(fdeltaTime/2) + fvelocity[i][1]*fdeltaTime;
      fposition[i][2] = fposition[i][2] + facceleration[i][2]*(fdeltaTime/2) + fvelocity[i][2]*fdeltaTime; 
      fvelocity[i][0] = fvelocity[i][0] + facceleration[i][0]*fdeltaTime;
      fvelocity[i][1] = fvelocity[i][1] + facceleration[i][1]*fdeltaTime; 
      fvelocity[i][2] = fvelocity[i][2] + facceleration[i][2]*fdeltaTime;
    }
return;
}










