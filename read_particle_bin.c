/** \file read_particle_bin.c
 * \author Tomasz Jakubczyk
 * \brief .mex, który czyta plik .bin zawierający położenia wszystkich cząstek zapisywane z zadanym krokiem
 * i zwraca do Matlaba strukturę zawierającą kolumny "time" i "pos". Kolumna "pos" zawiera pozycje (X,Y,Z,1) wszystkich cząstek,
 * czyli każde pole "pos" ma rozmiar 4xliczba cząstek (4ty wymiar wynika z przechowywania pozycji w GPU naturalnego
 * ze względu na obroty w GL
 * - (EN) .mex, which reads a .bin file containing the positions of all particles recorded with a preset step,
 * and returns to Matlab a structure containing the "time" and "pos" columns. The "pos" column contains
 * the positions (X,Y,Z,1) of all particles, i.e. each "pos" field has a size of 4x the number of particles
 * (the 4th dimension is due to the storage of positions in the GPU natural for rotations with the GL 
 */

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
