#include "mex.h"
#include<stdio.h>
#include<stdlib.h>

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    int buflen=mxGetM(prhs[0])+1;
    char* name=malloc(buflen);
    mxGetString(prhs[0],name,buflen);
    FILE* f=fopen(name,"rb");
    fseek(f,0,SEEK_END);
    long int fileSize=ftell(f);
    fseek(f,0,SEEK_SET);
    int numParticles;
    fread(&numParticles,sizeof(int),1,f);
    int numFrames=(fileSize-sizeof(int))/(numParticles*4*sizeof(float)+sizeof(long double));
    int dims[3]={wid, len, 3};
}
