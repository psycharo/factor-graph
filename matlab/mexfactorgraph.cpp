#include <mex.h>

#include <string>
#include <iostream>
using namespace std;

#include "../algonquin.h"
#include "convert.h"




const size_t FUNCTION_IDX = 0;
const size_t TYPE_IDX = 1;
const size_t POINTER_IDX = 2;



void processNetwork(const std::string &functionName,
                    int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{

    if (functionName == "create")
    {
        plhs[0] = ptrToMxArray(new Network());
        return;
    }

    Network *nwk = mxArrayToPtr<Network>(prhs[POINTER_IDX]);
    if (functionName == "train")
    {
        size_t maxNumIters = mxArrayTo<int>(prhs[POINTER_IDX+5]);

        double *speech;
        size_t rows_s, cols_s;
        mxArrayToDoubleArray(prhs[POINTER_IDX+1], speech, rows_s, cols_s);
        size_t numSpeechComps = mxArrayTo<int>(prhs[POINTER_IDX+2]);
        nwk->trainSpeech(speech, cols_s, numSpeechComps, maxNumIters);

        double *noise;
        size_t rows_n, cols_n;
        mxArrayToDoubleArray(prhs[POINTER_IDX+3], noise, rows_n, cols_n);
        size_t numNoiseComps = mxArrayTo<int>(prhs[POINTER_IDX+4]);

        // training mixtures
        nwk->trainNoise(noise, cols_n, numNoiseComps, maxNumIters);

        plhs[0] = toMxStruct(nwk->speechDistr());
        plhs[1] = toMxStruct(nwk->noiseDistr());
    }
    else if (functionName == "process")
    {
        pair<double,double> result = nwk->process(mxArrayTo<double>(prhs[POINTER_IDX+1]));
        plhs[0] = toMxArray(result.first);
        plhs[1] = toMxArray(result.second);
    }
    else if (functionName == "setDistrs")
    {
        cout << "not supported" << endl;
//        nwk->setDistributions(mxStructTo<MoG>(prhs[POINTER_IDX+1]),
//                              mxStructTo<MoG>(prhs[POINTER_IDX+2]));
    }
    else if (functionName == "distrs")
    {
        if (nwk->trained())
        {
            plhs[0] = toMxStruct(nwk->speechDistr());
            plhs[1] = toMxStruct(nwk->noiseDistr());
        }
    }
    else if (functionName == "delete")
        delete nwk;

}




void processGMM(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    double *data = NULL;
    size_t rows;
    size_t cols;
    mxArrayToDoubleArray(prhs[POINTER_IDX], data, rows, cols);


    const size_t numPoints = cols;
    const size_t maxNumIters = 150;
    const size_t numMixtures = 6;

    Parameters<MoG> params;
    double evidence;
    size_t iters;

    params =  trainGMM(data,
                       numPoints,
                       maxNumIters,
                       numMixtures,
                       1, GaussianParameters(0, 1e-3), GammaParameters(1e-3, 1e-3),
                       evidence,
                       iters);

    double means[numMixtures], precs[numMixtures];
    for (size_t m = 0; m < params.dims(); ++m)
    {
        means[m] = params.components[m].mean();
        precs[m] = params.components[m].precision;
    }
    plhs[0] = toMxArray(means, 1, numMixtures);
    plhs[1] = toMxArray(precs, 1, numMixtures);
    plhs[2] = toMxArray(params.weights.data(), 1, numMixtures);
}




// the entry point into the function
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    if (nrhs < 1)
    {
        mexErrMsgTxt("Not enough arguments");
        return;
    }

    try
    {

        string functionName(mxArrayToString(prhs[FUNCTION_IDX]));
        string typeName(mxArrayToString(prhs[TYPE_IDX]));

        // here the only possible typeName is Network
        if (typeName == "Network")
            processNetwork(functionName, nlhs, plhs, nrhs, prhs);
        else if (typeName == "GMM")
            processGMM(nlhs, plhs, nrhs, prhs);
        else
            mexErrMsgTxt("Unsupported operation\n");
    }
    catch (...)
    {
        mexErrMsgTxt("Unknown error occured\n");
    }


}
