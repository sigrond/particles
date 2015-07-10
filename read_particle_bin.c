#include "mex.h"
#include<stdio.h>
#include<stdlib.h>

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    int buflen=mxGetM(prhs[0])*mxGetN(prhs[0])+1;
    printf("buflen: %d\n",buflen);
    char* name=malloc(buflen);
    mxGetString(prhs[0],name,buflen);
    printf("name: %s\n",name);
    FILE* f=fopen(name,"rb");
    fseek(f,0,SEEK_END);
    long int fileSize=ftell(f);
    fseek(f,0,SEEK_SET);
    printf("fsize(bytes): %d\n",fileSize);
    int numParticles;
    fread(&numParticles,sizeof(int),1,f);
    printf("numParticles: %d\n",numParticles);
    int numFrames=(fileSize-sizeof(int))/(numParticles*4*sizeof(float)+sizeof(double));
    printf("numFrames: %d\n",numFrames);
    //int dims[2]={numParticles*4,numFrames};
    const char* field_names[]={"time","poz"};
    mxArray* retArray=mxCreateStructMatrix(numFrames,1,2,field_names);
    double time;
    mxArray* poz;
    float* pozData;
    //int tmp;
    for(int i=0;i<numFrames;i++)
    {
        fread(&time,sizeof(double),1,f);
        //printf("time: %f\n",time);
        mxSetField(retArray,i,"time",mxCreateDoubleScalar(time));
        poz=mxCreateNumericMatrix(4,numParticles,mxSINGLE_CLASS,mxREAL);
        pozData=mxGetData(poz);
        /*tmp=*/fread(pozData,sizeof(float),numParticles*4,f);
        //printf("fread: %d\n",tmp);
        mxSetField(retArray,i,"poz",poz);
    }
    plhs[0]=retArray;
    fclose(f);
    free(name);
}
