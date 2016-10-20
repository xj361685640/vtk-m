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
#ifndef vtk_m_rendering_Mapper_h
#define vtk_m_rendering_Mapper_h

#include <vtkm/rendering/Camera.h>
#include <vtkm/rendering/Canvas.h>
#include <vtkm/rendering/ColorTable.h>
namespace vtkm {
namespace rendering {

class VTKM_RENDERING_EXPORT Mapper
{
public:
  Mapper()
  {
  }

  virtual ~Mapper();


  virtual void RenderCells(const vtkm::cont::DynamicCellSet &cellset,
                           const vtkm::cont::CoordinateSystem &coords,
                           const vtkm::cont::Field &scalarField,
                           const vtkm::rendering::ColorTable &colorTable,
                           const vtkm::rendering::Camera &camera,
                           const vtkm::Range &scalarRange) = 0;

  virtual void SetActiveColorTable(const ColorTable &ct);

  virtual void StartScene() = 0;
  virtual void EndScene() = 0;
  virtual void SetCanvas(vtkm::rendering::Canvas *canvas) = 0;

  virtual vtkm::rendering::Mapper *NewCopy() const = 0;

protected:
  vtkm::cont::ArrayHandle<vtkm::Vec<vtkm::Float32,4> > ColorMap;
};
}} //namespace vtkm::rendering
#endif //vtk_m_rendering_Mapper_h
