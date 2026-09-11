// stepinfo.cpp
// 目标：用 OCCT 读取一个 STEP 文件，打印里面的"体/面/边/点"数量。
// 这是学习 OCCT 的第一步：先证明"OCCT 能读 STEP"这一条链路是通的。
//
// 用法：step-info <文件路径>   不传参数时使用下面的默认路径

#include <STEPControl_Reader.hxx>
#include <IFSelect_ReturnStatus.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS_Shape.hxx>
#include <TopAbs_ShapeEnum.hxx>

#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
    // 用法：step-info <文件路径>，不传则用默认路径
    std::string file = (argc > 1) ? argv[1] : "models/TCMT1107_4.stp";

    // 1) 读文件：只是把 STEP 文本解析成 OCCT 内部的"交换模型"
    STEPControl_Reader reader;
    IFSelect_ReturnStatus status = reader.ReadFile(file.c_str());
    if (status != IFSelect_RetDone)
    {
        std::cerr << "STEP 文件读取失败，状态码 = " << (int)status << std::endl;
        return 1;
    }

    // 2) 传输：把交换模型转成真正的 TopoDS_Shape 实体
    //    TransferRoots() 把文件里的每个根形状转成 OCCT 实体，返回成功数量
    Standard_Integer nRoots = reader.NbRootsForTransfer();
    Standard_Integer transferred = reader.TransferRoots();
    //    OneShape() 把所有结果合成一个整体形状（可能包含多个实体）
    TopoDS_Shape shape = reader.OneShape();

    // 3) 用 TopExp_Explorer 遍历拓扑，数一数各级元素的数量
    //    这正是以后"点线面体选择"要用的遍历方式
    int nSolid = 0, nFace = 0, nEdge = 0, nVertex = 0;
    for (TopExp_Explorer exp(shape, TopAbs_SOLID);  exp.More(); exp.Next()) nSolid++;
    for (TopExp_Explorer exp(shape, TopAbs_FACE);   exp.More(); exp.Next()) nFace++;
    for (TopExp_Explorer exp(shape, TopAbs_EDGE);   exp.More(); exp.Next()) nEdge++;
    for (TopExp_Explorer exp(shape, TopAbs_VERTEX); exp.More(); exp.Next()) nVertex++;

    std::cout << "========================================" << std::endl;
    std::cout << "STEP 读取成功: " << file << std::endl;
    std::cout << "根形状数     : " << nRoots << std::endl;
    std::cout << "成功传输数   : " << transferred << std::endl;
    std::cout << "体(solid)    : " << nSolid << std::endl;
    std::cout << "面(face)     : " << nFace << std::endl;
    std::cout << "边(edge)     : " << nEdge << std::endl;
    std::cout << "点(vertex)   : " << nVertex << std::endl;
    std::cout << "========================================" << std::endl;
    return 0;
}
