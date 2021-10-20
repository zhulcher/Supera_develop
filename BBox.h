/**
 * \file BBox.h
 *
 * \ingroup core_DataFormat
 *
 * \brief Class def header for a class larcv::BBox2D and larcv::BBox3D
 *
 * @author kazuhiro
 */

/** \addtogroup core_DataFormat
    @{*/
#ifndef __SUPERA_BBOX_H__
#define __SUPERA_BBOX_H__

#include <iostream>
#include <cmath>
//#include "DataFormatTypes.h"
namespace supera
{

    class Point3D {
  public:
    Point3D(double xv=0, double yv=0, double zv=0) : x(xv), y(yv), z(zv) {}
    ~Point3D() {}

    Point3D(const Point3D& pt) : x(pt.x), y(pt.y), z(pt.z) {}

    double x, y, z;
    
    inline bool operator== (const Point3D& rhs) const
    { return (x == rhs.x && y == rhs.y && z == rhs.z); }
    inline bool operator!= (const Point3D& rhs) const
    { return !(rhs == (*this)); }

    inline Point3D& operator/= (const double rhs)
    { x /= rhs; y /= rhs; z /= rhs; return (*this); }
    inline Point3D& operator*= (const double rhs)
    { x *= rhs; y *= rhs; z *= rhs; return (*this); }
    inline Point3D& operator+= (const Point3D& rhs)
    { x += rhs.x; y += rhs.y; z += rhs.z; return (*this); }
    inline Point3D& operator-= (const Point3D& rhs)
    { x -= rhs.x; y -= rhs.y; z -= rhs.z; return (*this); }

    inline Point3D operator/ (const double rhs) const
    { return Point3D(x/rhs,y/rhs,z/rhs); }
    inline Point3D operator* (const double rhs) const
    { return Point3D(x*rhs,y*rhs,z*rhs); }
    inline Point3D operator+ (const Point3D& rhs) const
    { return Point3D(x+rhs.x,y+rhs.y,z+rhs.z); }
    inline Point3D operator- (const Point3D& rhs) const
    { return Point3D(x-rhs.x,y-rhs.y,z-rhs.z); }

    inline double squared_distance(const Point3D& pt) const
    { return pow(x-pt.x,2)+pow(y-pt.y,2)+pow(z-pt.z,2); }
    inline double distance(const Point3D& pt) const
    { return sqrt(squared_distance(pt)); }
    inline Point3D direction(const Point3D& pt) const
    { Point3D res(pt.x - x, pt.y - y, pt.z - z); res /= distance(pt); return res; }

  };


    /**
       \class BBox3D
       \brief Bounding box in 3D
    */
    class BBox3D
    {

    public:
        /// Default constructor
        BBox3D(double xmin = 0, double ymin = 0, double zmin = 0,
               double xmax = 0, double ymax = 0, double zmax = 0);

        /// Default destructor
        virtual ~BBox3D() {}

        inline bool operator==(const BBox3D &rhs) const
        {
            return (_p1 == rhs._p1 && _p2 == rhs._p2);
        }

        void update(double xmin, double ymin, double zmin,
                    double xmax, double ymax, double zmax);

        void update(const Point3D &pt1, const Point3D &pt2);
        inline bool empty() const { return (_p1 == _p2); }
        inline const Point3D &origin() const { return _p1; }
        inline const Point3D &bottom_left() const { return _p1; }
        inline const Point3D &top_right() const { return _p2; }
        inline Point3D center() const { return Point3D(center_x(), center_y(), center_z()); }
        inline double center_x() const { return _p1.x + 0.5 * (_p2.x - _p1.x); }
        inline double center_y() const { return _p1.y + 0.5 * (_p2.y - _p1.y); }
        inline double center_z() const { return _p1.z + 0.5 * (_p2.z - _p1.z); }
        inline double min_x() const { return _p1.x; }
        inline double min_y() const { return _p1.y; }
        inline double min_z() const { return _p1.z; }
        inline double max_x() const { return _p2.x; }
        inline double max_y() const { return _p2.y; }
        inline double max_z() const { return _p2.z; }
        inline double width() const { return _p2.x - _p1.x; }
        inline double height() const { return _p2.y - _p1.y; }
        inline double depth() const { return _p2.z - _p1.z; }
        inline double volume() const { return (_p2.x - _p1.x) * (_p2.y - _p1.y) * (_p2.z - _p1.z); }
        double area(int axis) const;
        BBox3D overlap(const BBox3D &box) const;
        BBox3D inclusive(const BBox3D &box) const;
        bool contains(const Point3D &point) const;
        bool contains(double x, double y, double z) const;

        std::string dump() const;

    private:
        Point3D _p1; ///< bottom-left point coordinate (x1,y1,z1) where x1<x2 and y1<y2 and z1<z2
        Point3D _p2; ///< top-right point coordinate (x2,y2,z2) where x1<x2 and y1<y2 and z1<z2
    };
}
#endif
/** @} */ // end of doxygen group