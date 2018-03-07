//============================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//  Copyright 2014 National Technology & Engineering Solutions of Sandia, LLC (NTESS).
//  Copyright 2014 UT-Battelle, LLC.
//  Copyright 2014 Los Alamos National Security.
//
//  Under the terms of Contract DE-NA0003525 with NTESS,
//  the U.S. Government retains certain rights in this software.
//
//  Under the terms of Contract DE-AC52-06NA25396 with Los Alamos National
//  Laboratory (LANL), the U.S. Government retains certain rights in
//  this software.
//============================================================================

#include <vtkm/cont/testing/MakeTestDataSet.h>
#include <vtkm/cont/testing/Testing.h>
#include <vtkm/filter/CrossProduct.h>

#include <random>
#include <vector>

namespace
{
std::mt19937 randGenerator;

template <typename T>
void createVectors(std::size_t numPts,
                   int vecType,
                   std::vector<vtkm::Vec<T, 3>>& vecs1,
                   std::vector<vtkm::Vec<T, 3>>& vecs2)
{
  if (vecType == 0) // X x Y
  {
    vecs1.resize(numPts, vtkm::make_Vec(1, 0, 0));
    vecs2.resize(numPts, vtkm::make_Vec(0, 1, 0));
  }
  else if (vecType == 1) // Y x Z
  {
    vecs1.resize(numPts, vtkm::make_Vec(0, 1, 0));
    vecs2.resize(numPts, vtkm::make_Vec(0, 0, 1));
  }
  else if (vecType == 2) // Z x X
  {
    vecs1.resize(numPts, vtkm::make_Vec(0, 0, 1));
    vecs2.resize(numPts, vtkm::make_Vec(1, 0, 0));
  }
  else if (vecType == 3) // Y x X
  {
    vecs1.resize(numPts, vtkm::make_Vec(0, 1, 0));
    vecs2.resize(numPts, vtkm::make_Vec(1, 0, 0));
  }
  else if (vecType == 4) // Z x Y
  {
    vecs1.resize(numPts, vtkm::make_Vec(0, 0, 1));
    vecs2.resize(numPts, vtkm::make_Vec(0, 1, 0));
  }
  else if (vecType == 5) // X x Z
  {
    vecs1.resize(numPts, vtkm::make_Vec(1, 0, 0));
    vecs2.resize(numPts, vtkm::make_Vec(0, 0, 1));
  }
  else if (vecType == 6)
  {
    //Test some other vector combinations
    std::uniform_real_distribution<vtkm::Float64> randomDist(-10.0, 10.0);
    randomDist(randGenerator);

    vecs1.resize(numPts);
    vecs2.resize(numPts);
    for (std::size_t i = 0; i < numPts; i++)
    {
      vecs1[i] = vtkm::make_Vec(
        randomDist(randGenerator), randomDist(randGenerator), randomDist(randGenerator));
      vecs2[i] = vtkm::make_Vec(
        randomDist(randGenerator), randomDist(randGenerator), randomDist(randGenerator));
    }
  }
}

void TestCrossProduct()
{
  std::cout << "Testing CrossProduct Filter" << std::endl;

  vtkm::cont::testing::MakeTestDataSet testDataSet;

  const int numCases = 7;
  for (int i = 0; i < numCases; i++)
  {
    vtkm::cont::DataSet dataSet = testDataSet.Make3DUniformDataSet0();
    vtkm::Id nVerts = dataSet.GetCoordinateSystem(0).GetData().GetNumberOfValues();

    std::vector<vtkm::Vec<vtkm::FloatDefault, 3>> vecs1, vecs2;
    createVectors(static_cast<std::size_t>(nVerts), i, vecs1, vecs2);

    vtkm::cont::ArrayHandle<vtkm::Vec<vtkm::FloatDefault, 3>> field1, field2;
    field1 = vtkm::cont::make_ArrayHandle(vecs1);
    field2 = vtkm::cont::make_ArrayHandle(vecs2);

    vtkm::cont::DataSetFieldAdd::AddPointField(dataSet, "vec1", field1);
    vtkm::cont::DataSetFieldAdd::AddPointField(dataSet, "vec2", field2);

    vtkm::filter::Result result;
    vtkm::filter::CrossProduct filter;

    filter.SetSecondaryFieldName("vec2");
    result = filter.Execute(dataSet, dataSet.GetField("vec1"));

    VTKM_TEST_ASSERT(result.IsValid(), "Result should be valid");
    VTKM_TEST_ASSERT(result.GetField().GetName() == "crossproduct", "Output field has wrong name.");
    VTKM_TEST_ASSERT(result.GetField().GetAssociation() == vtkm::cont::Field::ASSOC_POINTS,
                     "Output field has wrong association");

    vtkm::cont::ArrayHandle<vtkm::Vec<vtkm::FloatDefault, 3>> outputArray;
    if (result.FieldAs(outputArray))
    {
      auto v1Portal = field1.GetPortalConstControl();
      auto v2Portal = field2.GetPortalConstControl();
      auto outPortal = outputArray.GetPortalConstControl();

      for (vtkm::Id j = 0; j < outputArray.GetNumberOfValues(); j++)
      {
        vtkm::Vec<vtkm::FloatDefault, 3> v1 = v1Portal.Get(j);
        vtkm::Vec<vtkm::FloatDefault, 3> v2 = v2Portal.Get(j);
        vtkm::Vec<vtkm::FloatDefault, 3> res = outPortal.Get(j);

        //Make sure result is orthogonal each input vector. Need to normalize to compare with zero.
        vtkm::Vec<vtkm::FloatDefault, 3> v1N(vtkm::Normal(v1)), v2N(vtkm::Normal(v1)),
          resN(vtkm::Normal(res));
        VTKM_TEST_ASSERT(test_equal(vtkm::dot(resN, v1N), vtkm::FloatDefault(0.0)),
                         "Wrong result for cross product");
        VTKM_TEST_ASSERT(test_equal(vtkm::dot(resN, v2N), vtkm::FloatDefault(0.0)),
                         "Wrong result for cross product");

        vtkm::FloatDefault sinAngle =
          vtkm::Magnitude(res) * vtkm::RMagnitude(v1) * vtkm::RMagnitude(v2);
        vtkm::FloatDefault cosAngle =
          vtkm::dot(v1, v2) * vtkm::RMagnitude(v1) * vtkm::RMagnitude(v2);
        VTKM_TEST_ASSERT(
          test_equal(sinAngle * sinAngle + cosAngle * cosAngle, vtkm::FloatDefault(1.0)),
          "Bad cross product length.");
      }
    }
  }
}
}

int UnitTestCrossProductFilter(int, char* [])
{
  return vtkm::cont::testing::Testing::Run(TestCrossProduct);
}