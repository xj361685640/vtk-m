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

#ifndef vtk_m_filter_ExtractPoints_h
#define vtk_m_filter_ExtractPoints_h

#include <vtkm/filter/FilterDataSet.h>
#include <vtkm/filter/CleanGrid.h>
#include <vtkm/worklet/ExtractPoints.h>
#include <vtkm/ImplicitFunctions.h>

namespace vtkm {
namespace filter {

class ExtractPoints : public vtkm::filter::FilterDataSet<ExtractPoints>
{
public:
  VTKM_CONT
  ExtractPoints();

  // When CompactPoints is set, instead of copying the points and point fields
  // from the input, the filter will create new compact fields without the unused elements
  VTKM_CONT
  bool GetCompactPoints() const     { return this->CompactPoints; }
  VTKM_CONT
  void SetCompactPoints(bool value) { this->CompactPoints = value; }

  // Set the list of point ids to extract
  VTKM_CONT
  vtkm::cont::ArrayHandle<vtkm::Id> GetPointIds() const     { return this->PointIds; }
  VTKM_CONT
  void SetPointIds(vtkm::cont::ArrayHandle<vtkm::Id> &pointIds);

  // Set the volume of interest to extract
/*
  VTKM_CONT
  vtkm::ImplicitFunction GetVolumeOfInterest() const     { return this->VolumeOfInterest; }
  VTKM_CONT
  void SetVolumeOfInterest(vtkm::ImplicitFunction &implicitFunction);
*/

  template<typename DerivedPolicy, typename DeviceAdapter>
  VTKM_CONT
  vtkm::filter::ResultDataSet DoExecute(const vtkm::cont::DataSet& input,
                                        const vtkm::filter::PolicyBase<DerivedPolicy>& policy,
                                        const DeviceAdapter& tag);

  //Map a new field onto the resulting dataset after running the filter
  template<typename T, typename StorageType, typename DerivedPolicy, typename DeviceAdapter>
  VTKM_CONT
  bool DoMapField(vtkm::filter::ResultDataSet& result,
                  const vtkm::cont::ArrayHandle<T, StorageType>& input,
                  const vtkm::filter::FieldMetadata& fieldMeta,
                  const vtkm::filter::PolicyBase<DerivedPolicy>& policy,
                  const DeviceAdapter& tag);


private:
  int ExtractType;
  vtkm::cont::ArrayHandle<vtkm::Id> PointIds;
  //vtkm::ImplicitFunction VolumeOfInterest;

  bool CompactPoints;
  vtkm::filter::CleanGrid Compactor;
};

}
} // namespace vtkm::filter


#include <vtkm/filter/ExtractPoints.hxx>

#endif // vtk_m_filter_ExtractPoints_h
