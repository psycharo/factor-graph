#include <mvalgonquin.h>

#include <mathutil.h>

using namespace vmp;


void MVAlgonquin::updatePosterior()
{
    auto &precision = m_parameters.precision;
    auto &varsSpeech = m_parameters.varsSpeech;
    auto &varsNoise = m_parameters.varsNoise;
    auto &meansSpeech = m_parameters.meansSpeech;
    auto &meansNoise = m_parameters.meansNoise;
    auto &weights = m_parameters.weights;

    for (size_t iter = 0; iter < m_numIters; ++iter)
    for (size_t s = 0; s < numSpeech(); ++s)
    {
        for (size_t n = 0; n < numNoise(); ++n)
        {
            size_t i = index(s, n);

            const vec &meanS = meansSpeech[i];
            const vec &meanN = meansNoise[i];

            vec logSumValue = logSumV(meanS, meanN);
            pair<vec,vec> jacob = logSumVJacobian(meanS, meanN);

            // fi = ( var^-1 + [dg_s, dg_n]^T * psi^-1 * [dg_s, dg_n])
            varsSpeech[i] = 1.0 / (priorPrecSpeech(s) + meanS % precision % meanS);
            varsNoise[i] = 1.0 / (priorPrecNoise(n) + meanN % precision % meanN);
            // nu^* = psi * (covar^-1 * (mu - nu) + g'^T * psi^-1 * (y - g))
            meansSpeech[i] = meanS + varsSpeech[i] % (priorPrecSpeech(s) % (priorMeanSpeech(s) - meanS) +
                                                      jacob.first % precision % (m_value - logSumValue));
            meansNoise[i] = meanN + varsNoise[i] % (priorPrecNoise(n) % (priorMeanNoise(n) - meanN) +
                                                    jacob.second % precision % (m_value - logSumValue));
        } // for n
    } // for s

    for (size_t s = 0; s < numSpeech(); ++s)
    {
        for (size_t n = 0; n < numNoise(); ++n)
        {
            size_t i = index(s, n);

            const vec &meanS = meansSpeech[i];
            const vec &meanN = meansNoise[i];

            pair<vec,vec> jacob = logSumVJacobian(meanS, meanN);

            vec diffValue = (m_value - logSumV(meanS, meanN));
            vec diffS = (meanS - priorMeanSpeech(s));
            vec diffN = (meanN - priorMeanNoise(n));

            double means = sum(diffValue % diffValue % precision) +
                           sum(diffS % diffS % priorPrecSpeech(s)) +
                           sum(diffN % diffN % priorPrecNoise(n));

            vec tmpS = jacob.first % jacob.first % precision;
            vec tmpN = jacob.second % jacob.second % precision;

            double traces = sum(priorPrecSpeech(s) % varsSpeech[i]) +
                            sum(priorPrecNoise(n) % varsNoise[i]) +
                            sum(tmpS % varsSpeech[i] + tmpN % varsNoise[i]);

            double determs = log(prod(1.0 / priorPrecSpeech(s))) +
                             log(prod(1.0 / priorPrecNoise(n))) -
                             log(prod(varsSpeech[i] % varsNoise[i]));
            weights(s,n) = priorWeightSpeech(s) * priorWeightNoise(n) * exp(-0.5 * (means + traces + determs));
        }
    }
    weights /= accu(weights);
}




void MVAlgonquin::updateMoments()
{
    m_speech.fill(0);
    m_noise.fill(0);
    for (size_t s = 0; s < numSpeech(); ++s)
        for (size_t n = 0; n < numNoise(); ++n)
        {
            size_t i = index(s,n);
            m_speech += m_parameters.weights(s, n) * m_parameters.meansSpeech[i];
            m_noise += m_parameters.weights(s, n) * m_parameters.meansNoise[i];
        }
}