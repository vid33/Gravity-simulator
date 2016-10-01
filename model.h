#ifndef MODEL_H
#define MODEL_H

extern double centerOfMass(double fmass[], double fposition[][3], int fobject_no);

extern void zeroMomentum(double fvelocity[][3], double fmass[0], double fmassTot, int fobject_no); 

extern void  model(double fforce[][3], double facceleration[][3], double fposition[][3], double fvelocity[][3], double fmass[], double &ftime, double fdeltaTime, int fstepNo, int fobject_no);

#endif



