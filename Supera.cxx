#include "Supera.h"

#if __has_include("larcv3/core/dataformat/Particle.h")
void init_Supera(pybind11::module m)
{
    // init_SuperaLorentz(m);
    init_superabase(m);
}
#endif

