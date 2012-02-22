#ifndef PCLtoVTK_HPP
#define PCLtoVTK_HPP

// PCL
#include <pcl/common/common.h>

// VTK
#include <vtkFloatArray.h>
#include <vtkSmartPointer.h>
#include <vtkUnsignedCharArray.h>
#include <vtkVertexGlyphFilter.h>

template <typename CloudT>
void PCLtoVTK(const CloudT& cloud, vtkPolyData* const pdata)
{
  typename CloudT::PointType testPoint = cloud.points[0];

  typedef typename pcl::traits::fieldList<typename CloudT::PointType>::type FieldList;

  // Coordiantes (always must have coordinates)
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  for (size_t i = 0; i < cloud.points.size (); ++i)
    {
    double p[3];
    p[0] = cloud.points[i].x;
    p[1] = cloud.points[i].y;
    p[2] = cloud.points[i].z;
    points->InsertNextPoint (p);
    }

  // Create a temporary PolyData and add the points to it
  vtkSmartPointer<vtkPolyData> tempPolyData = vtkSmartPointer<vtkPolyData>::New();
  tempPolyData->SetPoints(points);

  // Normals (optional)
  bool has_x_normal = false; bool has_y_normal = false; bool has_z_normal = false;
  float x_normal_val = 0.0f; float y_normal_val = 0.0f; float z_normal_val = 0.0f;
  pcl::for_each_type<FieldList> (pcl::CopyIfFieldExists<typename CloudT::PointType, float> (testPoint, "x_normal", has_x_normal, x_normal_val));
  pcl::for_each_type<FieldList> (pcl::CopyIfFieldExists<typename CloudT::PointType, float> (testPoint, "y_normal", has_y_normal, y_normal_val));
  pcl::for_each_type<FieldList> (pcl::CopyIfFieldExists<typename CloudT::PointType, float> (testPoint, "z_normal", has_z_normal, z_normal_val));
  if(has_x_normal && has_y_normal && has_z_normal)
  {
    typename CloudT::PointType testPoint = cloud.points[0];
    vtkSmartPointer<vtkFloatArray> normals = vtkSmartPointer<vtkFloatArray>::New();
    normals->SetNumberOfComponents(3); //3d normals (ie x,y,z)
    normals->SetNumberOfTuples(cloud.points.size());
    normals->SetName("Normals");

    for (size_t i = 0; i < cloud.points.size (); ++i)
      {
      typename CloudT::PointType p = cloud.points[i];
      pcl::for_each_type<FieldList> (pcl::CopyIfFieldExists<typename CloudT::PointType, float> (p, "x_normal", has_x_normal, x_normal_val));
      pcl::for_each_type<FieldList> (pcl::CopyIfFieldExists<typename CloudT::PointType, float> (p, "y_normal", has_y_normal, y_normal_val));
      pcl::for_each_type<FieldList> (pcl::CopyIfFieldExists<typename CloudT::PointType, float> (p, "z_normal", has_z_normal, z_normal_val));
      float normal[3] = {x_normal_val, y_normal_val, z_normal_val};
      normals->SetTupleValue(i, normal);
      }
    tempPolyData->GetPointData()->SetNormals(normals);
  }

  // Colors (optional)
  bool has_r = false; bool has_g = false; bool has_b = false;
  unsigned char r_val = 0; unsigned char g_val = 0; unsigned char b_val = 0;
  pcl::for_each_type<FieldList> (pcl::CopyIfFieldExists<typename CloudT::PointType, unsigned char> (testPoint, "r", has_r, r_val));
  pcl::for_each_type<FieldList> (pcl::CopyIfFieldExists<typename CloudT::PointType, unsigned char> (testPoint, "g", has_g, g_val));
  pcl::for_each_type<FieldList> (pcl::CopyIfFieldExists<typename CloudT::PointType, unsigned char> (testPoint, "b", has_b, b_val));
  if(has_r && has_g && has_b)
  {
    vtkSmartPointer<vtkUnsignedCharArray> colors = vtkSmartPointer<vtkUnsignedCharArray>::New();
    colors->SetNumberOfComponents(3);
    colors->SetNumberOfTuples(cloud.points.size());
    colors->SetName("RGB");

    for (size_t i = 0; i < cloud.points.size (); ++i)
      {
      typename CloudT::PointType p = cloud[i];
      pcl::for_each_type<FieldList> (pcl::CopyIfFieldExists<typename CloudT::PointType, unsigned char> (p, "r", has_r, r_val));
      pcl::for_each_type<FieldList> (pcl::CopyIfFieldExists<typename CloudT::PointType, unsigned char> (p, "g", has_g, g_val));
      pcl::for_each_type<FieldList> (pcl::CopyIfFieldExists<typename CloudT::PointType, unsigned char> (p, "b", has_b, b_val));
      unsigned char color[3] = {r_val, g_val, b_val};
      colors->SetTupleValue(i, color);
      }
    tempPolyData->GetPointData()->SetScalars(colors);
  }

  vtkSmartPointer<vtkVertexGlyphFilter> vertexGlyphFilter = vtkSmartPointer<vtkVertexGlyphFilter>::New();
  vertexGlyphFilter->AddInputConnection(tempPolyData->GetProducerPort());
  vertexGlyphFilter->Update();

  pdata->DeepCopy(vertexGlyphFilter->GetOutput());
}


//Template function declarations for inserting points of arbitrary dimension
template <typename CloudT>
void PCLtoVTK(CloudT* const cloud, vtkStructuredGrid* const structuredGrid)
{
  // This generic template will convert any PCL point type with .x, .y, and .z members
  // to a coordinate-only vtkPolyData.
  std::cout << "Generic" << std::endl;

  int dimensions[3] = {cloud->width, cloud->height, 1};
  structuredGrid->SetDimensions(dimensions);

  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  points->SetNumberOfPoints(cloud->width * cloud->height);

  for (size_t i = 0; i < cloud->width; ++i)
  {
    for (size_t j = 0; j < cloud->height; ++j)
    {
      int queryPoint[3] = {i, j, 0};
      vtkIdType pointId = vtkStructuredData::ComputePointId(dimensions, queryPoint);
      typename CloudT::PointType point = (*cloud)(i,j);

      if(pcl::isFinite(point))
      {
        float p[3] = {point.x, point.y, point.z};
        points->SetPoint(pointId, p);
      }
      else
      {
        float p[3] = {0,0,0};
        points->SetPoint(pointId, p);
        structuredGrid->BlankPoint(pointId);
      }
    }
  }

  structuredGrid->SetPoints(points);

}

#endif
