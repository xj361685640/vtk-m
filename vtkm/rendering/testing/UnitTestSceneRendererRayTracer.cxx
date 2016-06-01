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
#include <vtkm/cont/testing/MakeTestDataSet.h>
#include <vtkm/rendering/Window.h>
#include <vtkm/rendering/RenderSurfaceRayTracer.h>
#include <vtkm/rendering/WorldAnnotator.h>
#include <vtkm/rendering/Scene.h>
#include <vtkm/rendering/Plot.h>
#include <vtkm/rendering/SceneRendererRayTracer.h>
#include <vtkm/cont/DeviceAdapter.h>
#include <vtkm/cont/testing/Testing.h>

namespace {

void Set3DView(vtkm::rendering::View &view,
               const vtkm::cont::CoordinateSystem &coords,
               vtkm::Int32 w, vtkm::Int32 h)
{
    vtkm::Bounds coordsBounds =
        coords.GetBounds(VTKM_DEFAULT_DEVICE_ADAPTER_TAG());
    //set up a default view
    vtkm::Vec<vtkm::Float32,3> totalExtent;
    totalExtent[0] = vtkm::Float32(coordsBounds.X.Length());
    totalExtent[1] = vtkm::Float32(coordsBounds.Y.Length());
    totalExtent[2] = vtkm::Float32(coordsBounds.Z.Length());
    vtkm::Float32 mag = vtkm::Magnitude(totalExtent);
    vtkm::Normalize(totalExtent);

    view = vtkm::rendering::View(vtkm::rendering::View::VIEW_3D);
    view.View3d.Position = totalExtent * (mag * 2.f);
    view.View3d.Up = vtkm::Vec<vtkm::Float32,3>(0.f, 1.f, 0.f);
    view.View3d.LookAt = totalExtent * (mag * .5f);
    view.View3d.FieldOfView = 60.f;
    view.NearPlane = 1.f;
    view.FarPlane = 100.f;
    view.Width = w;
    view.Height = h;

    std::cout<<"View3d:  pos: "<<view.View3d.Position<<std::endl;
    std::cout<<"      lookAt: "<<view.View3d.LookAt<<std::endl;
    std::cout<<"          up: "<<view.View3d.Up<<std::endl;
    std::cout<<"near/far/fov: "<<view.NearPlane<<"/"<<view.FarPlane<<" "<<view.View3d.FieldOfView<<std::endl;
    std::cout<<"         w/h: "<<view.Width<<"/"<<view.Height<<std::endl;
}

void Render(const vtkm::cont::DataSet &ds,
            const std::string &fieldNm,
            const std::string &ctName,
            const std::string &outputFile)
{
    const vtkm::Int32 W = 512, H = 512;
    const vtkm::cont::CoordinateSystem coords = ds.GetCoordinateSystem();
    vtkm::rendering::SceneRendererRayTracer<VTKM_DEFAULT_DEVICE_ADAPTER_TAG> sceneRenderer;

    vtkm::rendering::View view;
    Set3DView(view, coords, W, H);

    vtkm::rendering::Scene scene;
    vtkm::rendering::Color bg(0.2f, 0.2f, 0.2f, 1.0f);
    vtkm::rendering::RenderSurfaceRayTracer surface(W,H,bg);
    scene.Plots.push_back(vtkm::rendering::Plot(ds.GetCellSet(),
                                                ds.GetCoordinateSystem(),
                                                ds.GetField(fieldNm),
                                                vtkm::rendering::ColorTable(ctName)));

    //TODO: W/H in window.  bg in window (window sets surface/renderer).
    vtkm::rendering::Window3D<vtkm::rendering::SceneRendererRayTracer<VTKM_DEFAULT_DEVICE_ADAPTER_TAG>,
                              vtkm::rendering::RenderSurfaceRayTracer,
                              vtkm::rendering::WorldAnnotator>
        w(scene, sceneRenderer, surface, view, bg);

    w.Initialize();
    w.Paint();
    w.SaveAs(outputFile);
}

void RenderTests()
{
    vtkm::cont::testing::MakeTestDataSet maker;

    //3D tests.
    Render(maker.Make3DRegularDataSet0(),
             "pointvar", "thermal", "reg3D.pnm");
    Render(maker.Make3DRectilinearDataSet0(),
             "pointvar", "thermal", "rect3D.pnm");
    Render(maker.Make3DExplicitDataSet4(),
             "pointvar", "thermal", "expl3D.pnm");
}

} //namespace
int UnitTestSceneRendererRayTracer(int, char *[])
{
    return vtkm::cont::testing::Testing::Run(RenderTests);
}
