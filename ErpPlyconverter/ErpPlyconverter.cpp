// ErpPlyconverter.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include<stdio.h>
#include<string>
#include <sstream>
#include <iostream>

#include "PCCPointSet.h"
#include "erp.h"

using namespace pcc;
using namespace erp;

void ErpToPointCloudPolar(CERPParameter &para);
void ErpToPointCloudPolar(CErpFrame &view, PCCPointSet3 &pointCloud);
void PointCloudToErpPolar(CERPParameter &para);
void PointCloudToErpPolar(PCCPointSet3 &pointCloud, CErpFrame &view);
std::string ToString(uint16_t W, uint16_t H, double Rnear, double Rfar, YUVformat yuvFormat, uint16_t bitDepth, size_t frameCounter);

int main(int argc, char *argv[])
{
	CERPParameter para;
	para.Parse(argc, argv);
	para.PrintParas();
	if (para.mode == 0) //ERP转点云(极坐标)
	{
		ErpToPointCloudPolar(para);
	}
	else if (para.mode == 1) //点云转ERP(极坐标)
	{
		PointCloudToErpPolar(para);
	}
	return 0;
}


//void CameraToWorld(pcc::PCCPoint3D &point3D, pcc::PCCPoint3D T, pcc::PCCPoint3D R)
//{
//	/*R = -R * PI / 180;
//
//	double x1 = point3D[0] * cos(R[1]) + point3D[2] * (-(sin(R[1])));
//	double y1 = point3D[1];
//	double z1 = point3D[0] * sin(R[1]) + point3D[2] * cos(R[1]);
//
//	double x2 = x1 * cos(R[0]) + y1 * sin(R[0]);
//	double y2 = x1 * (-sin(R[0])) + y1 * cos(R[0]);
//	double z2 = z1;
//
//	point3D[0] = x2;
//	point3D[1] = y2 * cos(R[2]) + z2 * sin(R[2]);
//	point3D[2] = y2 * (-sin(R[2])) + z2 * cos(R[2]);*/
//	point3D = point3D + T;
//}

void ErpToPointCloudPolar(CERPParameter &para)
{
	for (size_t i = 0; i < para.frameNumber; i++) //帧数
	{
		printf("正在转换第%d帧......\n", i + 1);

		PCCPointSet3 synPointCloud;

		for (int j = 0; j < para.viewNumber; j++) //每帧视点数
		{
			printf("\t正在合并第%d个视点:%s......\n",  j + 1,para.viewNames[j].c_str());

			CErpFrame view(para.viewNames[j], para.widths[j], para.heights[j], para.Rnears[j], para.Rfars[j],
				para.depthYUVFormats[j], para.depthbitDepths[j], para.textureYUVFormats[j],
				para.texturebitDepths[j], para.T[j], para.R[j]);

			view.read(para.textureFileNames[j], para.depthFileNames[j], i);
			view.textureYUV420ToYUV444();

			PCCPointSet3 pointCloud;
			ErpToPointCloudPolar(view, pointCloud);
			pointCloud.ToWorldCoordinates(para.R[j], para.T[j]);
			synPointCloud.addPoints(pointCloud);
		}

		synPointCloud.ToCameraCoordinates(para.R[0], para.T[0]);
		synPointCloud.ToPolarCoordinates(12, 11, 16, para.Rnears[0], para.Rfars[0]);

		printf("正在排序......\n");
		synPointCloud.sort();
		printf("排序完成......\n");

		printf("正在去除重复点......\n");
		synPointCloud.RemoveRepeatePoints();
		printf("去除重复点完成......\n");

		synPointCloud.ConvertColorFrom10bTo8b();
		synPointCloud.convertYUVToRGB();

		std::string synPointCloudName = para.outputPlyPath + "\\syn_" +
			ToString(para.widths[0], para.heights[0], para.Rnears[0], para.Rfars[0], para.depthYUVFormats[0],
				para.depthbitDepths[0], i) + ".ply";
		printf("\t正在将点云文件写入磁盘......\n");
		synPointCloud.write(synPointCloudName, true);
	}
}

void ErpToPointCloudPolar(CErpFrame &view, PCCPointSet3 &pointCloud)
{
	auto W = view.getWidth();
	auto H = view.getHeight();
	double phi;
	double theta;
	double R;

	long vmax = view.getVmax();
	double Rnear = view.getRnear();
	double Rfar = view.getRfar();

	pcc::PCCPoint3D T = view.getT();
	pcc::PCCPoint3D Rotation = view.getR();

	auto depth = view.getDepth();
	auto texture = view.getTexture();
	pcc::PCCPoint3D point3D;
	pcc::PCCColor3B Color3B;
	uint16_t Depth;
	size_t piontCount = pointCloud.getPointCount();
	pointCloud.resize(piontCount + W * H);

	for (size_t m = 0; m < H; m++)
	{
		for (size_t n = 0; n < W; n++)
		{
			Depth = (*depth)[0][m][n];
			if (W == H)
			{
				phi = ((n + 0.5) / W - 0.5) * PI;
				theta = (0.5 - (m + 0.5) / H)*PI;
			}
			else
			{
				phi = ((n + 0.5) / W - 0.5) * 2 * PI;
				theta = (0.5 - (m + 0.5) / H)*PI;
			}
			if (Depth != 0)
			{
				R = vmax * Rnear*Rfar / (Depth*(Rfar - Rnear) + vmax * Rnear);

				point3D[0] = R * cos(theta)*cos(phi);
				point3D[1] = -R * cos(theta)*sin(phi);
				point3D[2] = R * sin(theta);

				Color3B[0] = (*texture)[0][m][n];
				Color3B[1] = (*texture)[1][m][n];
				Color3B[2] = (*texture)[2][m][n];

				pointCloud.setPoint(piontCount, point3D);
				pointCloud.setColor(piontCount, Color3B);
				piontCount++;
			}
		}
	}
	pointCloud.resize(piontCount);
}

void PointCloudToErpPolar(PCCPointSet3 &pointCloud, CErpFrame &view)
{
	uint16_t W = view.getWidth();
	uint16_t H = view.getHeight();
	long vmax = view.getVmax();
	double Rnear = view.getRnear();
	double Rfar = view.getRfar();
	YUV* depth = view.getDepth();
	YUV* texture = view.getTexture();
	double X, Y, Z, R, phi, theta;
	int n, m;
	std::vector<std::vector<double>> nearest(H, std::vector<double>(W, INF));
	const size_t pointCount = pointCloud.getPointCount();
	for (int i = 0; i < pointCount; i++)
	{
		PCCPoint3D position = pointCloud[i];
		X = position.x();
		Y = position.y();
		Z = position.z();

		phi = -sign(Y)*acos(X / sqrt(X * X + Y * Y));
		theta = asin(Z / sqrt(X * X + Y * Y + Z * Z));

		if (W == H)
		{
			n = floor((phi / PI + 0.5)*W - 0.5 + 0.5);
			n = n >= W ? 0 : n;
			n = n < 0 ? W - 1 : n;
			m = floor((0.5 - theta / PI)*H - 0.5 + 0.5);
			m = m >= H ? 0 : m;
			m = m < 0 ? H - 1 : m;
		}
		else
		{
			n = floor((phi / (2 * PI) + 0.5)*W - 0.5 + 0.5);
			n = n >= W ? 0 : n;
			n = n < 0 ? W - 1 : n;
			m = floor((0.5 - theta / PI)*H - 0.5 + 0.5);
			m = m >= H ? 0 : m;
			m = m < 0 ? H - 1 : m;
		}

		R = sqrt(X * X + Y * Y + Z * Z);
		if (R < nearest[m][n])
		{
			nearest[m][n] = R;
			(*depth)[0][m][n] = floor(vmax * (1 / R - 1 / Rfar) / (1 / Rnear - 1 / Rfar) + 0.5);

			const pcc::PCCColor3B &color = pointCloud.getColor(i);
			(*texture)[0][m][n] = color[0];
			(*texture)[1][m][n] = color[1];
			(*texture)[2][m][n] = color[2];
		}
	}
}

void PointCloudToErpPolar(CERPParameter &para)
{
	for (size_t i = 0; i < para.frameNumber; i++) //帧数
	{
		printf("正在转换第%d帧......\n", i + 1);

		pcc::PCCVector3<double> T;
		pcc::PCCVector3<double> R;
		PCCPointSet3 pointCloud;

		pointCloud.read(para.plyFileNames[i]);
		pointCloud.convertRGBToYUV();
		pointCloud.ConvertColorFrom8bTo10b();

		pointCloud.ToWorldCoordinates(para.R[0], para.T[0]);
		pointCloud.ToDescartesCoordinates(12, 11, 16, para.Rnears[0], para.Rfars[0]);

		for (int j = 0; j < para.viewNumber; j++)
		{
			printf("正在生成第%d个视点：%s......\n", j + 1, para.viewNames[j].c_str());
			CErpFrame view(para.viewNames[j], para.widths[j], para.heights[j], para.Rnears[j], para.Rfars[j], para.depthYUVFormats[j], para.depthbitDepths[j],
				para.textureYUVFormats[j], para.texturebitDepths[j], para.T[j], para.R[j]);

			pointCloud.ToCameraCoordinates(para.R[j], para.T[j]);
			PointCloudToErpPolar(pointCloud, view);
			view.write(para.outputRecYUVPath, para.outputRecYUVPath);

			pointCloud.ToWorldCoordinates(para.R[j], para.T[j]);
		}
	}
}
std::string ToString(uint16_t W, uint16_t H, double Rnear, double Rfar, YUVformat yuvFormat, uint16_t bitDepth, size_t frameCounter)
{
	std::stringstream path;
	std::stringstream tempstrRnear;
	std::stringstream tempstrRfar;

	tempstrRnear << std::fixed << std::setprecision(1) << Rnear;
	tempstrRfar << std::fixed << std::setprecision(1) << Rfar;


	std::vector<std::string> tempstrRnearSplit = splitWithStl(tempstrRnear.str(), ".");
	std::vector<std::string> tempstrRfarSplit = splitWithStl(tempstrRfar.str(), ".");


	path << W << "_" << H << "_" << tempstrRnearSplit[0] << "_";
	path << tempstrRnearSplit[1] << "_" << tempstrRfarSplit[0] << "_" << tempstrRfarSplit[1] << "_";
	if (yuvFormat == YUV420) path << "420_" << bitDepth << "b_f" << frameCounter;
	else if (yuvFormat == YUV444) path << "444_" << bitDepth << "b_f" << frameCounter;
	else exit(1);
	return path.str();
}


