# ErpPlyconverter
ErpPlyconverter

参数示例：
ErpToPly.bat
  ErpPlyconverter.exe -mode 0 -frameNumber 1 -pointCloudGeoBitDepth 16 -outputPlyPath PLY -inputErpCfgPath cfg\classroom-part.cfg
PlyToErp.bat
  ErpPlyconverter.exe -mode 1 -inputPointCloudCfgPath cfg\PointCloud-all.cfg -outputRecYUVPath YUVrec
  
PlaneToPly.bat
  ErpPlyconverter.exe -mode 2 -frameNumber 1 -pointCloudGeoBitDepth 16 -outputPlyPath PLY -inputPlaneCfgPath cfg\plane.cfg

DirctToPly.bat
  ErpPlyconverter.exe -mode 3 -frameNumber 1 -pointCloudGeoBitDepth 16 -outputPlyPath PLY -inputPlaneCfgPath cfg\dirct.cfg
  
YUVDownsample.bat
  ErpPlyconverter.exe -mode 4 -frameNumber 300 -pointCloudGeoBitDepth 16 -outputPlyPath PLY -inputErpCfgPath cfg\hijack-all.cfg -outputRecYUVPath YUVrec
