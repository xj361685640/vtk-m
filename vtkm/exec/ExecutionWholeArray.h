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
#ifndef vtk_m_exec_ExecutionWholeArray_h
#define vtk_m_exec_ExecutionWholeArray_h

#include <vtkm/cont/DeviceAdapter.h>
#include <vtkm/cont/ArrayHandle.h>
#include <vtkm/exec/ExecutionObjectBase.h>

namespace vtkm {
namespace exec {

/// \c ExecutionWholeArray is an execution object that allows an array handle
/// content to be a parameter in an execution environment
/// function. This can be used to allow worklets to have a shared search
/// structure
///
template<typename T,
         typename StorageTag = VTKM_DEFAULT_STORAGE_TAG,
         typename DeviceAdapterTag = VTKM_DEFAULT_DEVICE_ADAPTER_TAG
         >
class ExecutionWholeArray : public vtkm::exec::ExecutionObjectBase
{
public:
  VTKM_CONT_EXPORT
  ExecutionWholeArray( ):
    Portal( )
  {
  }

  VTKM_CONT_EXPORT
  ExecutionWholeArray( const vtkm::cont::ArrayHandle<T,StorageTag>& handle,
                       vtkm::Id length ):
    Portal( handle.PrepareForInPlace(length, DeviceAdapterTag()) )
  {
  }

  VTKM_EXEC_EXPORT
  T Get(vtkm::Id index) const { return this->Portal.Get(index); }

  VTKM_EXEC_EXPORT
  void Set(vtkm::Id index, const T& t) const { return this->Portal.Set(index, t); }

private:
  typedef vtkm::cont::ArrayHandle<T,StorageTag> HandleType;
  typedef typename HandleType::template ExecutionTypes<DeviceAdapterTag>::Portal PortalType;
  PortalType Portal;
};

/// \c ExecutionWholeArrayConst is an execution object that allows an array handle
/// content to be a parameter in an execution environment
/// function. This can be used to allow worklets to have a shared search
/// structure
///
template<typename T,
         typename StorageTag = VTKM_DEFAULT_STORAGE_TAG,
         typename DeviceAdapterTag = VTKM_DEFAULT_DEVICE_ADAPTER_TAG
         >
class ExecutionWholeArrayConst : public vtkm::exec::ExecutionObjectBase
{
public:
  VTKM_CONT_EXPORT
  ExecutionWholeArrayConst( ):
    Portal( )
  {
  }

  VTKM_CONT_EXPORT
  ExecutionWholeArrayConst( const vtkm::cont::ArrayHandle<T,StorageTag>& handle):
    Portal( handle.PrepareForInput( DeviceAdapterTag() ) )
  {
  }

  VTKM_EXEC_EXPORT
  T Get(vtkm::Id index) const { return this->Portal.Get(index); }

private:
  typedef vtkm::cont::ArrayHandle<T,StorageTag> HandleType;
  typedef typename HandleType::template ExecutionTypes<DeviceAdapterTag>::PortalConst PortalType;
  PortalType Portal;
};

}
} // namespace vtkm::exec

#endif //vtk_m_exec_ExecutionObjectBase_h