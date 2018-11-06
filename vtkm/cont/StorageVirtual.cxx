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
#include "StorageVirtual.h"

#include <vtkm/cont/internal/DeviceAdapterError.h>

namespace vtkm
{
namespace cont
{
namespace internal
{


//--------------------------------------------------------------------
Storage<void, ::vtkm::cont::StorageTagVirtual>::Storage(
  const Storage<void, vtkm::cont::StorageTagVirtual>& src)
  : HostUpToDate(src.HostUpToDate)
  , DeviceUpToDate(src.DeviceUpToDate)
  , DeviceTransferState(src.DeviceTransferState)
{
}

//--------------------------------------------------------------------
Storage<void, ::vtkm::cont::StorageTagVirtual>::Storage(
  Storage<void, vtkm::cont::StorageTagVirtual>&& src) noexcept
  : HostUpToDate(src.HostUpToDate)
  , DeviceUpToDate(src.DeviceUpToDate)
  , DeviceTransferState(std::move(src.DeviceTransferState))
{
}

//--------------------------------------------------------------------
Storage<void, vtkm::cont::StorageTagVirtual>& Storage<void, ::vtkm::cont::StorageTagVirtual>::
operator=(const Storage<void, vtkm::cont::StorageTagVirtual>& src)
{
  this->HostUpToDate = src.HostUpToDate;
  this->DeviceUpToDate = src.DeviceUpToDate;
  this->DeviceTransferState = src.DeviceTransferState;
  return *this;
}

//--------------------------------------------------------------------
Storage<void, vtkm::cont::StorageTagVirtual>& Storage<void, ::vtkm::cont::StorageTagVirtual>::
operator=(Storage<void, vtkm::cont::StorageTagVirtual>&& src) noexcept
{
  this->HostUpToDate = src.HostUpToDate;
  this->DeviceUpToDate = src.DeviceUpToDate;
  this->DeviceTransferState = std::move(src.DeviceTransferState);
  return *this;
}

//--------------------------------------------------------------------
Storage<void, ::vtkm::cont::StorageTagVirtual>::~Storage() {}

//--------------------------------------------------------------------
void Storage<void, ::vtkm::cont::StorageTagVirtual>::ReleaseResourcesExecution()
{
  this->DeviceTransferState->releaseDevice();
  this->DeviceUpToDate = false;
}

//--------------------------------------------------------------------
void Storage<void, ::vtkm::cont::StorageTagVirtual>::ReleaseResources()
{
  this->DeviceTransferState->releaseAll();
  this->HostUpToDate = false;
  this->DeviceUpToDate = false;
}

//--------------------------------------------------------------------
bool Storage<void, ::vtkm::cont::StorageTagVirtual>::IsSameType(const std::type_info& other) const
{
  return typeid(*this) == other;
}

//--------------------------------------------------------------------
std::unique_ptr<Storage<void, ::vtkm::cont::StorageTagVirtual>>
Storage<void, ::vtkm::cont::StorageTagVirtual>::NewInstance() const
{
  return std::move(this->MakeNewInstance());
}

//--------------------------------------------------------------------
const vtkm::internal::PortalVirtualBase*
Storage<void, ::vtkm::cont::StorageTagVirtual>::PrepareForInput(
  vtkm::cont::DeviceAdapterId devId) const
{
  if (devId == vtkm::cont::DeviceAdapterTagUndefined())
  {
    throw vtkm::cont::ErrorBadValue("device should not be VTKM_DEVICE_ADAPTER_UNDEFINED");
  }
  if (devId == vtkm::cont::DeviceAdapterTagError())
  {
    throw vtkm::cont::ErrorBadValue("device should not be VTKM_DEVICE_ADAPTER_ERROR");
  }

  const bool needsUpload = !(this->DeviceTransferState->valid(devId) && this->DeviceUpToDate);

  if (needsUpload)
  { //Either transfer state is pointing to another device, or has
    //had the execution resources released. Either way we
    //need to re-transfer the execution information
    auto* payload = this->DeviceTransferState.get();
    this->TransferPortalForInput(*payload, devId);
    this->HostUpToDate = false;
    this->DeviceUpToDate = true;
  }
  return this->DeviceTransferState->devicePtr();
}

//--------------------------------------------------------------------
const vtkm::internal::PortalVirtualBase*
Storage<void, ::vtkm::cont::StorageTagVirtual>::PrepareForOutput(vtkm::Id numberOfValues,
                                                                 vtkm::cont::DeviceAdapterId devId)
{
  if (devId == vtkm::cont::DeviceAdapterTagUndefined())
  {
    throw vtkm::cont::ErrorBadValue("device should not be VTKM_DEVICE_ADAPTER_UNDEFINED");
  }
  if (devId == vtkm::cont::DeviceAdapterTagError())
  {
    throw vtkm::cont::ErrorBadValue("device should not be VTKM_DEVICE_ADAPTER_ERROR");
  }

  const bool needsUpload = !(this->DeviceTransferState->valid(devId) && this->DeviceUpToDate);
  if (needsUpload)
  {
    this->TransferPortalForOutput(*(this->DeviceTransferState), numberOfValues, devId);
    this->HostUpToDate = false;
    this->DeviceUpToDate = true;
  }
  return this->DeviceTransferState->devicePtr();
}

//--------------------------------------------------------------------
const vtkm::internal::PortalVirtualBase*
Storage<void, ::vtkm::cont::StorageTagVirtual>::GetPortalControl()
{
  if (!this->HostUpToDate)
  {
    //we need to prepare for input and grab the host ptr
    auto* payload = this->DeviceTransferState.get();
    this->ControlPortalForOutput(*payload);
  }

  this->DeviceUpToDate = false;
  this->HostUpToDate = true;
  return this->DeviceTransferState->hostPtr();
}

//--------------------------------------------------------------------
const vtkm::internal::PortalVirtualBase*
Storage<void, ::vtkm::cont::StorageTagVirtual>::GetPortalConstControl() const
{
  if (!this->HostUpToDate)
  {
    //we need to prepare for input and grab the host ptr
    vtkm::cont::internal::TransferInfoArray* payload = this->DeviceTransferState.get();
    this->ControlPortalForInput(*payload);
  }
  //We need to mark the device out of date after the conditions
  //as they can modify the state of the device
  this->DeviceUpToDate = false;
  this->HostUpToDate = true;
  return this->DeviceTransferState->hostPtr();
}

//--------------------------------------------------------------------
void Storage<void, ::vtkm::cont::StorageTagVirtual>::ControlPortalForOutput(
  vtkm::cont::internal::TransferInfoArray&)
{
  throw vtkm::cont::ErrorBadValue("StorageTagVirtual by default doesn't support output.");
}

//--------------------------------------------------------------------
void Storage<void, ::vtkm::cont::StorageTagVirtual>::TransferPortalForOutput(
  vtkm::cont::internal::TransferInfoArray&,
  vtkm::Id,
  vtkm::cont::DeviceAdapterId)
{
  throw vtkm::cont::ErrorBadValue("StorageTagVirtual by default doesn't support output.");
}
}
}
}