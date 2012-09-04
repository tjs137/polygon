// Boost.Polygon library voronoi_events.hpp header file

//          Copyright Andrii Sydorchuk 2010-2012.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

// See http://www.boost.org for updates, documentation, and revision history.

#ifndef BOOST_POLYGON_VORONOI_EVENTS
#define BOOST_POLYGON_VORONOI_EVENTS

namespace boost {
namespace polygon {
// Warning: only the below two enums and routine should be used by users.
// Represents topology type of the voronoi site.
enum GeometryCategory {
  GEOMETRY_CATEGORY_POINT = 0x0,
  GEOMETRY_CATEGORY_SEGMENT = 0x1
};

// Represents category of the input source that forms Voronoi cell.
enum SourceCategory {
  // Point subtypes.
  SOURCE_CATEGORY_SINGLE_POINT = 0x0,
  SOURCE_CATEGORY_SEGMENT_START_POINT = 0x1,
  SOURCE_CATEGORY_SEGMENT_END_POINT = 0x2,

  // Segment subtypes.
  SOURCE_CATEGORY_INITIAL_SEGMENT = 0x8,
  SOURCE_CATEGORY_REVERSE_SEGMENT = 0x9,

  SOURCE_CATEGORY_GEOMETRY_SHIFT = 0x3,
  SOURCE_CATEGORY_BITMASK = 0x1F
};

bool belongs(
    const SourceCategory& source_category,
    const GeometryCategory& geometry_category) {
  return (static_cast<std::size_t>(source_category) >>
      SOURCE_CATEGORY_GEOMETRY_SHIFT) ==
      static_cast<std::size_t>(geometry_category);
}

// Cartesian 2D point data structure.
template <typename T>
class point_2d {
public:
  typedef T coordinate_type;

  point_2d() {}

  point_2d(coordinate_type x, coordinate_type y) :
      x_(x),
      y_(y) {}

  bool operator==(const point_2d& that) const {
    return (this->x_ == that.x()) && (this->y_ == that.y());
  }

  bool operator!=(const point_2d& that) const {
    return (this->x_ != that.x()) || (this->y_ != that.y());
  }

  coordinate_type x() const {
    return x_;
  }

  coordinate_type y() const {
    return y_;
  }

  point_2d& x(coordinate_type x) {
    x_ = x;
    return *this;
  }

  point_2d& y(coordinate_type y) {
    y_ = y;
    return *this;
  }

private:
  coordinate_type x_;
  coordinate_type y_;
};

// Site event type.
// Occurs when the sweepline sweeps over one of the initial sites:
//   1) point site
//   2) start-point of the segment site
//   3) endpoint of the segment site
// Implicit segment direction is defined: the start-point of
// the segment compares less than its endpoint.
// Each input segment is divided onto two site events:
//   1) One going from the start-point to the endpoint
//      (is_inverse() = false)
//   2) Another going from the endpoint to the start-point
//      (is_inverse() = true)
// In beach line data structure segment sites of the first
// type precede sites of the second type for the same segment.
// Members:
//   point0_ - point site or segment's start-point
//   point1_ - segment's endpoint if site is a segment
//   sorted_index_ - the last bit encodes information if the site is inverse;
//     the other bits encode site event index among the sorted site events
//   initial_index_ - site index among the initial input set
// Note: for all sites is_inverse_ flag is equal to false by default.
template <typename T>
class site_event {
public:
  typedef T coordinate_type;
  typedef point_2d<T> point_type;

  site_event() :
      point0_(0, 0),
      point1_(0, 0),
      sorted_index_(0),
      flags_(0) {}

  site_event(coordinate_type x, coordinate_type y) :
      point0_(x, y),
      point1_(x, y),
      sorted_index_(0),
      flags_(0) {}

  site_event(const point_type& point) :
      point0_(point),
      point1_(point),
      sorted_index_(0),
      flags_(0) {}

  site_event(coordinate_type x1, coordinate_type y1,
             coordinate_type x2, coordinate_type y2):
      point0_(x1, y1),
      point1_(x2, y2),
      sorted_index_(0),
      flags_(0) {}

  site_event(const point_type& point1, const point_type& point2) :
      point0_(point1),
      point1_(point2),
      sorted_index_(0),
      flags_(0) {}

  bool operator==(const site_event& that) const {
    return (this->point0_ == that.point0_) &&
           (this->point1_ == that.point1_);
  }

  bool operator!=(const site_event& that) const {
    return (this->point0_ != that.point0_) ||
           (this->point1_ != that.point1_);
  }

  coordinate_type x(bool oriented = false) const {
    return x0(oriented);
  }

  coordinate_type y(bool oriented = false) const {
    return y0(oriented);
  }

  coordinate_type x0(bool oriented = false) const {
    if (!oriented)
      return point0_.x();
    return is_inverse() ? point1_.x() : point0_.x();
  }

  coordinate_type y0(bool oriented = false) const {
    if (!oriented)
      return point0_.y();
    return is_inverse() ? point1_.y() : point0_.y();
  }

  coordinate_type x1(bool oriented = false) const {
    if (!oriented)
      return point1_.x();
    return is_inverse() ? point0_.x() : point1_.x();
  }

  coordinate_type y1(bool oriented = false) const {
    if (!oriented)
      return point1_.y();
    return is_inverse() ? point0_.y() : point1_.y();
  }

  const point_type& point0(bool oriented = false) const {
    if (!oriented)
      return point0_;
    return is_inverse() ? point1_ : point0_;
  }

  const point_type& point1(bool oriented = false) const {
    if (!oriented)
      return point1_;
    return is_inverse() ? point0_ : point1_;
  }

  std::size_t sorted_index() const {
    return sorted_index_;
  }

  site_event& sorted_index(std::size_t index) {
    sorted_index_ = index;
    return *this;
  }

  std::size_t initial_index() const {
    return initial_index_;
  }

  site_event& initial_index(std::size_t index) {
    initial_index_ = index;
    return *this;
  }

  bool is_inverse() const {
    return (flags_ & IS_INVERSE) ? true : false;
  }

  site_event& inverse() {
    flags_ ^= IS_INVERSE;
    return *this;
  }

  SourceCategory source_category() const {
    return static_cast<SourceCategory>(flags_ & SOURCE_CATEGORY_BITMASK);
  }

  site_event& source_category(SourceCategory source_category) {
    flags_ |= source_category;
    return *this;
  }

  bool is_point() const {
    return (point0_.x() == point1_.x()) && (point0_.y() == point1_.y());
  }

  bool is_segment() const {
    return (point0_.x() != point1_.x()) || (point0_.y() != point1_.y());
  }

private:
  enum Bits {
    IS_INVERSE = 0x20  // 32
  };

  point_type point0_;
  point_type point1_;
  std::size_t sorted_index_;
  std::size_t initial_index_;
  std::size_t flags_;
};

// Circle event type.
// Occurs when the sweepline sweeps over the rightmost point of the Voronoi
// circle (with the center at the intersection point of the bisectors).
// Circle event is made of the two consecutive nodes in the beach line data
// structure. In case another node was inserted during algorithm execution
// between the given two nodes circle event becomes inactive.
// Variables:
//   center_x_ - center x-coordinate;
//   center_y_ - center y-coordinate;
//   lower_x_ - leftmost x-coordinate;
//   is_active_ - states whether circle event is still active.
// NOTE: lower_y coordinate is always equal to center_y.
template <typename T>
class circle_event {
public:
  typedef T coordinate_type;

  circle_event() : is_active_(true) {}

  circle_event(coordinate_type c_x,
               coordinate_type c_y,
               coordinate_type lower_x) :
      center_x_(c_x),
      center_y_(c_y),
      lower_x_(lower_x),
      is_active_(true) {}

  coordinate_type x() const {
    return center_x_;
  }

  circle_event& x(coordinate_type center_x) {
    center_x_ = center_x;
    return *this;
  }

  coordinate_type y() const {
    return center_y_;
  }

  circle_event& y(coordinate_type center_y) {
    center_y_ = center_y;
    return *this;
  }

  coordinate_type lower_x() const {
    return lower_x_;
  }

  circle_event& lower_x(coordinate_type lower_x) {
    lower_x_ = lower_x;
    return *this;
  }

  coordinate_type lower_y() const {
    return center_y_;
  }

  bool is_active() const {
    return is_active_;
  }

  circle_event& deactivate() {
    is_active_ = false;
    return *this;
  }

private:
  coordinate_type center_x_;
  coordinate_type center_y_;
  coordinate_type lower_x_;
  bool is_active_;
};
}  // polygon
}  // boost

#endif  // BOOST_POLYGON_VORONOI_EVENTS