
/**
 * \file Supera.h
 *
 * \ingroup Supera
 *
 * \brief Class def header for exception classes for larcv3 framework
 *
 * @author zhulcher
 */

/** \addtogroup Supera

    @{*/
#ifndef __LARCV3CORE_SUPERA_H__
#define __LARCV3CORE_SUPERA_H__

#include "SuperaLorentz.h"

#ifndef LARCV_NO_PYBIND
#ifdef LARCV_INTERNAL
#include <pybind11/pybind11.h>
__attribute__((visibility("default"))) void init_Supera(pybind11::module m);
#endif
// bindings
#endif

// include guards
#endif
