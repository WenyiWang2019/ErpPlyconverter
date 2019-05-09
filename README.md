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

参数说明：
1) ErpPlyconverter.exe参数列表说明
公共参数
-mode 转换模式
	0：ERP转点云
	1：点云转ERP

ERP转点云参数：	
-frameNumber 要将ERP转换为点云的帧数
-pointCloudGeoBitDepth 转换得到的点云几何位深
-outputPlyPath 生成的点云存储路径 
-inputErpCfgPath 待转换ERP配置文件路径

点云转ERP参数	
-inputPointCloudCfgPath 待转换点云配置文件路径
-outputRecYUVPath 生成的ERP文件存储路径

示例：
点云转ERP
ErpPlyconverter.exe -mode 0 -frameNumber 1 -pointCloudGeoBitDepth 16 -outputPlyPath PLY -inputErpCfgPath cfg\hijack-part.cfg
ERP转点云
ErpPlyconverter.exe -mode 1 -inputPointCloudCfgPath cfg\PointCloud-all.cfg -outputRecYUVPath YUVrec

2) ERP配置文件说明
element view后跟视点个数说明
property 后跟视点信息说明，有几个property 说明视点有几个信息，end_header后每一行的信息要与property相对应

示例：大括号{}中的内容均为说明
cfg
format ascii 1.0
comment This is comment
element view 10 {   视点个数,end_header后面每一行表示一个视点的信息，所以end_header后的行数与这里要相符  } 
property string depthFile 
property string textureFile
property float T1
property float T2
property float T3
property float R1
property float R2
property float R3
end_header
YUV\hijack\v2_4096x4096_0_5_25_0_420_10b.yuv YUV\hijack\v2_4096x4096_420_10b.yuv -0.2427 -0.1763 1.65 0 -0 0
YUV\hijack\v2vs0out_4096x4096_0_5_25_0_420_10b.yuv YUV\hijack\v2vs0out_4096x4096_420_10b.yuv 0.3 -0 1.65 0 -0 0 
YUV\hijack\v2vs1out_4096x4096_0_5_25_0_420_10b.yuv YUV\hijack\v2vs1out_4096x4096_420_10b.yuv 0.0927 -0.2853 1.65 0 -0 0
YUV\hijack\v2vs3out_4096x4096_0_5_25_0_420_10b.yuv YUV\hijack\v2vs3out_4096x4096_420_10b.yuv -0.2427 0.1763 1.65 0 -0 0
YUV\hijack\v2vs4out_4096x4096_0_5_25_0_420_10b.yuv YUV\hijack\v2vs4out_4096x4096_420_10b.yuv 0.0927 0.2853 1.65 0 -0 0
YUV\hijack\v2vs5out_4096x4096_0_5_25_0_420_10b.yuv YUV\hijack\v2vs5out_4096x4096_420_10b.yuv -0.15 0 1.3902 0 -0 0
YUV\hijack\v2vs6out_4096x4096_0_5_25_0_420_10b.yuv YUV\hijack\v2vs6out_4096x4096_420_10b.yuv 0.15 0 1.3902 0 -0 0
YUV\hijack\v2vs7out_4096x4096_0_5_25_0_420_10b.yuv YUV\hijack\v2vs7out_4096x4096_420_10b.yuv -0.15 0 1.9098 0 -0 0
YUV\hijack\v2vs8out_4096x4096_0_5_25_0_420_10b.yuv YUV\hijack\v2vs8out_4096x4096_420_10b.yuv 0.15 0 1.9098 0 -0 0
YUV\hijack\v2vs9out_4096x4096_0_5_25_0_420_10b.yuv YUV\hijack\v2vs9out_4096x4096_420_10b.yuv 0 -0 1.65 0 -0 0

3) 点云配置文件说明
element pointCloud 后跟点云个数说明，
紧随其后的 property 后跟点云信息说明
element view后跟视点个数说明
紧随其后的 property 后跟视点信息说明

示例：
cfg
format ascii 1.0
comment This is comment
element pointCloud 1
property string plyFile
element view 3
property string viewName
property float T1
property float T2
property float T3
property float R1
property float R2
property float R3
end_header
PLY\syn_4096_4096_0_5_25_0_420_10b_0_000000000000000_0_000000000000000_0_000000000000000_0_000000000000000_0.ply
v0 0.3 -0 1.65 0 -0 0
v1 0.0927 -0.2853 1.65 0 -0 0
v2 -0.2427 -0.1763 1.65 0 -0 0
v3 -0.2427 0.1763 1.65 0 -0 0
v4 0.0927 0.2853 1.65 0 -0 0
v5 -0.15 0 1.3902 0 -0 0
v6 0.15 0 1.3902 0 -0 0
v7 -0.15 0 1.9098 0 -0 0
v8 0.15 0 1.9098 0 -0 0
v9 0 -0 1.65 0 -0 0
