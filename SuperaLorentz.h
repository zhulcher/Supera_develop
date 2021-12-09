#ifndef SuperaLorentz_h_seen
#define SuperaLorentz_h_seen

// adapted from TLorentzvector
// Author: Pasha Murat , Peter Malzacher  12/02/99

//#include <math.h>
//#include <vector>
#include "geometry.h"

#if __has_include("larcv3/core/dataformat/Particle.h")
#define larcv larcv3
#endif
namespace larcv
{
    class SLorentzvector
    {
    private:
        larcv::Vec3d fP; // 3 vector component
        double_t fE;     // time or energy of (x,y,z,t) or (px,py,pz,e)
    public:
        typedef double_t Scalar;

        //enum
        //{
        //    kX = 0,
        //    kY = 1,
        //    kZ = 2,
        //    kT = 3,
        //    kNUM_COORDINATES = 4,
        //    kSIZE = kNUM_COORDINATES
        //};
        // Safe indexing of the coordinates when using with matrices, arrays, etc.

        SLorentzvector();

        SLorentzvector(double_t x, double_t y, double_t z, double_t t);
        // Constructor giving the components x, y, z, t.

        //SLorentzvector(const double_t *carray);
        //SLorentzvector(const float_t *carray);
        // Constructor from an array, not checked!

        //SLorentzvector(std::vector<double_t> &vector3, double_t t);
        // Constructor giving a 3-Vector and a time component.

        //SLorentzvector(const SLorentzvector &lorentzvector);
        // Copy constructor.

        virtual ~SLorentzvector(){};
        // Destructor

        // inline operator TVector3 () const;
        // inline operator TVector3 & ();
        // Conversion (cast) to TVector3.

        inline double_t X() const;
        inline double_t Y() const;
        inline double_t Z() const;
        inline double_t T() const;
        // Get position and time.

        // Set position and time.

        inline double_t Px() const;
        inline double_t Py() const;
        inline double_t Pz() const;
        inline double_t P() const;
        inline double_t E() const;
        //inline double_t Energy() const;
        // Get momentum and energy.

        inline larcv::Vec3d Vect() const;
        // Get spatial component.

        //inline void SetVect(larcv::Vec3d &vect3);
        inline void SetVect(larcv::Vec3d p);
        // Set spatial component.

        //double_t operator()(int i) const;
        //inline double_t operator[](int i) const;
        // Get components by index.

        //double_t &operator()(int i);
        //inline double_t &operator[](int i);
        // Set components by index.

        //inline SLorentzvector &operator=(const SLorentzvector &);
        // Assignment.

        inline SLorentzvector operator+(const SLorentzvector &) const;
        inline SLorentzvector &operator+=(const SLorentzvector &);
        // Additions.

        inline SLorentzvector operator-(const SLorentzvector &) const;
        inline SLorentzvector &operator-=(const SLorentzvector &);
        // Subtractions.

        //inline SLorentzvector operator-() const;
        // Unary minus.

        inline SLorentzvector operator*(double_t a) const;
        inline SLorentzvector &operator*=(double_t a);
        // Scaling with real numbers.

        inline bool operator==(const SLorentzvector &) const;
        inline bool operator!=(const SLorentzvector &) const;
        // Comparisons.


        inline double_t Mag2() const;
        inline double_t Mag() const;
    };

    // inline SLorentzvector operator * (const SLorentzvector &, double_t a);
    //  moved to SLorentzvector::operator * (double_t a)
    //inline SLorentzvector operator*(double_t a, const SLorentzvector &);
    // Scaling LorentzVector with a real number

    inline double_t SLorentzvector::X() const { return fP.x; }
    inline double_t SLorentzvector::Y() const { return fP.y; }
    inline double_t SLorentzvector::Z() const { return fP.z; }
    inline double_t SLorentzvector::T() const { return fE; }

    inline double_t SLorentzvector::Px() const { return X(); }
    inline double_t SLorentzvector::Py() const { return Y(); }
    inline double_t SLorentzvector::Pz() const { return Z(); }
    inline double_t SLorentzvector::P() const { return sqrt(fP.x * fP.x + fP.y * fP.y + fP.z * fP.z); }
    inline double_t SLorentzvector::E() const { return T(); }
    //inline double_t SLorentzvector::Energy() const { return T(); }

    inline larcv::Vec3d SLorentzvector::Vect() const { return fP; }

    //inline void SLorentzvector::SetVect(larcv::Vec3d &p) { fP = p; }
    inline void SLorentzvector::SetVect(larcv::Vec3d p) { fP = p; }

    //inline double_t &SLorentzvector::operator[](int i) { return (*this)(i); }
    //inline double_t SLorentzvector::operator[](int i) const { return (*this)(i); }

    //inline SLorentzvector &SLorentzvector::operator=(const SLorentzvector &q)
    //{
    //    fP = q.Vect();
    //    fE = q.T();
    //    return *this;
   // }

    inline SLorentzvector SLorentzvector::operator+(const SLorentzvector &q) const
    {
        return SLorentzvector(fP.x + q.Vect().x, fP.y + q.Vect().y, fP.z + q.Vect().z, fE + q.T());
    }

    inline SLorentzvector &SLorentzvector::operator+=(const SLorentzvector &q)
    {
        fP = fP + q.Vect();
        fE += q.T();
        return *this;
    }

    inline SLorentzvector SLorentzvector::operator-(const SLorentzvector &q) const
    {
        return SLorentzvector(fP.x - q.Vect().x, fP.y - q.Vect().y, fP.z - q.Vect().z, fE - q.T());
    }

    inline SLorentzvector &SLorentzvector::operator-=(const SLorentzvector &q)
    {
        fP = fP - q.Vect();
        fE -= q.T();
        return *this;
    }

    //inline SLorentzvector SLorentzvector::operator-() const
    //{
    //    return SLorentzvector(-X(), -Y(), -Z(), -T());
    //}

    inline SLorentzvector &SLorentzvector::operator*=(double_t a)
    {
        fP = fP * a;
        fE *= a;
        return *this;
    }
    inline SLorentzvector SLorentzvector::operator*(double_t a) const
    {
        return SLorentzvector(a * X(), a * Y(), a * Z(), a * T());
    }
    inline bool SLorentzvector::operator==(const SLorentzvector &q) const
    {
        return (X() == q.X() && Y() == q.Y() && Z() == q.Z() && T() == q.T());
    }

    inline bool SLorentzvector::operator!=(const SLorentzvector &q) const
    {
        return (X() != q.X() || Y() != q.Y() || Z() != q.Z() || T() != q.T());
    }

    inline double_t SLorentzvector::Mag2() const
    {
        return T() * T() - (fP.x * fP.x + fP.y * fP.y + fP.z * fP.z);
    }

    inline double_t SLorentzvector::Mag() const
    {
        double_t mm = Mag2();
        return mm < 0.0 ? -sqrt(-mm) : sqrt(mm);
    }

    //inline SLorentzvector operator*(double_t a, const SLorentzvector &p)
    //{
    //    return SLorentzvector(a * p.X(), a * p.Y(), a * p.Z(), a * p.T());
    //}

    inline SLorentzvector::SLorentzvector()
        : fP(), fE(0.0) {}

     inline SLorentzvector::SLorentzvector(double_t x, double_t y, double_t z, double_t t)
         : fP(x, y, z), fE(t) {}

    // inline SLorentzvector::SLorentzvector(const double_t *x0)
    //     : fP(x0), fE(x0[3]) {}

    // inline SLorentzvector::SLorentzvector(const float_t *x0)
    //     : fP(x0), fE(x0[3]) {}

    // inline SLorentzvector::SLorentzvector(std::vector<double_t> &p, double_t e)
    //     : fP(p), fE(e) {}

    //inline double_t SLorentzvector::operator()(int i) const
    //{
    //    // dereferencing operator const
    //    switch (i)
    //    {
    //    case kX:
    //        return fP.x;
    //    case kY:
    //        return fP.y;
    //    case kZ:
    //       return fP.z;
     //   case kT:
    //        return fE;
    //    default:
    //        throw 20;
    //    }
    //    return 0.;
    //}
}

#ifdef LARCV_INTERNAL
#include <pybind11/pybind11.h>
void init_SuperaLorentz(pybind11::module m);
#endif
// bindings
#endif

