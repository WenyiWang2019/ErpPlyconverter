/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * <OWNER> = Apple Inc.
 * <ORGANIZATION> = Apple Inc.
 * <YEAR> = 2017
 *
 * Copyright (c) 2017, Apple Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *  * Neither the name of the <ORGANIZATION> nor the names of its contributors may
 *    be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef PCCPointSet_h
#define PCCPointSet_h

#include <assert.h>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#include "PCCMath.h"
#include "PCCMisc.h"
#include <thread>
#include <algorithm>

namespace pcc {
#define PI (3.1415926)
#define EPS (1e-15)
#define INF (100000000000000000)

	class PCCPoint
	{
	public:
		PCCPoint() {}
		~PCCPoint() = default;
		PCCPoint3D getPosition() const
		{
			return position;
		}
		PCCColor3B getColor() const
		{
			return color;
		}

		bool operator<(const PCCPoint &rhs) const {
			return (*this).position < rhs.position;
		}

		PCCPoint3D &getPosition()
		{
			return position;
		}
		PCCColor3B &getColor()
		{
			return color;
		}
	private:
		PCCPoint3D position;
		PCCColor3B color;
	};

	class PCCPointSet3 {
	 public:
	  PCCPointSet3() {}
	  PCCPointSet3(const PCCPointSet3 &) = default;
	  PCCPointSet3 &operator=(const PCCPointSet3 &rhs) = default;
	  ~PCCPointSet3() = default;
		  
	  PCCPoint3D operator[](const size_t index) const {
		  assert(index < points.size());
		return points[index].getPosition();
	  }
	  PCCPoint3D &operator[](const size_t index) {
		assert(index < points.size());
		return points[index].getPosition();
	  }

	  void setPosition(const size_t index, const PCCPoint3D position) {
		assert(index < points.size());
		points[index].getPosition() = position;
	  }
	  PCCColor3B getColor(const size_t index) const {
		assert(index < points.size());
		return points[index].getColor();
	  }
	  PCCColor3B &getColor(const size_t index) {
		assert(index < points.size());
		return points[index].getColor();
	  }
	  void setColor(const size_t index, const PCCColor3B color) {
		assert(index < points.size());
		points[index].getColor() = color;
	  }
	 
	  size_t getPointCount() const { return points.size(); }
	  void resize(const size_t size) {
		  points.resize(size);
	  }
	  void reserve(const size_t size) {
		  points.reserve(size);
	  }
	  void clear() {
		  points.clear();
	  }
	  size_t addPoint(const PCCPoint3D &position) {
		const size_t index = getPointCount();
		resize(index + 1);
		points[index].getPosition() = position;
		return index;
	  }
	  void setPoint(const size_t index, const PCCPoint3D &position) {
		  const size_t Pindex = getPointCount();
		  assert(index < Pindex);
		  points[index].getPosition() = position;
	  }
	  PCCPoint3D &getPosition(const size_t index) {
		  assert(index < points.size());
		  return points[index].getPosition();
	  }
	  void swapPoints(const size_t index1, const size_t index2) {
		assert(index1 < getPointCount());
		assert(index2 < getPointCount());
		std::swap(points[index1], points[index2]);
	  }

	  PCCPoint3D computeCentroid() const {
		PCCPoint3D bary(0.0);
		const size_t pointCount = getPointCount();
		if (pointCount) {
		  for (size_t i = 0; i < pointCount; ++i) {
			const PCCPoint3D &pt = (*this)[i];
			bary += pt;
		  }
		  bary /= double(pointCount);
		}
		return bary;
	  }
	  PCCBox3D computeBoundingBox() const {
		PCCBox3D bbox = {std::numeric_limits<double>::max(), std::numeric_limits<double>::lowest()};
		const size_t pointCount = getPointCount();
		for (size_t i = 0; i < pointCount; ++i) {
		  const PCCPoint3D &pt = (*this)[i];
		  for (int k = 0; k < 3; ++k) {
			if (pt[k] > bbox.max[k]) {
			  bbox.max[k] = pt[k];
			}
			if (pt[k] < bbox.min[k]) {
			  bbox.min[k] = pt[k];
			}
		  }
		}
		return bbox;
	  }
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
		  } else if (buf.length() > 0) {
			tokens.push_back(buf);
			buf = "";
		  }
		  i++;
		}
		if (!buf.empty()) tokens.push_back(buf);
		return !tokens.empty();
	  }
	  bool write(const std::string &fileName, const bool asAscii = false) {
		std::ofstream fout(fileName, std::ofstream::out);
		if (!fout.is_open()) {
		  return false;
		}
		const size_t pointCount = getPointCount();
		fout << "ply" << std::endl;

		if (asAscii) {
		  fout << "format ascii 1.0" << std::endl;
		} else {
		  PCCEndianness endianess = PCCSystemEndianness();
		  if (endianess == PCC_BIG_ENDIAN) {
			fout << "format binary_big_endian 1.0" << std::endl;
		  } else {
			fout << "format binary_little_endian 1.0" << std::endl;
		  }
		}
		fout << "element vertex " << pointCount << std::endl;
		fout << "property float64 x" << std::endl;
		fout << "property float64 y" << std::endl;
		fout << "property float64 z" << std::endl;
		fout << "property uchar red" << std::endl;
		fout << "property uchar green" << std::endl;
		fout << "property uchar blue" << std::endl;
	
		fout << "element face 0" << std::endl;
		fout << "property list uint8 int32 vertex_index" << std::endl;
		fout << "end_header" << std::endl;
		if (asAscii) {
		  fout << std::setprecision(std::numeric_limits<double>::max_digits10);
		  for (size_t i = 0; i < pointCount; ++i) {
			const PCCPoint3D &position = (*this)[i];
			fout << position.x() << " " << position.y() << " " << position.z();
			const PCCColor3B &color = getColor(i);
			fout << " " << static_cast<int>(color[0]) << " " << static_cast<int>(color[1]) << " "
				<< static_cast<int>(color[2]);
			fout << std::endl;
		  }
		} else {
		  fout.clear();
		  fout.close();
		  fout.open(fileName, std::ofstream::binary | std::ofstream::app);
		  for (size_t i = 0; i < pointCount; ++i) {
			const PCCPoint3D &position = (*this)[i];
			fout.write(reinterpret_cast<const char *const>(&position), sizeof(double) * 3);
			
			const PCCColor3B &color = getColor(i);
			fout.write(reinterpret_cast<const char *>(&color), sizeof(uint16_t) * 3);
			

		  }
		}
		fout.close();
		return true;
	  }
	  bool read(const std::string &fileName) {
		std::ifstream ifs(fileName, std::ifstream::in);
		if (!ifs.is_open()) {
		  return false;
		}
		enum AttributeType {
		  ATTRIBUTE_TYPE_FLOAT64 = 0,
		  ATTRIBUTE_TYPE_FLOAT32 = 1,
		  ATTRIBUTE_TYPE_UINT64 = 2,
		  ATTRIBUTE_TYPE_UINT32 = 3,
		  ATTRIBUTE_TYPE_UINT16 = 4,
		  ATTRIBUTE_TYPE_UINT8 = 5,
		  ATTRIBUTE_TYPE_INT64 = 6,
		  ATTRIBUTE_TYPE_INT32 = 7,
		  ATTRIBUTE_TYPE_INT16 = 8,
		  ATTRIBUTE_TYPE_INT8 = 9,
		};
		struct AttributeInfo {
		  std::string name;
		  AttributeType type;
		  size_t byteCount;
		};

		std::vector<AttributeInfo> attributesInfo;
		attributesInfo.reserve(16);
		const size_t MAX_BUFFER_SIZE = 4096;
		char tmp[MAX_BUFFER_SIZE];
		const char *sep = " \t\r";
		std::vector<std::string> tokens;

		ifs.getline(tmp, MAX_BUFFER_SIZE);
		getTokens(tmp, sep, tokens);
		if (tokens.empty() || tokens[0] != "ply") {
		  std::cout << "Error: corrupted file!" << std::endl;
		  return false;
		}
		bool isAscii = false;
		double version = 1.0;
		size_t pointCount = 0;
		bool isVertexProperty = true;
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
		  } else if (tokens[0] == "element") {
			if (tokens.size() != 3) {
			  std::cout << "Error: corrupted element info!" << std::endl;
			  return false;
			}
			if (tokens[1] == "vertex") {
			  pointCount = atoi(tokens[2].c_str());
			} else {
			  isVertexProperty = false;
			}
		  } else if (tokens[0] == "property" && isVertexProperty) {
			if (tokens.size() != 3) {
			  std::cout << "Error: corrupted property info!" << std::endl;
			  return false;
			}
			const std::string &propertyType = tokens[1];
			const std::string &propertyName = tokens[2];
			const size_t attributeIndex = attributesInfo.size();
			attributesInfo.resize(attributeIndex + 1);
			AttributeInfo &attributeInfo = attributesInfo[attributeIndex];
			attributeInfo.name = propertyName;
			if (propertyType == "float64") {
			  attributeInfo.type = ATTRIBUTE_TYPE_FLOAT64;
			  attributeInfo.byteCount = 8;
			} else if (propertyType == "float" || propertyType == "float32") {
			  attributeInfo.type = ATTRIBUTE_TYPE_FLOAT32;
			  attributeInfo.byteCount = 4;
			} else if (propertyType == "uint64") {
			  attributeInfo.type = ATTRIBUTE_TYPE_UINT64;
			  attributeInfo.byteCount = 8;
			} else if (propertyType == "uint32") {
			  attributeInfo.type = ATTRIBUTE_TYPE_UINT32;
			  attributeInfo.byteCount = 4;
			} else if (propertyType == "uint16") {
			  attributeInfo.type = ATTRIBUTE_TYPE_UINT16;
			  attributeInfo.byteCount = 2;
			} else if (propertyType == "uchar" || propertyType == "uint8") {
			  attributeInfo.type = ATTRIBUTE_TYPE_UINT8;
			  attributeInfo.byteCount = 1;
			} else if (propertyType == "int64") {
			  attributeInfo.type = ATTRIBUTE_TYPE_INT64;
			  attributeInfo.byteCount = 8;
			} else if (propertyType == "int32") {
			  attributeInfo.type = ATTRIBUTE_TYPE_INT32;
			  attributeInfo.byteCount = 4;
			} else if (propertyType == "int16") {
			  attributeInfo.type = ATTRIBUTE_TYPE_INT16;
			  attributeInfo.byteCount = 2;
			} else if (propertyType == "char" || propertyType == "int8") {
			  attributeInfo.type = ATTRIBUTE_TYPE_INT8;
			  attributeInfo.byteCount = 1;
			}
		  } else if (tokens[0] == "end_header") {
			break;
		  }
		}
		if (version != 1.0) {
		  std::cout << "Error: non-supported version!" << std::endl;
		  return false;
		}

		size_t indexX = PCC_UNDEFINED_INDEX;
		size_t indexY = PCC_UNDEFINED_INDEX;
		size_t indexZ = PCC_UNDEFINED_INDEX;
		size_t indexR = PCC_UNDEFINED_INDEX;
		size_t indexG = PCC_UNDEFINED_INDEX;
		size_t indexB = PCC_UNDEFINED_INDEX;
		size_t indexReflectance = PCC_UNDEFINED_INDEX;
		const size_t attributeCount = attributesInfo.size();
		for (size_t a = 0; a < attributeCount; ++a) {
		  const auto &attributeInfo = attributesInfo[a];
		  if (attributeInfo.name == "x" &&
			  (attributeInfo.byteCount == 8 || attributeInfo.byteCount == 4)) {
			indexX = a;
		  } else if (attributeInfo.name == "y" &&
					 (attributeInfo.byteCount == 8 || attributeInfo.byteCount == 4)) {
			indexY = a;
		  } else if (attributeInfo.name == "z" &&
					 (attributeInfo.byteCount == 8 || attributeInfo.byteCount == 4)) {
			indexZ = a;
		  } else if (attributeInfo.name == "red" && (attributeInfo.byteCount == 1|| attributeInfo.byteCount == 2)) {
			indexR = a;
		  } else if (attributeInfo.name == "green" && (attributeInfo.byteCount == 1 || attributeInfo.byteCount == 2)) {
			indexG = a;
		  } else if (attributeInfo.name == "blue" && (attributeInfo.byteCount == 1 || attributeInfo.byteCount == 2)) {
			indexB = a;
		  } else if ((attributeInfo.name == "reflectance" || attributeInfo.name == "refc") &&
					 attributeInfo.byteCount <= 2) {
			indexReflectance = a;
		  }
		}
		if (indexX == PCC_UNDEFINED_INDEX || indexY == PCC_UNDEFINED_INDEX ||
			indexZ == PCC_UNDEFINED_INDEX) {
		  std::cout << "Error: missing coordinates!" << std::endl;
		  return false;
		}
		resize(pointCount);
		if (isAscii) {
		  size_t pointCounter = 0;
		  while (!ifs.eof() && pointCounter < pointCount) {
			ifs.getline(tmp, MAX_BUFFER_SIZE);
			getTokens(tmp, sep, tokens);
			if (tokens.empty()) {
			  continue;
			}
			if (tokens.size() < attributeCount) {
			  return false;
			}
			auto &position = points[pointCounter].getPosition();
			position[0] = atof(tokens[indexX].c_str());
			position[1] = atof(tokens[indexY].c_str());
			position[2] = atof(tokens[indexZ].c_str());
			auto &color = points[pointCounter].getColor();
			color[0] = atoi(tokens[indexR].c_str());
			color[1] = atoi(tokens[indexG].c_str());
			color[2] = atoi(tokens[indexB].c_str());
			

			++pointCounter;
		  }
		} else {
		  for (size_t pointCounter = 0; pointCounter < pointCount && !ifs.eof(); ++pointCounter) {
			auto &position = points[pointCounter].getPosition();
			for (size_t a = 0; a < attributeCount && !ifs.eof(); ++a) {
			  const auto &attributeInfo = attributesInfo[a];
			  if (a == indexX) {
				if (attributeInfo.byteCount == 4) {
				  float x;
				  ifs.read(reinterpret_cast<char *>(&x), sizeof(float));
				  position[0] = x;
				} else {
				  double x;
				  ifs.read(reinterpret_cast<char *>(&x), sizeof(double));
				  position[0] = x;
				}
			  } else if (a == indexY) {
				if (attributeInfo.byteCount == 4) {
				  float y;
				  ifs.read(reinterpret_cast<char *>(&y), sizeof(float));
				  position[1] = y;
				} else {
				  double y;
				  ifs.read(reinterpret_cast<char *>(&y), sizeof(double));
				  position[1] = y;
				}
			  } else if (a == indexZ) {
				if (attributeInfo.byteCount == 4) {
				  float z;
				  ifs.read(reinterpret_cast<char *>(&z), sizeof(float));
				  position[2] = z;
				} else {
				  double z;
				  ifs.read(reinterpret_cast<char *>(&z), sizeof(double));
				  position[2] = z;
				}
			  } else if (a == indexR && attributeInfo.byteCount == 1) {
				auto &color = points[pointCounter].getColor();
				ifs.read(reinterpret_cast<char *>(&color[0]), sizeof(uint8_t));
			  } else if (a == indexG && attributeInfo.byteCount == 1) {
				auto &color = points[pointCounter].getColor();
				ifs.read(reinterpret_cast<char *>(&color[1]), sizeof(uint8_t));
			  } else if (a == indexB && attributeInfo.byteCount == 1) {
				auto &color = points[pointCounter].getColor();
				ifs.read(reinterpret_cast<char *>(&color[2]), sizeof(uint8_t));
			  } else {
				char buffer[128];
				ifs.read(buffer, attributeInfo.byteCount);
			  }
			}
		  }
		}
		return true;
	  }
	  void convertRGBToYUV() {  // BT709
		for (auto &point : points) {

		  auto& color = point.getColor();
		  const uint16_t r = color[0];
		  const uint16_t g = color[1];
		  const uint16_t b = color[2];
		  const double y = std::round(0.212600 * r + 0.715200 * g + 0.072200 * b);
		  const double u = std::round(-0.114572 * r - 0.385428 * g + 0.500000 * b + 128.0);
		  const double v = std::round(0.500000 * r - 0.454153 * g - 0.045847 * b + 128.0);
		  assert(y >= 0.0 && y <= 255.0 && u >= 0.0 && u <= 255.0 && v >= 0.0 && v <= 255.0);
		  color[0] = static_cast<uint16_t>(y);
		  color[1] = static_cast<uint16_t>(u);
		  color[2] = static_cast<uint16_t>(v);
		}
	  }
	  void convertRGBToYUVClosedLoop() {  // BT709
		for (auto &point : points) {
		  auto& color = point.getColor();
		  const uint16_t r = color[0];
		  const uint16_t g = color[1];
		  const uint16_t b = color[2];
		  const double y = std::round(0.212600 * r + 0.715200 * g + 0.072200 * b);
		  const double u = std::round((b - y) / 1.8556 + 128.0);
		  const double v = std::round((r - y) / 1.5748 + 128.0);
		  assert(y >= 0.0 && y <= 255.0 && u >= 0.0 && u <= 255.0 && v >= 0.0 && v <= 255.0);
		  color[0] = static_cast<uint16_t>(y);
		  color[1] = static_cast<uint16_t>(u);
		  color[2] = static_cast<uint16_t>(v);
		}
	  }
	  void convertYUVToRGB() {  // BT709
		for (auto &point : points) {
			auto &color = point.getColor();
		  const double y1 = color[0];
		  const double u1 = color[1] - 128.0;
		  const double v1 = color[2] - 128.0;
		  const double r = PCCClip(round(y1 /*- 0.00000 * u1*/ + 1.57480 * v1), 0.0, 255.0);
		  const double g = PCCClip(round(y1 - 0.18733 * u1 - 0.46813 * v1), 0.0, 255.0);
		  const double b = PCCClip(round(y1 + 1.85563 * u1 /*+ 0.00000 * v1*/), 0.0, 255.0);
		  color[0] = static_cast<uint16_t>(r);
		  color[1] = static_cast<uint16_t>(g);
		  color[2] = static_cast<uint16_t>(b);
		}
	  }
	  
	  void ConvertColorFrom10bTo8b() //将颜色位深从10bit转换为8bit
	  {
		  for (int i = 0; i < points.size(); i++)
		  {
			  const PCCColor3B &color = getColor(i);
			  PCCColor3B colorTemp;
			  colorTemp[0] = color[0] / 4;
			  colorTemp[1] = color[1] / 4;
			  colorTemp[2] = color[2] / 4;
			  setColor(i, colorTemp);
		  }
	  }
	  void ConvertColorFrom8bTo10b() //将颜色位深从10bit转换为8bit
	  {
		  for (int i = 0; i < points.size(); i++)
		  {
			  const PCCColor3B &color = getColor(i);
			  PCCColor3B colorTemp;
			  colorTemp[0] = color[0] * 4;
			  colorTemp[1] = color[1] * 4;
			  colorTemp[2] = color[2] * 4;
			  setColor(i, colorTemp);
		  }
	  }
	  size_t addPoints(PCCPointSet3 &pointCloudFrame) //添加点集
	  {
		  const size_t m_pointCount = getPointCount();
		  const size_t pointCount = pointCloudFrame.getPointCount();

		  resize(m_pointCount + pointCount);
		  for (int i = 0; i < pointCount; i++)
		  {
			  setPosition(m_pointCount + i, pointCloudFrame.getPosition(i));
			  setColor(m_pointCount + i, pointCloudFrame.getColor(i));
		  }
		  return getPointCount();
	  }
	  void ToCameraCoordinates(PCCVector3<double> R, PCCVector3<double> T)//世界坐标->相机坐标
	  {
		  const size_t pointCount = getPointCount();
		  for (int i = 0; i < pointCount; i++)
		  {
			  points[i].getPosition() = points[i].getPosition()-T;
		  }
	  }
	  void ToWorldCoordinates(PCCVector3<double> R, PCCVector3<double> T)//相机坐标->世界坐标
	  {
		  const size_t pointCount = getPointCount();
		  for (int i = 0; i < pointCount; i++)
		  {
			  points[i].getPosition() = points[i].getPosition() + T;
		  }
	  }
	  
	  int sign(double Z)
	  {
		  return (Z > 0 ? 1 : -1);
	  }

	  void ToPolarCoordinates(uint8_t xBitDepth, uint8_t yBitDepth,uint8_t zBitDepth, double Rnear, double Rfar)
	  {
		  //W->X H->Y depth->Z
		  uint16_t W = 1 << xBitDepth;
		  uint16_t H = 1 << yBitDepth;
		  long vmax = (1 << zBitDepth) - 1;

		  double phi, theta;
		  uint16_t n, m, z;
		  const size_t pointCount = getPointCount();
		  for (int i = 0; i < pointCount; i++)
		  {
			  PCCPoint3D &position = getPosition(i);
	
			  double R = sqrt(position.x() * position.x() + position.y() * position.y() + position.z() * position.z());

			  phi = -sign(position.y())*acos(position.x() / sqrt(position.x() * position.x() + position.y() * position.y()));
			  theta = asin(position.z() / R);

			  n = floor((phi / (2 * PI) + 0.5)*W - 0.5 + 0.5);
			  n = n >= W ? 0 : n;
			  n = n < 0 ? W - 1 : n;
			  m = floor((0.5 - theta / PI)*H - 0.5 + 0.5);
			  m = m >= H ? 0 : m;
			  m = m < 0 ? H - 1 : m;

			  z= floor(vmax * (1 / R - 1 / Rfar) / (1 / Rnear - 1 / Rfar) + 0.5);
			  position = PCCPoint3D(n, m, z);
		  }
	  }
	  void ToDescartesCoordinates(uint8_t xBitDepth, uint8_t yBitDepth, uint8_t zBitDepth, double Rnear, double Rfar)
	  {
		  uint16_t W = 1 << xBitDepth;
		  uint16_t H = 1 << yBitDepth;
		  long vmax = (1 << zBitDepth) - 1;

		  double phi;
		  double theta;
		  double R;

		  PCCPoint3D point3D;
		  PCCColor3B Color3B;
		  size_t piontCount = getPointCount();

		  const size_t pointCount = getPointCount();
		  for (int i = 0; i < pointCount; i++)
		  {
			  PCCPoint3D &position = getPosition(i);

			  uint16_t n = position[0], m = position[1], Depth = position[2];
			
			  phi = ((n + 0.5) / W - 0.5) * 2 * PI;
			  theta = (0.5 - (m + 0.5) / H)*PI;

			  R = vmax * Rnear*Rfar / (Depth*(Rfar - Rnear) + vmax * Rnear);

			  position[0] = R * cos(theta)*cos(phi);
			  position[1] = -R * cos(theta)*sin(phi);
			  position[2] = R * sin(theta);
		  }
	  }

	  void sort()
	  {
		  std::sort(points.begin(), points.end());
	  }
	  void RemoveRepeatePoints()
	  {
		  size_t i = 0, j = 0, k;
		  for (k = 1; k < points.size(); k++)
		  {
			  if (points[k].getPosition() == points[j].getPosition()) continue;
			  PCCPoint3D tempPosition = points[j].getPosition();
			  PCCColor3B tempColor(0, 0, 0);
			  for (size_t m = j; m < k; m++)
			  {
				  tempColor += points[m].getColor();
			  }
			  tempColor /= (k - j);
			  points[i].getPosition() = tempPosition;
			  points[i].getColor() = tempColor;
			  i++;
			  j = k;
		  }

		  PCCPoint3D tempPosition = points[j].getPosition();
		  PCCColor3B tempColor(0, 0, 0);
		  for (size_t m = j; m < k; m++)
		  {
			  tempColor += points[m].getColor();
		  }
		  tempColor /= (k - j);
		  points[i].getPosition() = tempPosition;
		  points[i].getColor() = tempColor;
		  i++;

		  resize(i);
	  }

	  void Voxelize(uint8_t PointCloudGeometryBitDepth) //点云体素化
	  {

		  assert(PointCloudGeometryBitDepth <= 16 );
		  PCCBox3D box3D = computeBoundingBox();
		  double xSize = box3D.max.x() - box3D.min.x();
		  double ySize = box3D.max.y() - box3D.min.y();
		  double zSize = box3D.max.z() - box3D.min.z();
		  double maxSize = (xSize > ySize ? xSize : ySize) > zSize ? (xSize > ySize ? xSize : ySize) : zSize;
		  double stepSize = maxSize / pow(2, PointCloudGeometryBitDepth);

		  const size_t pointCount = getPointCount();
		  for (int i = 0; i < pointCount; i++)
		  {
			  const PCCPoint3D& position = points[i].getPosition();
			  PCCPoint3D positionTemp;
			  positionTemp[0] = std::floor((position[0] - box3D.min.x()) / stepSize);
			  positionTemp[1] = std::floor((position[1] - box3D.min.y()) / stepSize);
			  positionTemp[2] = std::floor((position[2] - box3D.min.z()) / stepSize);
			  setPosition(i, positionTemp);
		  }
		  //在这里可以打印box3D和stepSize
	  }

	 private:
	  std::vector<PCCPoint> points;
	};
}

#endif /* PCCPointSet_h */
