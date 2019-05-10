#ifndef erp_h
#define erp_h

#include <vector>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <string>
#include <stdlib.h>
#include <sstream>
#include <math.h>
#include <assert.h>
#include "PCCMath.h"
#include "PCCMisc.h"

namespace erp 
{
#define PI (3.1415926)
#define EPS (1e-15)
#define INF (100000000000000000)
	std::vector<std::string> splitWithStl(const std::string &str, const std::string &pattern)
	{
		std::vector<std::string> resVec;

		if ("" == str)
		{
			return resVec;
		}
		//方便截取最后一段数据
		std::string strs = str + pattern;

		size_t pos = strs.find(pattern);
		size_t size = strs.size();

		while (pos != std::string::npos)
		{
			std::string x = strs.substr(0, pos);
			resVec.push_back(x);
			strs = strs.substr(pos + 1, size);
			pos = strs.find(pattern);
		}

		return resVec;
	}

	enum YUVformat
	{
		YUV444,
		YUV420
	};

	class CERPParameter
	{
	public:
		//公共参数
		int mode; //模式（需输入参数）
		size_t frameNumber; //帧数（需输入参数）
		std::vector<pcc::PCCVector3<double>> T; //各个视点平移向量，由ERP配置文件解析得到（解析参数）
		std::vector<pcc::PCCVector3<double>> R; //各个视点旋转矩阵，由ERP配置文件解析得到（解析参数）

		//ERP转点云参数 mode=0;
		uint8_t pointCloudGeoBitDepth;//点云几何位深（需输入参数）
		std::string outputPlyPath; //输出点云文件路径，不包含文件名（需输入参数）
		std::string inputErpCfgPath; //输入ERP配置文件路径，包含文件名（需输入参数）

		size_t viewNumber;//视点数量，由ERP配置文件解析得到（解析参数）
		std::vector<std::string> depthFileNames; //各个视点深度文件名，由ERP配置文件解析得到（解析参数）
		std::vector<std::string> textureFileNames; //各个视点纹理文件名，由ERP配置文件解析得到（解析参数）
		


		//点云转ERP参数 mode=1
		std::string inputPointCloudCfgPath;//输入点云列表配置文件路径，包含文件名（需输入参数）
		std::string outputRecYUVPath; //重建YUV文件路径，不包含文件名（需输入参数）

		std::vector<std::string> plyFileNames; //点云文件名，由点云列表配置文件解析得到（解析参数）

		//plane转点云参数 mode=2
		std::string inputPlaneCfgPath; //输入plane配置文件路径，包含文件名（需输入参数）

		//view 参数
		std::vector<std::string> viewNames;
		std::vector<uint16_t> widths;
		std::vector<uint16_t> heights;
		//纹理图信息
		std::vector<uint16_t> texturebitDepths;
		std::vector<YUVformat> textureYUVFormats;
		//深度图信息
		std::vector<double> Rnears;
		std::vector<double> Rfars;
		std::vector<YUVformat> depthYUVFormats;
		std::vector<uint16_t> depthbitDepths;
		std::vector<long> vmaxs;

		std::vector<double> deltaXs;
		std::vector<double> deltaYs;
		std::vector<double> fxs;
		std::vector<double> fys;

		static bool compareSeparators(char aChar, const char *const sep) {
			int i = 0;
			while (sep[i] != '\0') {
				if (aChar == sep[i]) return false;
				i++;
			}
			return true;
		}

		static inline bool getTokens(const char *str, const char *const sep,
			std::vector<std::string> &tokens) {
			if (!tokens.empty()) tokens.clear();
			std::string buf = "";
			size_t i = 0;
			size_t length = ::strlen(str);
			while (i < length) {
				if (compareSeparators(str[i], sep)) {
					buf += str[i];
				}
				else if (buf.length() > 0) {
					tokens.push_back(buf);
					buf = "";
				}
				i++;
			}
			if (!buf.empty()) tokens.push_back(buf);
			return !tokens.empty();
		}

		bool ParseErpCfgFile()
		{
			std::ifstream ifs(inputErpCfgPath, std::ifstream::in);
			if (!ifs.is_open()) {
				std::cout << "Can not open \"inputErpCfgPath:" << inputErpCfgPath << "\"!" << std::endl;
				exit(1);
			}

			const size_t MAX_BUFFER_SIZE = 4096;
			char tmp[MAX_BUFFER_SIZE];
			const char *sep = " \t\r";
			std::vector<std::string> tokens;

			ifs.getline(tmp, MAX_BUFFER_SIZE);
			getTokens(tmp, sep, tokens);
			if (tokens.empty() || tokens[0] != "cfg") {
				std::cout << "Error: corrupted file!" << std::endl;
				return false;
			}
			bool isAscii = false;
			double version = 1.0;
			bool isViewProperty = true;
			size_t attributeCount = 0;
			while (1) {
				if (ifs.eof()) {
					std::cout << "Error: corrupted header!" << std::endl;
					return false;
				}
				ifs.getline(tmp, MAX_BUFFER_SIZE);
				getTokens(tmp, sep, tokens);
				if (tokens.empty() || tokens[0] == "comment") {
					continue;
				}
				if (tokens[0] == "format") {
					if (tokens.size() != 3) {
						std::cout << "Error: corrupted format info!" << std::endl;
						return false;
					}
					isAscii = tokens[1] == "ascii";
					version = atof(tokens[2].c_str());
				}
				else if (tokens[0] == "element") {
					if (tokens.size() != 3) {
						std::cout << "Error: corrupted element info!" << std::endl;
						return false;
					}
					if (tokens[1] == "view") {
						viewNumber = atoi(tokens[2].c_str());
					}
					else {
						isViewProperty = false;
					}
				}
				else if (tokens[0] == "property" && isViewProperty) {
					attributeCount++;

				}
				else if (tokens[0] == "end_header") {
					break;
				}
			}
			if (version != 1.0) {
				std::cout << "Error: non-supported version!" << std::endl;
				return false;
			}
			assert(mode == 0 && attributeCount == 16 || mode == 1 && attributeCount == 6);
			if (isAscii) {
				size_t viewCounter = 0;
				while (!ifs.eof() && viewCounter < viewNumber) {
					ifs.getline(tmp, MAX_BUFFER_SIZE);
					getTokens(tmp, sep, tokens);
					if (tokens.empty()) {
						continue;
					}
					if (tokens.size() < attributeCount) {
						return false;
					}
					if (attributeCount == 16)
					{
						depthFileNames.push_back(tokens[0]);
						textureFileNames.push_back(tokens[1]);
						T.push_back(pcc::PCCVector3D(atof(tokens[2].c_str()), atof(tokens[3].c_str()), atof(tokens[4].c_str())));
						R.push_back(pcc::PCCVector3D(atof(tokens[5].c_str()), atof(tokens[6].c_str()), atof(tokens[7].c_str())));
						viewNames.push_back(tokens[8]);
						widths.push_back(atoi(tokens[9].c_str()));
						heights.push_back(atoi(tokens[10].c_str()));
						if (tokens[11] == "420")
						{
							textureYUVFormats.push_back(YUV420);
							depthYUVFormats.push_back(YUV420);
						}
						else
						{
							std::cout << "Unknown YUV format!" << std::endl;
						}
						depthbitDepths.push_back(atoi(tokens[12].c_str()));
						texturebitDepths.push_back(atoi(tokens[13].c_str()));
						Rnears.push_back(atof(tokens[14].c_str()));
						Rfars.push_back(atof(tokens[15].c_str()));
					}
					else
					{
						T.push_back(pcc::PCCVector3D(atof(tokens[0].c_str()), atof(tokens[1].c_str()), atof(tokens[2].c_str())));
						R.push_back(pcc::PCCVector3D(atof(tokens[3].c_str()), atof(tokens[4].c_str()), atof(tokens[5].c_str())));
					}
					viewCounter++;
				}
			}
			return true;
		}
		bool ParsePointCloudCfgFile()
		{
			std::ifstream ifs(inputPointCloudCfgPath, std::ifstream::in);
			if (!ifs.is_open()) {
				std::cout << "Can not open \"inputPointCloudCfgPath:" << inputPointCloudCfgPath << "\"!" << std::endl;
				exit(1);
			}

			const size_t MAX_BUFFER_SIZE = 4096;
			char tmp[MAX_BUFFER_SIZE];
			const char *sep = " \t\r";
			std::vector<std::string> tokens;

			ifs.getline(tmp, MAX_BUFFER_SIZE);
			getTokens(tmp, sep, tokens);
			if (tokens.empty() || tokens[0] != "cfg") {
				std::cout << "Error: corrupted file!" << std::endl;
				return false;
			}
			bool isAscii = false;
			double version = 1.0;
			bool isViewProperty = false;
			bool isPointCloudProperty = false;
			size_t PointCloudAttributeCount = 0;
			size_t viewAttributeCount = 0;
			while (1) {
				if (ifs.eof()) {
					std::cout << "Error: corrupted header!" << std::endl;
					return false;
				}
				ifs.getline(tmp, MAX_BUFFER_SIZE);
				getTokens(tmp, sep, tokens);
				if (tokens.empty() || tokens[0] == "comment") {
					continue;
				}
				if (tokens[0] == "format") {
					if (tokens.size() != 3) {
						std::cout << "Error: corrupted format info!" << std::endl;
						return false;
					}
					isAscii = tokens[1] == "ascii";
					version = atof(tokens[2].c_str());
				}
				else if (tokens[0] == "element") {

					if (tokens.size() != 3) {
						std::cout << "Error: corrupted element info!" << std::endl;
						return false;
					}
					if (tokens[1] == "view") {
						viewNumber = atoi(tokens[2].c_str());
						isViewProperty = true;
						isPointCloudProperty = false;
					}
					else if (tokens[1] == "pointCloud")
					{
						frameNumber= atoi(tokens[2].c_str());
						isViewProperty = false;
						isPointCloudProperty = true;
					}
					else {
						isViewProperty = false;
						isPointCloudProperty = false;
					}
				}
				else if (tokens[0] == "property" && isViewProperty) {
					viewAttributeCount++;

				}
				else if (tokens[0] == "property" && isPointCloudProperty) {
					PointCloudAttributeCount++;

				}
				else if (tokens[0] == "end_header") {
					break;
				}
			}
			if (version != 1.0) {
				std::cout << "Error: non-supported version!" << std::endl;
				return false;
			}
			
			assert(PointCloudAttributeCount == 1);
			size_t pointCloudCounter = 0;
			while (!ifs.eof() && pointCloudCounter < frameNumber) {
				ifs.getline(tmp, MAX_BUFFER_SIZE);
				getTokens(tmp, sep, tokens);
				if (tokens.empty()) {
					continue;
				}
				if (tokens.size() < PointCloudAttributeCount) {
					return false;
				}

				plyFileNames.push_back(tokens[0]);
				pointCloudCounter++;
			}

			if (isAscii) {
				assert(viewAttributeCount == 14);
				size_t viewCounter = 0;
				while (!ifs.eof() && viewCounter < viewNumber) {
					ifs.getline(tmp, MAX_BUFFER_SIZE);
					getTokens(tmp, sep, tokens);
					if (tokens.empty()) {
						continue;
					}
					if (tokens.size() < viewAttributeCount) {
						return false;
					}

					viewNames.push_back(tokens[0].c_str());
					T.push_back(pcc::PCCVector3D(atof(tokens[1].c_str()), atof(tokens[2].c_str()), atof(tokens[3].c_str())));
					R.push_back(pcc::PCCVector3D(atof(tokens[4].c_str()), atof(tokens[5].c_str()), atof(tokens[6].c_str())));

					widths.push_back(atoi(tokens[7].c_str()));
					heights.push_back(atoi(tokens[8].c_str()));
					if (tokens[9] == "420")
					{
						textureYUVFormats.push_back(YUV420);
						depthYUVFormats.push_back(YUV420);
					}
					else
					{
						std::cout << "Unknown YUV format!" << std::endl;
					}
					depthbitDepths.push_back(atoi(tokens[10].c_str()));
					texturebitDepths.push_back(atoi(tokens[11].c_str()));
					Rnears.push_back(atof(tokens[12].c_str()));
					Rfars.push_back(atof(tokens[13].c_str()));
					viewCounter++;
				}
			}
			return true;
		}
		bool Parse(int argc, char* argv[])
		{
			// -mode 0 -frameNumber 1 -pointCloudGeoBitDepth 16 -outputPlyPath PLY -inputErpCfgPath cfg\hijack-part.cfg
			// -mode 1 -inputPointCloudCfgPath cfg\PointCloud-all.cfg -outputRecYUVPath YUVrec
			for (int iArgIdx = 1; iArgIdx < argc; iArgIdx += 2)
			{
				if (strcmp("-help", argv[iArgIdx]) == 0) { PrintHelp(); exit(1); }
				else if (strcmp("-mode", argv[iArgIdx]) == 0) mode = atoi(argv[iArgIdx + 1]);
				else if (strcmp("-frameNumber", argv[iArgIdx]) == 0) frameNumber = atoi(argv[iArgIdx + 1]);
				else if (strcmp("-pointCloudGeoBitDepth", argv[iArgIdx]) == 0) pointCloudGeoBitDepth = atoi(argv[iArgIdx + 1]);
				else if (strcmp("-outputPlyPath", argv[iArgIdx]) == 0) outputPlyPath = argv[iArgIdx + 1];
				else if (strcmp("-inputErpCfgPath", argv[iArgIdx]) == 0) inputErpCfgPath = argv[iArgIdx + 1];
				else if (strcmp("-inputPointCloudCfgPath", argv[iArgIdx]) == 0)	inputPointCloudCfgPath = argv[iArgIdx + 1];
				else if (strcmp("-outputRecYUVPath", argv[iArgIdx]) == 0) outputRecYUVPath = argv[iArgIdx + 1];
				else if (strcmp("-inputPlaneCfgPath", argv[iArgIdx]) == 0) inputPlaneCfgPath = argv[iArgIdx + 1];
			}

			if (mode == 0) ParseErpCfgFile();
			else if (mode == 1)	ParsePointCloudCfgFile();
			else exit(1);
		}
		void PrintParas()
		{
			if (mode == 0)
			{
				printf("参数：\n");
				printf("\tmode:\t\t\t\t%d\n", mode);
				printf("\tframeNumber:\t\t\t%d\n", frameNumber);
				printf("\tpointCloudGeoBitDepth:\t\t%d\n", pointCloudGeoBitDepth);
				printf("\tinputErpCfgPath:\t\t%s\n", inputErpCfgPath.c_str());
				printf("\t\tviewNumber:\t\t\t%d\n", viewNumber);
				for (int i = 0; i < viewNumber; i++)
				{
					//printf("\t\t\t%s %s\n", depthFileNames[i].c_str(), textureFileNames[i].c_str());
				}
				printf("\toutputPlyPath:\t\t\t%s\n", outputPlyPath.c_str());
				
			}
			else if (mode == 1)
			{
				printf("参数：\n");
				printf("\tmode:\t\t\t\t%d\n", mode);
				printf("\tinputPointCloudCfgPath:\t\t%s\n", inputPointCloudCfgPath.c_str());
				printf("\t\tframeNumber:\t\t\t%d\n", frameNumber);
				for (int i = 0; i < frameNumber; i++)
				{
					//printf("\t\t\t%s\n", plyFileNames[i].c_str());
				}
				printf("\t\tviewNumber:\t\t\t%d\n", viewNumber);
				for (int i = 0; i < viewNumber; i++)
				{
					printf("\t\t\t%s\n", viewNames[i].c_str());
				}
				
				printf("\toutputRecYUVPath\t\t%s\n", outputRecYUVPath.c_str());
				
			}

			printf("\n\n");
		}
		void PrintHelp()
		{
			std::cout<< "--帮助" << std::endl;
			std::cout << "公共参数:" << std::endl;
			std::cout << std::setw(25) << std::setiosflags(std::ios::left) << "-mode" << "模式" << std::endl;
			std::cout << std::setw(25) << std::setiosflags(std::ios::left) << "-frameNumber" << "帧数" << std::endl;
			std::cout << "ERP转点云参数:" << std::endl;
			std::cout << std::setw(25) << std::setiosflags(std::ios::left) << "-pointCloudGeoBitDepth" << "点云几何位深" << std::endl;
			std::cout << std::setw(25) << std::setiosflags(std::ios::left) << "-outputPlyPath" << "输出点云文件路径，不包含文件名" << std::endl;
			std::cout << std::setw(25) << std::setiosflags(std::ios::left) << "-inputErpCfgPath" << "输入ERP配置文件路径，包含文件名" << std::endl;
			std::cout << "点云转ERP参数:" << std::endl;
			std::cout << std::setw(25) << std::setiosflags(std::ios::left) << "-inputPointCloudCfgPath" << "输入点云列表配置文件路径，包含文件名" << std::endl;
			std::cout << std::setw(25) << std::setiosflags(std::ios::left) << "-outputRecYUVPath" << "重建YUV文件输出路径，不包含文件名" << std::endl;
		}
	};
	

	class YUV
	{
	private:
		std::vector< std::vector <uint16_t> > *Y;
		std::vector< std::vector <uint16_t> > *U;
		std::vector< std::vector <uint16_t> > *V;
		uint16_t W;
		uint16_t H;
		uint8_t bitDepth;
		YUVformat cs;
	public:
		std::vector< std::vector <uint16_t> > &operator[](size_t i)
		{
			assert(i <= 2);
			switch (i)
			{
			case 0: return *Y;
			case 1: return *U;
			case 2: return *V;
			}
		}
		YUV()
		{
			W = 0; H = 0; bitDepth = 0;
			Y = NULL; U = NULL; V = NULL;
		}
		YUV(uint16_t pW, uint16_t pH, uint8_t pbitDepth, YUVformat pcs):W(pW),H(pH),bitDepth(pbitDepth),cs(pcs)
		{
			assert(bitDepth <= 16);
			uint16_t emptyVal= 1 << (bitDepth - 1);
			Y = new std::vector< std::vector <uint16_t> >(H, std::vector <uint16_t>(W, emptyVal));
			U = new std::vector< std::vector <uint16_t> >(H, std::vector <uint16_t>(W, emptyVal));
			V = new std::vector< std::vector <uint16_t> >(H, std::vector <uint16_t>(W, emptyVal));
		}
		~YUV()
		{
			if (Y) delete Y;
			if (U) delete U;
			if (V) delete V;
		}
		bool downsample(uint16_t div)
		{
			for (int i = 0; i < H / div; i++)
			{
				(*Y)[i] = (*Y)[i*div];
				(*U)[i] = (*U)[i*div];
				(*V)[i] = (*V)[i*div];
			}

			for (int i = 0; i < H; i++)
			{
				for (int j = 0; j < W / div; j++)
				{
					(*Y)[i][j] = (*Y)[i][j*div];
					(*U)[i][j] = (*U)[i][j*div];
					(*V)[i][j] = (*V)[i][j*div];
				}
			}
			W = W / div;
			H = H / div;
			return true;
		}
		void YUVRead(std::string fileName,size_t frameCounter)
		{
			assert(Y&&U&&V);
			if (cs == YUV420)
			{
				uint16_t *FileBuffer = (uint16_t*)malloc(H*W * sizeof(uint16_t) * 3 / 2);
				long long offset = H*W * 3 * frameCounter;
				
				FILE *Fp = fopen(fileName.c_str(), "rb");
				_fseeki64(Fp, offset, SEEK_SET);
				if (fread(FileBuffer, W*H * 3, 1, Fp) != 1)
				{
					printf("can't open the YUV File %s \n", fileName.c_str());
					exit(1);
				}
				fclose(Fp);

				for (int j = 0; j < H; j++)
				{
					for (int i = 0; i < W; i++)
					{
						int address_y = j * W + i;
						int address_u = H*W + (int)(j / 2)*(W / 2) + (int)(i / 2);
						int address_v = H*W * 5 / 4 + (int)(j / 2)*(W / 2) + (int)(i / 2);

						(*Y)[j][i] = FileBuffer[address_y];
						(*U)[j][i] = FileBuffer[address_u];
						(*V)[j][i] = FileBuffer[address_v];
						
					}
				}
				free(FileBuffer);
				return;
			}
			else
			{
				std::cout<<"Haven't relized reading such format YUV File!" << std::endl;
				exit(1);
			}
		}
		void YUVWrite(std::string fileName)
		{
			assert(Y&&U&&V);
			FILE *Fp = fopen(fileName.c_str(), "ab");

			for (int i = 0; i <H; i++)
			{
				for (int j = 0; j < W; j++)
				{
					fwrite(&((*Y)[i][j]), sizeof(uint16_t), 1, Fp);
				}
			}
			for (int i = 0; i < H; i += 2)
			{
				for (int j = 0; j < W; j += 2)
				{
					fwrite(&((*U)[i][j]), sizeof(uint16_t), 1, Fp);
				}
			}
			for (int i = 0; i < H; i += 2)
			{
				for (int j = 0; j < W; j += 2)
				{
					fwrite(&((*V)[i][j]), sizeof(uint16_t), 1, Fp);
				}
			}
			fclose(Fp);

			return;

		}
		bool YUV444To420(){return true;}
		bool YUV420To444(){return true;}
		void SetY(uint16_t m, uint16_t n, uint16_t val){assert(m < H&&n < W);(*Y)[m][n] = val;}
		void SetU(uint16_t m, uint16_t n, uint16_t val){assert(m < H&&n < W);(*U)[m][n] = val;}
		void SetV(uint16_t m, uint16_t n, uint16_t val){assert(m < H&&n < W);(*V)[m][n] = val;}
		uint16_t GetY(uint16_t m, uint16_t n) {return (*Y)[m][n];}
		uint16_t GetU(uint16_t m, uint16_t n) {return (*U)[m][n];}
		uint16_t GetV(uint16_t m, uint16_t n) {return (*V)[m][n];}
		
	};

	class CErpFrame
	{
	private:
		YUV *texture;
		YUV *depth;
		//共有信息
		std::string viewName;
		uint16_t width;
		uint16_t height;
		//纹理图信息
		uint16_t texturebitDepth;
		YUVformat textureYUVFormat;
		//深度图信息
		double Rnear;
		double Rfar;
		YUVformat depthYUVFormat;
		uint16_t depthbitDepth;
		long vmax;

		pcc::PCCVector3<double> T;
		pcc::PCCVector3<double> R;

		double deltaX;
		double deltaY;
		double fx;
		double fy;


	public:
		CErpFrame()
		{
			texture = NULL;
			depth = NULL;
			T = pcc::PCCVector3<double>(0.0, 0.0, 0.0);
			R = pcc::PCCVector3<double>(0.0, 0.0, 0.0);
		}
		CErpFrame(pcc::PCCVector3<double> pT, pcc::PCCVector3<double> pR) :T(pT), R(pR) { texture = NULL; depth = NULL; }
		CErpFrame(std::string pviewName, uint16_t pwidth, uint16_t pheight, double pRnear, double pRfar, YUVformat pdepthYUVFormat, uint8_t pdepthbitDepth,
			YUVformat ptextureYUVFormat, uint8_t ptexturebitDepth, pcc::PCCVector3<double> pT, pcc::PCCVector3<double> pR)
		{
			viewName = pviewName;
			width = pwidth;
			height = pheight;
			//纹理图信息
			texturebitDepth = ptexturebitDepth;
			textureYUVFormat = ptextureYUVFormat;
			//深度图信息
			Rnear = pRnear;
			Rfar = pRfar;
			depthYUVFormat = pdepthYUVFormat;
			depthbitDepth = pdepthbitDepth;
			T = pT;
			R = pR;
			if (texture) delete texture;
			if (depth) delete depth;
			texture = new YUV(width, height, texturebitDepth, textureYUVFormat);
			depth = new YUV(width, height, depthbitDepth, depthYUVFormat);
			vmax = (1 << depthbitDepth )- 1;
	
		}
		CErpFrame(std::string pviewName, uint16_t pwidth, uint16_t pheight, double pRnear, double pRfar, YUVformat pdepthYUVFormat, uint8_t pdepthbitDepth,
			YUVformat ptextureYUVFormat, uint8_t ptexturebitDepth, pcc::PCCVector3<double> pT, pcc::PCCVector3<double> pR,double pdeltaX,double pdeltaY,
			double pfx,double pfy)
		{
			viewName = pviewName;
			width = pwidth;
			height = pheight;
			//纹理图信息
			texturebitDepth = ptexturebitDepth;
			textureYUVFormat = ptextureYUVFormat;
			//深度图信息
			Rnear = pRnear;
			Rfar = pRfar;
			depthYUVFormat = pdepthYUVFormat;
			depthbitDepth = pdepthbitDepth;
			T = pT;
			R = pR;
			deltaX = pdeltaX;
			deltaY = pdeltaY;
			fx = pfx;
			fy = pfy;
			if (texture) delete texture;
			if (depth) delete depth;
			texture = new YUV(width, height, texturebitDepth, textureYUVFormat);
			depth = new YUV(width, height, depthbitDepth, depthYUVFormat);
			vmax = 1 << depthbitDepth - 1;
		}
		~CErpFrame()
		{
			if (texture) delete texture;
			if (depth) delete depth;
		}

		YUV *getTexture() { return texture; }
		YUV *getDepth() { return depth; }
		std::string &getViewName() { return viewName; }
		uint16_t &getWidth() { return width; }
		uint16_t &getHeight() { return height; }
		uint16_t &getTexturebitDepth() { return texturebitDepth; }
		YUVformat &getTextureYUVFormat() { return textureYUVFormat; }
		double &getRnear() { return Rnear; }
		double &getRfar() { return Rfar; }
		YUVformat &getDepthYUVFormat() { return depthYUVFormat; }
		uint16_t &getDepthbitDepth() { return depthbitDepth; }
		long &getVmax() { return vmax; }
		pcc::PCCVector3<double> &getT() { return T; }
		pcc::PCCVector3<double> &getR() { return R; }
		double &getdeltaX() { return deltaX; }
		double &getdeltaY() { return deltaY; }
		double &getfx() { return fx; }
		double &getfy() { return fy; }
		bool downsample(uint16_t div)
		{
			texture->downsample(div);
			depth->downsample(div);
			width = width / div;
			height = height / div;
			return true;
		}

		void setTAndR(pcc::PCCVector3<double> pT, pcc::PCCVector3<double> pR) { T = pT; R = pR; }
		void textureYUV420ToYUV444()
		{
			assert(texture&&depth&&textureYUVFormat == YUV420);
			texture->YUV420To444();
		}
		void textureYUV444ToYUV420()
		{
			assert(texture&&depth&&textureYUVFormat == YUV444);
			texture->YUV444To420();
		}
		void read(std::string textureFileName, std::string depthFileName, size_t frameCounter)
		{
			if (texture) delete texture;
			if (depth) delete depth;
			texture = new YUV(width, height, texturebitDepth, textureYUVFormat);
			depth = new YUV(width, height, depthbitDepth, depthYUVFormat);
			texture->YUVRead(textureFileName, frameCounter);
			depth->YUVRead(depthFileName, frameCounter);
			return;
		}
		void write(std::string textureFilePath, std::string depthFilePath)
		{
			assert(texture&&depth);
			std::stringstream textureFileName;
			textureFileName << viewName << "_" << width << "x" << height << "_";
			if (textureYUVFormat == YUV420) textureFileName << "420_" << texturebitDepth << "b.yuv";
			else if (textureYUVFormat == YUV444) textureFileName << "444_" << texturebitDepth << "b.yuv";
			else
			{
				std::cout << "Can not handle such format YUV File for now!" << std::endl;
				exit(1);
			}


			std::stringstream depthFileName;
			std::stringstream tempstrRnear;
			std::stringstream tempstrRfar;
			tempstrRnear << std::fixed << std::setprecision(1) << Rnear;
			tempstrRfar << std::fixed << std::setprecision(1) << Rfar;

			std::vector<std::string> tempstrRnearSplit = splitWithStl(tempstrRnear.str(), ".");
			std::vector<std::string> tempstrRfarSplit = splitWithStl(tempstrRfar.str(), ".");

			depthFileName << viewName << "_" << width << "x" << height << "_" << tempstrRnearSplit[0] << "_";
			depthFileName << tempstrRnearSplit[1] << "_" << tempstrRfarSplit[0] << "_" << tempstrRfarSplit[1] << "_";
			if (depthYUVFormat == YUV420) depthFileName << "420_" << depthbitDepth << "b.yuv";
			else if (depthYUVFormat == YUV444) depthFileName << "444_" << depthbitDepth << "b.yuv";
			else
			{
				std::cout << "Can not handle such format YUV File for now!" << std::endl;
				exit(1);
			}

			texture->YUVWrite(textureFilePath + "\\" + textureFileName.str());
			depth->YUVWrite(depthFilePath + "\\" + depthFileName.str());
			return;
		}
	};

	struct ErpPara
	{
		std::string viewName;
		uint16_t W;
		uint16_t H;
		double Rnear;
		double Rfar;
		YUVformat depthYUVFormat;
		YUVformat textureYUVFormat;
		uint16_t depthbitDepth;
		uint16_t texturebitDepth;
	};
	typedef CErpFrame CPlaneFrame;
	
	int sign(double Z)
	{
		return (Z > 0 ? 1 : -1);
	}

}

#endif // !erp_h

