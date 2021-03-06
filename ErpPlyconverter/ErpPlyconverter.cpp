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


void CameraToWorld(pcc::PCCPoint3D &point3D, pcc::PCCPoint3D T, pcc::PCCPoint3D R);
void ErpToPointCloud(CERPParameter &para);
void ErpToPointCloud(CErpFrame &view, PCCPointSet3 &pointCloud);
void PointCloudToErp(CERPParameter &para);
void PointCloudToErp(PCCPointSet3 &pointCloud, CErpFrame &view);
void PlaneToPointCloud(CERPParameter &para);
void PlaneToPointCloud(CErpFrame &view, PCCPointSet3 &pointCloud);
void DirctToPointCloud(CERPParameter &para);
void DirctToPointCloud(CErpFrame &view, PCCPointSet3 &pointCloud);
void YUVdownsample(CERPParameter &para);
std::string ToString(uint16_t W, uint16_t H, double Rnear, double Rfar, YUVformat yuvFormat, uint16_t bitDepth, double stepSize, pcc::PCCBox3D box3D, size_t frameCounter);
void ParseDepthAndTextureFileName(std::string textureFileName, std::string depthFileName, ErpPara &erpPara);
void ParsePlyName(std::string PLYPath, ErpPara &erpPara, PCCBox3D &box3D, double &stepSize);



int main(int argc, char *argv[]) 
{
	CERPParameter para;
	para.Parse(argc, argv);
	para.PrintParas();
	if (para.mode == 0) //ERP转点云
	{
		ErpToPointCloud(para);
	}
	else if (para.mode == 1) //点云转ERP
	{
		PointCloudToErp(para);
	}
	else if (para.mode == 2) //plane3D转点云
	{
		PlaneToPointCloud(para);
	}
	else if (para.mode == 3) //直接转点云(不利用3D关系)
	{
		DirctToPointCloud(para);
	}
	else if (para.mode == 4) //YUV降分辨率
	{
		YUVdownsample(para);
	}

	return 0;
}


void CameraToWorld(pcc::PCCPoint3D &point3D, pcc::PCCPoint3D T, pcc::PCCPoint3D R)
{
	R = -R * PI / 180;

	double x1 = point3D[0] * cos(R[1]) + point3D[2] * (-(sin(R[1])));
	double y1 = point3D[1];
	double z1 = point3D[0] * sin(R[1]) + point3D[2] * cos(R[1]);

	double x2 = x1 * cos(R[0]) + y1 * sin(R[0]);
	double y2 = x1 * (-sin(R[0])) + y1 * cos(R[0]);
	double z2 = z1;

	point3D[0] = x2;
	point3D[1] = y2 * cos(R[2]) + z2 * sin(R[2]);
	point3D[2] = y2 * (-sin(R[2])) + z2 * cos(R[2]);
	point3D = point3D + T;
}

void ErpToPointCloud(CERPParameter &para)
{
	for (size_t i = 0; i < para.frameNumber; i++) //帧数
	{
		printf("正在转换第%d帧......\n", i + 1);

		PCCPointSet3 synPointCloud;
		ErpPara erpPara;

		for (int j = 0; j < para.viewNumber; j++) //每帧视点数
		{
			printf("\t正在合并第%d个视点......\n", j + 1);
			ParseDepthAndTextureFileName(para.textureFileNames[j], para.depthFileNames[j], erpPara);

			CErpFrame view(erpPara.viewName, erpPara.W, erpPara.H, erpPara.Rnear, erpPara.Rfar,
				erpPara.depthYUVFormat, erpPara.depthbitDepth, erpPara.textureYUVFormat,
				erpPara.texturebitDepth, para.T[j], para.R[j]);

			view.read(para.textureFileNames[j], para.depthFileNames[j], i);
			view.textureYUV420ToYUV444();
			PCCPointSet3 pointCloud;
			ErpToPointCloud(view, pointCloud);
			synPointCloud.addPoints(pointCloud);
		}

		synPointCloud.Voxelize(para.pointCloudGeoBitDepth);
		synPointCloud.ConvertColorFrom10bTo8b();
		synPointCloud.convertYUVToRGB();

		std::string synPointCloudName = para.outputPlyPath + "\\syn_" +
			ToString(erpPara.W, erpPara.H, erpPara.Rnear, erpPara.Rfar, erpPara.depthYUVFormat,
				erpPara.depthbitDepth, synPointCloud.getStepSize(), synPointCloud.getBox3D(), i) + ".ply";
		printf("\t正在将点云文件写入磁盘......\n");
		synPointCloud.write(synPointCloudName, true);
	}
}

void ErpToPointCloud(CErpFrame &view, PCCPointSet3 &pointCloud)
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
	if (!pointCloud.hasColors()) pointCloud.addColors();
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
			
				//变化坐标系（相机坐标 ---> 世界坐标）
				CameraToWorld(point3D, T, Rotation);

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

void PointCloudToErp(PCCPointSet3 &pointCloud, CErpFrame &view)
{
	uint16_t W = view.getWidth();
	uint16_t H = view.getHeight();
	long vmax = view.getVmax();
	double Rnear = view.getRnear();
	double Rfar = view.getRfar();
	auto T = view.getT();
	YUV* depth = view.getDepth();
	YUV* texture = view.getTexture();
	double stepSize = pointCloud.getStepSize();
	PCCBox3D box3D = pointCloud.getBox3D();
	double X, Y, Z, R, phi, theta;
	int n, m;
	std::vector<std::vector<double>> nearest(H, std::vector<double>(W, INF));
	const size_t pointCount = pointCloud.getPointCount();
	for (int i = 0; i < pointCount; i++)
	{
		PCCPoint3D position = pointCloud[i];
		X = position.x() - T[0];
		Y = position.y() - T[1];
		Z = position.z() - T[2];

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

void PointCloudToErp(CERPParameter &para)
{
	for (size_t i = 0; i < para.frameNumber; i++) //帧数
	{
		printf("正在转换第%d帧......\n", i + 1);

		PCCBox3D box3D;
		double stepSize;

		ErpPara erpPara;

		pcc::PCCVector3<double> T;
		pcc::PCCVector3<double> R;
		PCCPointSet3 pointCloud(true);


		ParsePlyName(para.plyFileNames[i], erpPara, box3D, stepSize);
		pointCloud.read(para.plyFileNames[i]);
		pointCloud.setVoxelizationParas(box3D, stepSize);
		pointCloud.convertRGBToYUV();
		pointCloud.ConvertColorFrom8bTo10b();
		pointCloud.Devoxelize();

		for (int j = 0; j < para.viewNumber; j++)
		{
			printf("正在生成第%d个视点：%s......\n", j + 1, para.viewNames[j].c_str());
			CErpFrame view(para.viewNames[j], erpPara.W, erpPara.H, erpPara.Rnear, erpPara.Rfar, erpPara.depthYUVFormat, erpPara.depthbitDepth,
				erpPara.textureYUVFormat, erpPara.texturebitDepth, para.T[j], para.R[j]);
			PointCloudToErp(pointCloud, view);
			view.write(para.outputRecYUVPath, para.outputRecYUVPath);
		}
	}
}

void PlaneToPointCloud(CERPParameter &para)
{
	for (size_t i = 0; i < para.frameNumber; i++) //帧数
	{
		printf("正在转换第%d帧......\n", i + 1);

		PCCPointSet3 synPointCloud;
		uint16_t W;
		uint16_t H;
		double Rnear;
		double Rfar;
		YUVformat yuvFormat;
		uint16_t bitDepth;

		for (int j = 0; j < para.viewNumber; j++) //每帧视点数
		{
			printf("\t正在合并第%d个视点......\n", j + 1);
			CPlaneFrame view(para.viewNames[j], para.widths[j], para.heights[j], para.Rnears[j], para.Rfars[j], para.depthYUVFormats[j], para.depthbitDepths[j], para.textureYUVFormats[j], para.texturebitDepths[j], para.T[j], para.R[j], para.deltaXs[j], para.deltaYs[j], para.fxs[j], para.fys[j]);

			view.read(para.textureFileNames[j], para.depthFileNames[j], i);
			view.textureYUV420ToYUV444();
			if (!j)
			{
				W = view.getWidth();
				H = view.getHeight();
				Rnear = view.getRnear();
				Rfar = view.getRfar();
				yuvFormat = view.getDepthYUVFormat();
				bitDepth = view.getDepthbitDepth();
			}
			PCCPointSet3 pointCloud;
			PlaneToPointCloud(view, pointCloud);
			synPointCloud.addPoints(pointCloud);
		}
		synPointCloud.Voxelize(para.pointCloudGeoBitDepth);
		synPointCloud.ConvertColorFrom10bTo8b();
		synPointCloud.convertYUVToRGB();

		std::string synPointCloudName = para.outputPlyPath + "\\syn_" + ToString(W, H, Rnear, Rfar, yuvFormat, bitDepth, synPointCloud.getStepSize(), synPointCloud.getBox3D(), i) + ".ply";
		synPointCloud.write(synPointCloudName, true);

	}
}

void PlaneToPointCloud(CErpFrame &view, PCCPointSet3 &pointCloud)
{
	auto W = view.getWidth();
	auto H = view.getHeight();

	long vmax = view.getVmax();
	double Rnear = view.getRnear();
	double Rfar = view.getRfar();

	pcc::PCCPoint3D T = view.getT();
	pcc::PCCPoint3D Rotation = view.getR();

	auto depth = view.getDepth();
	auto texture = view.getTexture();

	double deltaX = view.getdeltaX();
	double deltaY = view.getdeltaY();
	double fx = view.getfx();
	double fy = view.getfy();

	pcc::PCCPoint3D point3D;
	pcc::PCCColor3B Color3B;
	uint16_t Depth;
	size_t piontCount = pointCloud.getPointCount();
	pointCloud.resize(piontCount + W * H);
	if (!pointCloud.hasColors()) pointCloud.addColors();
	for (size_t m = 0; m < H; m++)
	{
		for (size_t n = 0; n < W; n++)
		{
			Depth = (*depth)[0][m][n];

			if (Depth != 0)
			{
				//计算三维坐标
				point3D[2] = vmax * Rnear*Rfar / (Depth*(Rfar - Rnear) + vmax * Rnear);
				point3D[0] = point3D[2] * (n - deltaX) / fx;
				point3D[1] = point3D[2] * (m - deltaY) / fy;
	
				//变化坐标（相机坐标 ---> 世界坐标）
				//CameraToWorld(point3D, T, Rotation);

				//获得纹理
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

void DirctToPointCloud(CERPParameter &para)
{
	for (size_t i = 0; i < para.frameNumber; i++) //帧数
	{
		printf("正在转换第%d帧......\n", i + 1);

		PCCPointSet3 synPointCloud;
		uint16_t W;
		uint16_t H;
		double Rnear;
		double Rfar;
		YUVformat yuvFormat;
		uint16_t bitDepth;

		for (int j = 0; j < para.viewNumber; j++) //每帧视点数
		{
			printf("\t正在合并第%d个视点......\n", j + 1);
			CPlaneFrame view(para.viewNames[j], para.widths[j], para.heights[j], para.Rnears[j], para.Rfars[j], para.depthYUVFormats[j], para.depthbitDepths[j], para.textureYUVFormats[j], para.texturebitDepths[j], para.T[j], para.R[j], para.deltaXs[j], para.deltaYs[j], para.fxs[j], para.fys[j]);

			view.read(para.textureFileNames[j], para.depthFileNames[j], i);
			view.textureYUV420ToYUV444();
			if (!j)
			{
				W = view.getWidth();
				H = view.getHeight();
				Rnear = view.getRnear();
				Rfar = view.getRfar();
				yuvFormat = view.getDepthYUVFormat();
				bitDepth = view.getDepthbitDepth();
			}
			PCCPointSet3 pointCloud;
			DirctToPointCloud(view, pointCloud);
			synPointCloud.addPoints(pointCloud);
		}
		//synPointCloud.Voxelize(para.pointCloudGeoBitDepth);
		synPointCloud.ConvertColorFrom10bTo8b();
		synPointCloud.convertYUVToRGB();

		std::string synPointCloudName = para.outputPlyPath + "\\syn_" + ToString(W, H, Rnear, Rfar, yuvFormat, bitDepth, synPointCloud.getStepSize(), synPointCloud.getBox3D(), i) + ".ply";
		synPointCloud.write(synPointCloudName, true);

	}
}

void DirctToPointCloud(CErpFrame &view, PCCPointSet3 &pointCloud)
{
	auto W = view.getWidth();
	auto H = view.getHeight();

	long vmax = view.getVmax();
	double Rnear = view.getRnear();
	double Rfar = view.getRfar();

	pcc::PCCPoint3D T = view.getT();
	pcc::PCCPoint3D Rotation = view.getR();

	auto depth = view.getDepth();
	auto texture = view.getTexture();

	double deltaX = view.getdeltaX();
	double deltaY = view.getdeltaY();
	double fx = view.getfx();
	double fy = view.getfy();

	pcc::PCCPoint3D point3D;
	pcc::PCCColor3B Color3B;
	uint16_t Depth;
	size_t piontCount = pointCloud.getPointCount();
	pointCloud.resize(piontCount + W * H);
	if (!pointCloud.hasColors()) pointCloud.addColors();
	for (size_t m = 0; m < H; m++)
	{
		for (size_t n = 0; n < W; n++)
		{
			Depth = (*depth)[0][m][n];

			if (Depth != 0)
			{
				//计算三维坐标
				point3D[2] = Depth;
				point3D[0] = n;
				point3D[1] = m;

				//获得纹理
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

void YUVdownsample(CERPParameter &para)
{
	for (size_t i = 0; i < para.frameNumber; i++) //帧数
	{
		printf("正在转换第%d帧......\n", i + 1);

		PCCPointSet3 synPointCloud;

		ErpPara erpPara;

		for (int j = 0; j < para.viewNumber; j++) //每帧视点数
		{
			printf("\t正在合并第%d个视点......\n", j + 1);

			ParseDepthAndTextureFileName(para.textureFileNames[j], para.depthFileNames[j], erpPara);
			CErpFrame view(erpPara.viewName, erpPara.W, erpPara.H, erpPara.Rnear, erpPara.Rfar,
				erpPara.depthYUVFormat, erpPara.depthbitDepth, erpPara.textureYUVFormat,
				erpPara.texturebitDepth, para.T[j], para.R[j]);
			view.read(para.textureFileNames[j], para.depthFileNames[j], i);
			view.textureYUV420ToYUV444();
			view.downsample(4);
			view.textureYUV444ToYUV420();
			view.write(para.outputRecYUVPath, para.outputRecYUVPath);
		}
	}
}

std::string ToString(uint16_t W, uint16_t H, double Rnear, double Rfar, YUVformat yuvFormat,uint16_t bitDepth,double stepSize, pcc::PCCBox3D box3D,size_t frameCounter)
{
	std::stringstream path;
	std::stringstream tempstrRnear;
	std::stringstream tempstrRfar;
	std::stringstream tempstrStepSize;
	std::stringstream tempstrX;
	std::stringstream tempstrY;
	std::stringstream tempstrZ;

	tempstrRnear << std::fixed << std::setprecision(1) << Rnear;
	tempstrRfar << std::fixed << std::setprecision(1) << Rfar;
	tempstrStepSize << std::fixed << std::setprecision(15) << stepSize;
	tempstrX << std::fixed << std::setprecision(15) << box3D.min.x();
	tempstrY << std::fixed << std::setprecision(15) << box3D.min.y();
	tempstrZ << std::fixed << std::setprecision(15) << box3D.min.z();

	std::vector<std::string> tempstrRnearSplit = splitWithStl(tempstrRnear.str(), ".");
	std::vector<std::string> tempstrRfarSplit = splitWithStl(tempstrRfar.str(), ".");
	std::vector<std::string> tempstrStepSizeSplit = splitWithStl(tempstrStepSize.str(), ".");
	std::vector<std::string> tempstrXSplit = splitWithStl(tempstrX.str(), ".");
	std::vector<std::string> tempstrYSplit = splitWithStl(tempstrY.str(), ".");
	std::vector<std::string> tempstrZSplit = splitWithStl(tempstrZ.str(), ".");

	path << W << "_" << H << "_" << tempstrRnearSplit[0] << "_";
	path << tempstrRnearSplit[1] << "_" << tempstrRfarSplit[0] << "_" << tempstrRfarSplit[1] << "_";
	if (yuvFormat == YUV420) path << "420_" << bitDepth << "b_" << tempstrStepSizeSplit[0] << "_";
	else if(yuvFormat == YUV444) path << "444_" << bitDepth << "b_" << tempstrStepSizeSplit[0] << "_";
	else exit(1);
	path << tempstrStepSizeSplit[1] << "_" << tempstrXSplit[0] << "_" << tempstrXSplit[1] << "_";
	path << tempstrYSplit[0] << "_" << tempstrYSplit[1] << "_" << tempstrZSplit[0] << "_" << tempstrZSplit[1]<< "_"<<frameCounter;
	return path.str();
}

void ParsePlyName(std::string PLYPath, ErpPara &erpPara, PCCBox3D &box3D, double &stepSize)
{
	std::vector<std::string> PLYPathSplit = splitWithStl(PLYPath, "\\");
	PLYPathSplit = splitWithStl(PLYPathSplit.back(), "_");
	assert(PLYPathSplit.size() == 18);
	erpPara.W = atoi(PLYPathSplit[1].c_str());
	erpPara.H = atoi(PLYPathSplit[2].c_str());
	erpPara.Rnear = atof((PLYPathSplit[3] + "." + PLYPathSplit[4]).c_str());
	erpPara.Rfar = atof((PLYPathSplit[5] + "." + PLYPathSplit[6]).c_str());
	stepSize = atof((PLYPathSplit[9] + "." + PLYPathSplit[10]).c_str());
	double X = atof((PLYPathSplit[11] + "." + PLYPathSplit[12]).c_str());
	double Y = atof((PLYPathSplit[13] + "." + PLYPathSplit[14]).c_str());
	double Z = atof((PLYPathSplit[15] + "." + PLYPathSplit[16]).c_str());
	box3D.min.SetXYZ(X, Y, Z);
	if (PLYPathSplit[7] == "420") erpPara.textureYUVFormat = YUV420, erpPara.depthYUVFormat = YUV420;
	else if(PLYPathSplit[7] == "444") erpPara.textureYUVFormat = YUV444, erpPara.depthYUVFormat = YUV444;
	else exit(1);
	 
	std::vector<std::string> PLYPathSplit16Split = splitWithStl(PLYPathSplit[8], "b");
	assert(PLYPathSplit16Split.size() == 2);
	erpPara.texturebitDepth = atoi(PLYPathSplit16Split[0].c_str()), erpPara.depthbitDepth = atoi(PLYPathSplit16Split[0].c_str());
}

void ParseDepthAndTextureFileName(std::string textureFileName, std::string depthFileName, ErpPara &erpPara)
{
	std::vector<std::string> depthFilePathSplit = splitWithStl(depthFileName, "\\");
	depthFilePathSplit = splitWithStl(depthFilePathSplit.back(), "_");
	assert(depthFilePathSplit.size() == 8);
	erpPara.viewName = depthFilePathSplit[0];
	std::vector<std::string> depthFilePathSplit1Split = splitWithStl(depthFilePathSplit[1], "x");
	assert(depthFilePathSplit1Split.size() == 2);
	erpPara.W = atoi(depthFilePathSplit1Split[0].c_str());
	erpPara.H = atoi(depthFilePathSplit1Split[1].c_str());

	erpPara.Rnear = atof((depthFilePathSplit[2] + "." + depthFilePathSplit[3]).c_str());
	erpPara.Rfar = atof((depthFilePathSplit[4] + "." + depthFilePathSplit[5]).c_str());
	if (depthFilePathSplit[6] == "420") erpPara.depthYUVFormat = YUV420;
	else if (depthFilePathSplit[6] == "444") erpPara.depthYUVFormat = YUV444;
	else
	{
		std::cout << "Can not handle such format YUV File for now!" << std::endl;
		exit(1);
	}

	std::vector<std::string> depthFilePathSplit7Split = splitWithStl(depthFilePathSplit[7], "b");
	assert(depthFilePathSplit7Split.size() == 2);
	erpPara.depthbitDepth = atoi(depthFilePathSplit7Split[0].c_str());

	
	std::vector<std::string> textureFilePathSplit = splitWithStl(textureFileName, "\\");
	textureFilePathSplit = splitWithStl(textureFilePathSplit.back(), "_");
	assert(textureFilePathSplit.size() == 4);
	if (erpPara.viewName != textureFilePathSplit[0])
	{
		std::cout << "textureFile:" << textureFileName << std::endl;
		std::cout << "depthFile:" << depthFileName << std::endl;
		std::cout << "texture file and depth file do not match!" << std::endl;
	}
	std::vector<std::string> textureFilePathSplit1Split = splitWithStl(textureFilePathSplit[1], "x");
	assert(textureFilePathSplit1Split.size() == 2);
	if (erpPara.W != atoi(textureFilePathSplit1Split[0].c_str()) || erpPara.H != atoi(textureFilePathSplit1Split[1].c_str()))
	{
		std::cout << "textureFile:" << textureFileName << std::endl;
		std::cout << "depthFile:" << depthFileName << std::endl;
		std::cout << "texture file and depth file do not match!" << std::endl;
	}
	if (textureFilePathSplit[2] == "420") erpPara.textureYUVFormat = YUV420;
	else if (textureFilePathSplit[2] == "444") erpPara.textureYUVFormat = YUV444;
	else
	{
		std::cout << "Can not handle such format YUV File for now!" << std::endl;
		exit(1);
	}

	std::vector<std::string> textureFilePathSplit3Split = splitWithStl(textureFilePathSplit[3], "b");
	assert(textureFilePathSplit3Split.size() == 2);
	erpPara.texturebitDepth = atoi(textureFilePathSplit3Split[0].c_str());
	return;
}

