//============================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//  Copyright 2015 Sandia Corporation.
//  Copyright 2015 UT-Battelle, LLC.
//  Copyright 2015 Los Alamos National Security.
//
//  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
//  the U.S. Government retains certain rights in this software.
//
//  Under the terms of Contract DE-AC52-06NA25396 with Los Alamos National
//  Laboratory (LANL), the U.S. Government retains certain rights in
//  this software.
//============================================================================
#ifndef vtk_m_exec_arg_ThreadIndicesBasic_h
#define vtk_m_exec_arg_ThreadIndicesBasic_h

#include <vtkm/internal/Invocation.h>

namespace vtkm {
namespace exec {
namespace arg {

/// \brief Basic container for thread indices in a worklet invocation
///
/// During the execution of a worklet function in an execution environment
/// thread, VTK-m has to manage several indices. To simplify this management
/// and to provide a single place to store them (so that they do not have to be
/// recomputed), \c WorkletInvokeFunctor creates a \c ThreadIndices object.
/// This object gets passed to \c Fetch operations to help them load data.
///
/// All \c ThreadIndices classes should implement the functions provided in
/// the \c ThreadIndicesBasic class. (It is in fact a good idea to subclass
/// it.) Other \c ThreadIndices classes may provide additional indices if
/// appropriate for the scheduling.
///
class ThreadIndicesBasic
{
public:
  template<typename Invocation>
  VTKM_EXEC_EXPORT
  ThreadIndicesBasic(vtkm::Id threadIndex, const Invocation &)
    : Index(threadIndex)
  {
  }

  /// \brief The index into the input domain.
  ///
  /// This index refers to the input element (array value, cell, etc.) that
  /// this thread is being invoked for. This is the typical index used during
  /// fetches.
  ///
  VTKM_EXEC_EXPORT
  vtkm::Id GetIndex() const { return this->Index; }

  /// \brief The 3D index into the input domain.
  ///
  /// This index refers to the input element (array value, cell, etc.) that
  /// this thread is being invoked for. If the input domain has 2 or 3
  /// dimensional indexing, this result will preserve that. If the domain
  /// indexing is just one dimensional, the result will have the index in the
  /// first component with the remaining components set to 0.
  ///
  VTKM_EXEC_EXPORT
  vtkm::Id3 GetIndex3D() const { return vtkm::Id3(this->GetIndex(), 0, 0); }

private:
  vtkm::Id Index;
};

}
}
} // namespace vtkm::exec::arg

#endif //vtk_m_exec_arg_ThreadIndicesBasic_h