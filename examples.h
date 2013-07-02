#ifndef EXAMPLES_H
#define EXAMPLES_H

#include <util.h>
#include <vector>
#include <memory>

#include <variable.h>
#include <gaussian.h>
#include <mixture.h>


namespace vmp
{

const double DIRICHLET_PRIOR = 1;
const double GAMMA_PRIOR_SHAPE = 1e-3;
const double GAMMA_PRIOR_RATE = 1e-3;
const double GAUSS_PRIOR_MEAN = 0;
const double GAUSS_PRIOR_PREC = 1e-3;


// TODO: use tuple/real network instead
// TODO: at least parameterize using templates
struct MixtureNetwork
{
    MixtureNetwork():
        weightsPrior(NULL),
        weights(NULL),
        meanPrior(NULL), precPrior(NULL),
        means(NULL), precs(NULL),
        evidence(LB_INIT),
        iters(0)
    {}

    virtual ~MixtureNetwork()
    {
        delete weightsPrior;
        delete weights;
        delete meanPrior;
        delete precPrior;
        delete means;
        delete precs;
    }

    Parameters<UnivariateMixture> parameters() const
    {
        assert(means != NULL);
        Parameters<UnivariateMixture> result;
        result.components.resize(means->size());
        result.weights.resize(means->size());

        for (size_t m = 0; m < means->size(); ++m)
            result.components[m] = Parameters<Gaussian>(precs->moments(m).precision * means->moments(m).mean,
                                                        precs->moments(m).precision);
        result.weights = vector<double>(weights->moments().probs.begin(),
                                        weights->moments().probs.end());

        return result;
    }

    Dirichlet *weightsPrior;
    Discrete *weights;
    ConstGaussian *meanPrior;
    ConstGamma *precPrior;
    VariableArray<Gaussian> *means;
    VariableArray<Gamma> *precs;
    double evidence;
    size_t iters;
};

//! univariate mixture training
MixtureNetwork *trainMixture(const double *points, size_t numPoints, size_t numMixtures, size_t maxNumIters);






struct MVMixtureNetwork
{
    MVMixtureNetwork():
        weightsPrior(NULL), weights(NULL),
        meanPrior(NULL), precPrior(NULL),
        means(NULL), precs(NULL),
        evidence(LB_INIT), iters(0)
    {}

    virtual ~MVMixtureNetwork()
    {
        delete weightsPrior;
        delete weights;
        delete meanPrior;
        delete precPrior;
        delete means;
    }

    size_t numComps() const { return weights->dims(); }
    size_t dims() const { return meanPrior->dims(); }

    Dirichlet *weightsPrior;
    Discrete *weights;
    ConstMVGaussian *meanPrior;
    ConstWishart *precPrior;
    VariableArray<MVGaussian> *means;
    VariableArray<Wishart> *precs;
    double evidence;
    size_t iters;
};


// TODO: add some return type
void trainMVMixture(const mat &POINTS, size_t numMixtures, size_t maxNumIters,
                    mat &means, mat &sigmas, vec &weights);

MVMixtureNetwork *trainMVMixture(const mat &POINTS, size_t numMixtures, size_t maxNumIters);

// pure VB implementation
void trainMVMixtureVB(const mat &POINTS, size_t numMixtures, size_t maxNumIters,
                      mat &means, mat &sigmas, vec &weights);


// various tests
void testLogPDF();
void testMVMoG();
void testSpeechGMM(const vector<double> &bin);
void testLogSum();




} // namespace vmp

#endif // EXAMPLES_H
