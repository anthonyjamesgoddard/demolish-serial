#include "dynamics.h"
#include <vector>
#include "math.h"

/* vectorizable exponential map */
void expmap (iREAL Omega1, iREAL Omega2, iREAL Omega3,
                iREAL &Lambda1, iREAL &Lambda2, iREAL &Lambda3,
			          iREAL &Lambda4, iREAL &Lambda5, iREAL &Lambda6,
			          iREAL &Lambda7, iREAL &Lambda8, iREAL &Lambda9)
{
  iREAL angsq, sx, cx, v0, v1, v2, v01, v02, v12, s0, s1, s2;

  v0 = Omega1 * Omega1;
  v1 = Omega2 * Omega2;
  v2 = Omega3 * Omega3;

  angsq = v0 + v1 + v2;

  if (angsq < 3.0461741978671E-02) // use Taylor expansion if |Omega| < 10 deg
  {
    sx = 1.0 +
    (-1.666666666666667E-1 +
    (8.333333333333333E-3 +
    (-1.984126984126984E-4 +
    (2.755731922398589E-6 +
    (-2.505210838544172E-8 +
     1.605904383682161E-10 * angsq
    )*angsq
    )*angsq
    )*angsq
    )*angsq
    )*angsq;
  cx = 0.5 +
    (-4.166666666666667E-2 +
    (1.388888888888889E-3 +
    (-2.480158730158730E-5 +
    (2.755731922398589E-7 +
    (-2.087675698786810E-9 +
     1.147074559772972E-11 * angsq
    )*angsq
    )*angsq
    )*angsq
    )*angsq
    )*angsq;
  }
  else
  {
    iREAL t, s, c;
    t = angsq;
    angsq = sqrt (angsq);
    s = sin (angsq);
    c = cos (angsq);
    sx = s / angsq;
    cx = (1.0 - c) / t;
  }

  v01 = Omega1 * Omega2;
  v02 = Omega1 * Omega3;
  v12 = Omega2 * Omega3;
  s0 = sx * Omega1;
  s1 = sx * Omega2;
  s2 = sx * Omega3;

  Lambda1 = -cx*(v2+v1);
  Lambda2 = cx*v01;
  Lambda3 = cx*v02;
  Lambda4 = Lambda2;
  Lambda5 = -cx*(v2+v0);
  Lambda6 = cx*v12;
  Lambda7 = Lambda3;
  Lambda8 = Lambda6;
  Lambda9 = -cx*(v1+v0);
  Lambda1 += 1.0;
  Lambda2 += s2;
  Lambda3 -= s1;
  Lambda4 -= s2;
  Lambda5 += 1.0;
  Lambda6 += s0;
  Lambda7 += s1;
  Lambda8 -= s0;
  Lambda9 += 1.0;
}

 void demolish::dynamics::updateRotationMatrix(
     iREAL *angular,
     iREAL *refAngular,
     iREAL *rotation,
     iREAL step)
{
  iREAL DL[9], rot0[9];
  expmap (step*refAngular[0], step*refAngular[1], step*refAngular[2], DL[0], DL[1], DL[2], DL[3], DL[4], DL[5], DL[6], DL[7], DL[8]);

  rot0[0] = rotation[0];
  rot0[1] = rotation[1];
  rot0[2] = rotation[2];
  rot0[3] = rotation[3];
  rot0[4] = rotation[4];
  rot0[5] = rotation[5];
  rot0[6] = rotation[6];
  rot0[7] = rotation[7];
  rot0[8] = rotation[8];

  //NNMUL (rot0, DL, rotation);
  rotation[0] = rot0[0]*DL[0]+rot0[3]*DL[1]+rot0[6]*DL[2];
  rotation[1] = rot0[1]*DL[0]+rot0[4]*DL[1]+rot0[7]*DL[2];
  rotation[2] = rot0[2]*DL[0]+rot0[5]*DL[1]+rot0[8]*DL[2];
  rotation[3] = rot0[0]*DL[3]+rot0[3]*DL[4]+rot0[6]*DL[5];
  rotation[4] = rot0[1]*DL[3]+rot0[4]*DL[4]+rot0[7]*DL[5];
  rotation[5] = rot0[2]*DL[3]+rot0[5]*DL[4]+rot0[8]*DL[5];
  rotation[6] = rot0[0]*DL[6]+rot0[3]*DL[7]+rot0[6]*DL[8];
  rotation[7] = rot0[1]*DL[6]+rot0[4]*DL[7]+rot0[7]*DL[8];
  rotation[8] = rot0[2]*DL[6]+rot0[5]*DL[7]+rot0[8]*DL[8];

  //NVMUL (rotation, refAngular, angular);
  angular[0] = rotation[0]*refAngular[0]+rotation[3]*refAngular[1]+rotation[6]*refAngular[2];
  angular[1] = rotation[1]*refAngular[0]+rotation[4]*refAngular[1]+rotation[7]*refAngular[2];
  angular[2] = rotation[2]*refAngular[0]+rotation[5]*refAngular[1]+rotation[8]*refAngular[2];
}

void demolish::dynamics::updateAngular(
    iREAL *refAngular,
    iREAL *rotation,
    iREAL *inertia,
    iREAL *inverse,
    iREAL *torque,
    iREAL step)
{
	iREAL half = 0.5*step;

	iREAL T[3];
	iREAL DL[9];
	iREAL A[3];
	iREAL B[3];

	////EQUATION (13) START
	///////////////////////
	T[0] = rotation[0]*torque[0]+rotation[1]*torque[1]+rotation[2]*torque[2];
	T[1] = rotation[3]*torque[0]+rotation[4]*torque[1]+rotation[5]*torque[2];
	T[2] = rotation[6]*torque[0]+rotation[7]*torque[1]+rotation[8]*torque[2];
	////EQUATION (13) END
	/////////////////////

	////EQUATION (14) START
	///////////////////////
	expmap (-half*refAngular[0], -half*refAngular[1], -half*refAngular[2], DL[0], DL[1], DL[2], DL[3], DL[4], DL[5], DL[6], DL[7], DL[8]);

	//NVMUL (inertia, refAngular, A);
	A[0] = inertia[0]*refAngular[0]+inertia[3]*refAngular[1]+inertia[6]*refAngular[2];
	A[1] = inertia[1]*refAngular[0]+inertia[4]*refAngular[1]+inertia[7]*refAngular[2];
	A[2] = inertia[2]*refAngular[0]+inertia[5]*refAngular[1]+inertia[8]*refAngular[2];

	//NVMUL (DL, A, B);
	B[0] = DL[0]*A[0]+DL[3]*A[1]+DL[6]*A[2];
	B[1] = DL[1]*A[0]+DL[4]*A[1]+DL[7]*A[2];
	B[2] = DL[2]*A[0]+DL[5]*A[1]+DL[8]*A[2];

	//ADDMUL (B, half, T, B);
	B[0] = B[0] + half*T[0];
	B[1] = B[1] + half*T[1];
	B[2] = B[2] + half*T[2];

	//NVMUL (inverse, B, A); // O(t+h/2)
	A[0] = inverse[0]*B[0]+inverse[3]*B[1]+inverse[6]*B[2];
	A[1] = inverse[1]*B[0]+inverse[4]*B[1]+inverse[7]*B[2];
	A[2] = inverse[2]*B[0]+inverse[5]*B[1]+inverse[8]*B[2];

	////EQUATION (14) END
	/////////////////////

	////EQUATION (15) START
	///////////////////////
	//NVMUL (inertia, A, B);
	B[0] = inertia[0]*A[0]+inertia[3]*A[1]+inertia[6]*A[2];
	B[1] = inertia[1]*A[0]+inertia[4]*A[1]+inertia[7]*A[2];
	B[2] = inertia[2]*A[0]+inertia[5]*A[1]+inertia[8]*A[2];

	//PRODUCTSUB (A, B, T); // T - O(t+h/2) x J O(t+h/2)
	T[0] -= A[1]*B[2] - A[2]*B[1];
	T[1] -= A[2]*B[0] - A[0]*B[2];
	T[2] -= A[0]*B[1] - A[1]*B[0];

	//SCALE (T, step);
	T[0] *= step;
	T[1] *= step;
	T[2] *= step;

	//NVADDMUL (refAngular, inverse, T, refAngular); // O(t+h)
	refAngular[0] = refAngular[0] + inverse[0]*T[0]+inverse[3]*T[1]+inverse[6]*T[2];
	refAngular[1] = refAngular[1] + inverse[1]*T[0]+inverse[4]*T[1]+inverse[7]*T[2];
	refAngular[2] = refAngular[2] + inverse[2]*T[0]+inverse[5]*T[1]+inverse[8]*T[2];
	////EQUATION (15) END
	/////////////////////
}

void demolish::dynamics::updateVertices(
    iREAL *x,
    iREAL *y,
    iREAL *z,
    iREAL *refx,
    iREAL *refy,
    iREAL *refz,
    iREAL *rotation,
    iREAL *position,
    iREAL *refposition)
{
	iREAL C[3], c[3];

	//point A REFERENCIAL
	C[0] = *refx - refposition[0];
	C[1] = *refy - refposition[1];
	C[2] = *refz - refposition[2];

	//SCC (location, C);
	c[0] = position[0] + (rotation[0]*(C)[0]+rotation[3]*C[1]+rotation[6]*C[2]);
	c[1] = position[1] + (rotation[1]*(C)[0]+rotation[4]*C[1]+rotation[7]*C[2]);
	c[2] = position[2] + (rotation[2]*(C)[0]+rotation[5]*C[1]+rotation[8]*C[2]);

	//point A SPATIAL
	*x = c[0];
	*y = c[1];
	*z = c[2];
}
