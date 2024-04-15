/** \file read_particle_bin_range.c
 * \author Tomasz Jakubczyk
 * \brief to samo co read_particle_bin, ale odczytuje przedział punktów czasowych
 * - (EN)same as read_particle_bin, but reading a range of temporal points
 */

#include "mex.h"
#include<stdio.h>
#include<stdlib.h>

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	if(nrhs != 2)
	{
		printf("2 arguments must be provided: file name and frame number vector\n");
		return;
	}
    int buflen=mxGetM(prhs[0])*mxGetN(prhs[0])+1;
    //printf("buflen: %d\n",buflen);
    char* name=malloc(buflen);
    mxGetString(prhs[0],name,buflen);
    //printf("name: %s\n",name);
	double* rangem = mxGetPr(prhs[1]);
	int rangeSize = mxGetM(prhs[1])*mxGetN(prhs[1]);
	double* range=malloc(rangeSize*sizeof(double)+1);
	memcpy(range, rangem, rangeSize*sizeof(double));
	printf("rangeSize: %d\n",rangeSize);
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
    mxArray* retArray=mxCreateStructMatrix(rangeSize,1,2,field_names);
    double time;
    mxArray* poz;
    float* pozData;
    //int tmp;
	int frameNumber = 0;
	int k = 0;
    for(int i=0;i<numFrames;i++)
    {
        fread(&time,sizeof(double),1,f);
		//printf("range[%d]: %f\n",k,range[k]);
		if(k < rangeSize){
			frameNumber = (int)range[k];
			//printf("frameNumber: %d, i: %d\n",frameNumber, i);
			if(i == frameNumber - 1)
			{
				//printf("time: %f\n",time);
				mxSetField(retArray,k,"time",mxCreateDoubleScalar(time));
				poz=mxCreateNumericMatrix(4,numParticles,mxSINGLE_CLASS,mxREAL);
				pozData=mxGetData(poz);
				/*tmp=*/fread(pozData,sizeof(float),numParticles*4,f);
				//printf("fread: %d\n",tmp);
				mxSetField(retArray,k,"poz",poz);
				k++;
			} else {
				fseek(f,sizeof(float)*numParticles*4,SEEK_CUR);
			}
		} else {
			break;
		}
    }
    plhs[0]=retArray;
    fclose(f);
    free(name);
	free(range);
}
