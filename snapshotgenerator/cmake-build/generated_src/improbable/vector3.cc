// Generated by SpatialOS codegen. DO NOT EDIT!
// source: improbable/vector3.schema
#include "improbable/vector3.h"

namespace improbable {

// Serialization helpers.
//----------------------------------------------------------------

namespace detail {

void Write_Vector3d(
    const ::improbable::Vector3d& data, ::worker::detail::internal::Pbio_Object* object) {
  ::worker::detail::internal::Pbio_AddDouble(object, 1, data.x());
  ::worker::detail::internal::Pbio_AddDouble(object, 2, data.y());
  ::worker::detail::internal::Pbio_AddDouble(object, 3, data.z());
}

::improbable::Vector3d Read_Vector3d(::worker::detail::internal::Pbio_Object* object) {
  auto _x = ::worker::detail::internal::Pbio_GetDouble(object, 1);
  auto _y = ::worker::detail::internal::Pbio_GetDouble(object, 2);
  auto _z = ::worker::detail::internal::Pbio_GetDouble(object, 3);
  return {
      std::move(_x),
      std::move(_y),
      std::move(_z)};
}

void Write_Vector3f(
    const ::improbable::Vector3f& data, ::worker::detail::internal::Pbio_Object* object) {
  ::worker::detail::internal::Pbio_AddFloat(object, 1, data.x());
  ::worker::detail::internal::Pbio_AddFloat(object, 2, data.y());
  ::worker::detail::internal::Pbio_AddFloat(object, 3, data.z());
}

::improbable::Vector3f Read_Vector3f(::worker::detail::internal::Pbio_Object* object) {
  auto _x = ::worker::detail::internal::Pbio_GetFloat(object, 1);
  auto _y = ::worker::detail::internal::Pbio_GetFloat(object, 2);
  auto _z = ::worker::detail::internal::Pbio_GetFloat(object, 3);
  return {
      std::move(_x),
      std::move(_y),
      std::move(_z)};
}

}  // ::detail

// Implementation of Vector3d.
//----------------------------------------------------------------

Vector3d::Vector3d(
    double x,
    double y,
    double z)
: _x{x}
, _y{y}
, _z{z} {}

Vector3d::Vector3d()
: _x{0}
, _y{0}
, _z{0} {}

bool Vector3d::operator==(const Vector3d& value) const {
  return
      _x == value._x &&
      _y == value._y &&
      _z == value._z;
}

bool Vector3d::operator!=(const Vector3d& value) const {
  return !operator==(value);
}

double Vector3d::x() const {
  return _x;
}

double& Vector3d::x() {
  return _x;
}

Vector3d& Vector3d::set_x(double value) {
  _x = value;
  return *this;
}

double Vector3d::y() const {
  return _y;
}

double& Vector3d::y() {
  return _y;
}

Vector3d& Vector3d::set_y(double value) {
  _y = value;
  return *this;
}

double Vector3d::z() const {
  return _z;
}

double& Vector3d::z() {
  return _z;
}

Vector3d& Vector3d::set_z(double value) {
  _z = value;
  return *this;
}

// Implementation of Vector3f.
//----------------------------------------------------------------

Vector3f::Vector3f(
    float x,
    float y,
    float z)
: _x{x}
, _y{y}
, _z{z} {}

Vector3f::Vector3f()
: _x{0}
, _y{0}
, _z{0} {}

bool Vector3f::operator==(const Vector3f& value) const {
  return
      _x == value._x &&
      _y == value._y &&
      _z == value._z;
}

bool Vector3f::operator!=(const Vector3f& value) const {
  return !operator==(value);
}

float Vector3f::x() const {
  return _x;
}

float& Vector3f::x() {
  return _x;
}

Vector3f& Vector3f::set_x(float value) {
  _x = value;
  return *this;
}

float Vector3f::y() const {
  return _y;
}

float& Vector3f::y() {
  return _y;
}

Vector3f& Vector3f::set_y(float value) {
  _y = value;
  return *this;
}

float Vector3f::z() const {
  return _z;
}

float& Vector3f::z() {
  return _z;
}

Vector3f& Vector3f::set_z(float value) {
  _z = value;
  return *this;
}

}  // ::improbable

std::size_t std::hash< ::improbable::Vector3d >::operator()(const ::improbable::Vector3d& value) const {
  size_t result = 1327;
  result = (result * 977) + std::hash< double >{}(value.x());
  result = (result * 977) + std::hash< double >{}(value.y());
  result = (result * 977) + std::hash< double >{}(value.z());
  return result;
}

std::size_t std::hash< ::improbable::Vector3f >::operator()(const ::improbable::Vector3f& value) const {
  size_t result = 1327;
  result = (result * 977) + std::hash< float >{}(value.x());
  result = (result * 977) + std::hash< float >{}(value.y());
  result = (result * 977) + std::hash< float >{}(value.z());
  return result;
}
