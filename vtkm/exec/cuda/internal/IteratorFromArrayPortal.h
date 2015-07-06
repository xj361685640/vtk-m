//============================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//  Copyright 2014 Sandia Corporation.
//  Copyright 2014 UT-Battelle, LLC.
//  Copyright 2014 Los Alamos National Security.
//
//  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
//  the U.S. Government retains certain rights in this software.
//
//  Under the terms of Contract DE-AC52-06NA25396 with Los Alamos National
//  Laboratory (LANL), the U.S. Government retains certain rights in
//  this software.
//============================================================================
#ifndef vtk_m_exec_cuda_internal_IteratorFromArrayPortal_h
#define vtk_m_exec_cuda_internal_IteratorFromArrayPortal_h

#include <vtkm/Types.h>
#include <vtkm/Pair.h>
#include <vtkm/internal/ExportMacros.h>

// Disable warnings we check vtkm for but Thrust does not.
#if defined(VTKM_GCC) || defined(VTKM_CLANG)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wconversion"
#endif // gcc || clang

#include <thrust/functional.h>
#include <thrust/iterator/iterator_facade.h>
#include <thrust/system/cuda/execution_policy.h>

#if defined(VTKM_GCC) || defined(VTKM_CLANG)
#pragma GCC diagnostic pop
#endif // gcc || clang

namespace vtkm {
namespace exec {
namespace cuda {
namespace internal {

template<class ArrayPortalType>
struct PortalValue
{
  typedef typename ArrayPortalType::ValueType ValueType;

  VTKM_EXEC_CONT_EXPORT
  PortalValue(const ArrayPortalType &portal, vtkm::Id index)
    : Portal(portal), Index(index) {  }

  VTKM_EXEC_EXPORT
  void Swap( PortalValue<ArrayPortalType> &rhs ) throw()
  {
    //we need use the explicit type not a proxy temp object
    //A proxy temp object would point to the same underlying data structure
    //and would not hold the old value of *this once *this was set to rhs.
    const ValueType aValue = *this;
    *this = rhs;
    rhs = aValue;
  }

  VTKM_EXEC_EXPORT
  PortalValue<ArrayPortalType> &operator=(
    const PortalValue<ArrayPortalType> &rhs)
  {
    this->Portal.Set(this->Index, rhs.Portal.Get(rhs.Index));
    return *this;
  }

  VTKM_EXEC_EXPORT
  ValueType operator=(const ValueType& value)
  {
    this->Portal.Set(this->Index, value);
    return value;
  }

  VTKM_EXEC_EXPORT
  operator ValueType(void) const
  {
    return this->Portal.Get(this->Index);
  }

  const ArrayPortalType& Portal;
  vtkm::Id Index;
};

template<class ArrayPortalType>
class IteratorFromArrayPortal : public
    ::thrust::iterator_facade<
      IteratorFromArrayPortal<ArrayPortalType>,
      typename ArrayPortalType::ValueType,
      ::thrust::system::cuda::tag,
      ::thrust::random_access_traversal_tag,
      PortalValue<ArrayPortalType>,
      vtkm::Id>
{
public:

  VTKM_EXEC_CONT_EXPORT
  IteratorFromArrayPortal()
    : Portal(), Index(0) { }

  VTKM_CONT_EXPORT
  explicit IteratorFromArrayPortal(const ArrayPortalType &portal,
                                   vtkm::Id index = 0)
    : Portal(portal), Index(index) {  }

  VTKM_EXEC_EXPORT
  PortalValue<ArrayPortalType>
  operator[](std::ptrdiff_t idx) const //NEEDS to be signed
  {
    return PortalValue<ArrayPortalType>(this->Portal,
           this->Index + static_cast<vtkm::Id>(idx) );
  }

private:
  ArrayPortalType Portal;
  vtkm::Id Index;

  // Implementation for ::thrust iterator_facade
  friend class ::thrust::iterator_core_access;

  VTKM_EXEC_EXPORT
  PortalValue<ArrayPortalType> dereference() const
  {
    return PortalValue<ArrayPortalType>(this->Portal,
           this->Index);
  }

  VTKM_EXEC_EXPORT
  bool equal(const IteratorFromArrayPortal<ArrayPortalType> &other) const
  {
    // Technically, we should probably check that the portals are the same,
    // but the portal interface does not specify an equal operator.  It is
    // by its nature undefined what happens when comparing iterators from
    // different portals anyway.
    return (this->Index == other.Index);
  }

  VTKM_EXEC_CONT_EXPORT
  void increment()
  {
    this->Index++;
  }

  VTKM_EXEC_CONT_EXPORT
  void decrement()
  {
    this->Index--;
  }

  VTKM_EXEC_CONT_EXPORT
  void advance(vtkm::Id delta)
  {
    this->Index += delta;
  }

  VTKM_EXEC_CONT_EXPORT
  vtkm::Id
  distance_to(const IteratorFromArrayPortal<ArrayPortalType> &other) const
  {
    // Technically, we should probably check that the portals are the same,
    // but the portal interface does not specify an equal operator.  It is
    // by its nature undefined what happens when comparing iterators from
    // different portals anyway.
    return other.Index - this->Index;
  }
};

}
}
}
} //namespace vtkm::exec::cuda::internal

//So for the unary_transform_functor and binary_transform_functor inside
//of thrust, they verify that the index they are storing into is a reference
//instead of a value, so that the contents actually are written to global memory.
//
//But for vtk-m we pass in facade objects, which are passed by value, but
//must be treated as references. So do to do that properly we need to specialize
//is_non_const_reference to state a PortalValue by value is valid for writing
namespace thrust
{
namespace detail
{

  template< typename T > struct is_non_const_reference;

  template< typename T >
  struct is_non_const_reference< vtkm::exec::cuda::internal::PortalValue<T> >
    : thrust::detail::true_type
  { };

}
}

#endif //vtk_m_exec_cuda_internal_IteratorFromArrayPortal_h
