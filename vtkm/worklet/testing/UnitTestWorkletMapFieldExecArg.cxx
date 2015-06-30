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

#include <vtkm/worklet/DispatcherMapField.h>
#include <vtkm/worklet/WorkletMapField.h>

#include <vtkm/cont/ArrayHandle.h>
#include <vtkm/cont/ArrayHandleCounting.h>
#include <vtkm/cont/DynamicArrayHandle.h>

#include <vtkm/exec/ExecutionWholeArray.h>

#include <vtkm/cont/testing/Testing.h>

namespace map_exec_field{
namespace worklets {

class TestWorklet : public vtkm::worklet::WorkletMapField
{
public:
  typedef void ControlSignature( FieldIn<>, ExecObject, FieldOut<>);
  typedef void ExecutionSignature(_1, _2, _3);

  template<typename T, typename StorageTag>
  VTKM_EXEC_EXPORT
  void operator()(const vtkm::Id &index,
                  const vtkm::exec::ExecutionWholeArrayConst<T,StorageTag> &execArg,
                  T& out) const
  {
    if (!test_equal(execArg.Get(index), TestValue(index, T()) + T(100)))
    {
      this->RaiseError("Got wrong input value.");
    }
    out = execArg.Get(index) - T(100);
  }

  template<typename T1, typename T2>
  VTKM_EXEC_EXPORT
  void operator()(const vtkm::Id &, const T1 &, const T2 &) const
  {
    this->RaiseError("Cannot call this worklet with different types.");
  }
};

} // worklet namespace

static const vtkm::Id ARRAY_SIZE = 10;

template<typename WorkletType>
struct DoTestWorklet
{
  template<typename T>
  VTKM_CONT_EXPORT
  void operator()(T) const
  {
    std::cout << "Set up data." << std::endl;
    T inputArray[ARRAY_SIZE];

    for (vtkm::Id index = 0; index < ARRAY_SIZE; index++)
    {
      inputArray[index] = TestValue(index, T()) + T(100);
    }

    vtkm::cont::ArrayHandleCounting<vtkm::Id> counting(0,ARRAY_SIZE);
    vtkm::cont::ArrayHandle<T> inputHandle =
        vtkm::cont::make_ArrayHandle(inputArray, ARRAY_SIZE);
    vtkm::cont::ArrayHandle<T> outputHandle;

    std::cout << "Create and run dispatcher." << std::endl;
    vtkm::worklet::DispatcherMapField<WorkletType> dispatcher;
    dispatcher.Invoke(counting,
                      vtkm::exec::ExecutionWholeArrayConst<T>(inputHandle),
                      outputHandle);

    std::cout << "Check result." << std::endl;
    CheckPortal(outputHandle.GetPortalConstControl());

    std::cout << "Repeat with dynamic arrays." << std::endl;
    // Clear out output array.
    outputHandle = vtkm::cont::ArrayHandle<T>();
    vtkm::cont::DynamicArrayHandle outputDynamic(outputHandle);
    dispatcher.Invoke(counting,
                      vtkm::exec::ExecutionWholeArrayConst<T>(inputHandle),
                      outputDynamic);
    CheckPortal(outputHandle.GetPortalConstControl());
  }
};

void TestWorkletMapFieldExecArg()
{
  typedef vtkm::cont::internal::DeviceAdapterTraits<
                    VTKM_DEFAULT_DEVICE_ADAPTER_TAG> DeviceAdapterTraits;
  std::cout << "Testing Worklet with ExecutionWholeArray on device adapter: "
            << DeviceAdapterTraits::GetId() << std::endl;

  std::cout << "--- Worklet accepting all types." << std::endl;
  vtkm::testing::Testing::TryTypes(
                         map_exec_field::DoTestWorklet< worklets::TestWorklet >(),
                         vtkm::TypeListTagCommon());

}

} // anonymous namespace

int UnitTestWorkletMapFieldExecArg(int, char *[])
{
  return vtkm::cont::testing::Testing::Run(map_exec_field::TestWorkletMapFieldExecArg);
}