#include <DataStreamClient.h>

#include <iomanip>
#include <iostream>
#include <sstream>

#include "serial_port/serial_port.hpp"

int main() {
  // 创建 Vicon DataStream 客户端实例
  ViconDataStreamSDK::CPP::Client dataStreamClient;

  // 连接到 Vicon DataStream 服务器
  const std::string ipAddress = "192.168.10.1";  // 根据实际情况修改 IP 地址
  if (dataStreamClient.Connect(ipAddress).Result !=
      ViconDataStreamSDK::CPP::Result::Success) {
    std::cerr << "连接到 Vicon DataStream 服务器失败！" << std::endl;
    return 1;
  }

  std::cout << "已成功连接到 Vicon DataStream 服务器！" << std::endl;

  // 设置数据流模式为服务器推送
  dataStreamClient.EnableSegmentData();
  dataStreamClient.SetStreamMode(
      ViconDataStreamSDK::CPP::StreamMode::ServerPush);

  // 配置串口
  std::string portName = "COM3";  // 串口端口名称，根据实际情况修改
  unsigned int baudRate = 115200;  // 串口波特率

  // 打开串口
  SerialPort serialPort(portName, baudRate);
  if (!serialPort.is_open()) {
    std::cerr << "无法打开串口！" << std::endl;
    return 1;
  }

  std::cout << "已成功打开串口！" << std::endl;

  // 获取数据
  while (true) {
    // 等待新帧
    dataStreamClient.WaitForFrame();

    // 获取主体数量
    ViconDataStreamSDK::CPP::Output_GetSubjectCount subjectCount =
        dataStreamClient.GetSubjectCount();

    // 遍历每个主体
    for (unsigned int i = 0; i < subjectCount.SubjectCount; ++i) {
      // 获取主体名称
      ViconDataStreamSDK::CPP::Output_GetSubjectName subjectName =
          dataStreamClient.GetSubjectName(i);

      // 获取主体的根节点名称
      ViconDataStreamSDK::CPP::Output_GetSegmentName rootSegmentName =
          dataStreamClient.GetSegmentName(subjectName.SubjectName, 0);

      // 获取根节点的全局位置和旋转
      ViconDataStreamSDK::CPP::Output_GetSegmentGlobalTranslation
          globalTranslation = dataStreamClient.GetSegmentGlobalTranslation(
              subjectName.SubjectName, rootSegmentName.SegmentName);
      ViconDataStreamSDK::CPP::Output_GetSegmentGlobalRotationQuaternion
          globalRotation = dataStreamClient.GetSegmentGlobalRotationQuaternion(
              subjectName.SubjectName, rootSegmentName.SegmentName);

      // 将数据格式化为字符串
      std::stringstream dataStream;
      dataStream << std::fixed << std::setprecision(2) << "P:("
                 << globalTranslation.Translation[0] << ", "
                 << globalTranslation.Translation[1] << ", "
                 << globalTranslation.Translation[2] << ")"
                 << " R:(" << globalRotation.Rotation[0] << ", "
                 << globalRotation.Rotation[1] << ", "
                 << globalRotation.Rotation[2] << ", "
                 << globalRotation.Rotation[3] << ")\n";

      // 将位置和旋转数据发送到串口
      std::string dataToSend = dataStream.str();
      serialPort.write(dataToSend.c_str(), dataToSend.length());
    }
  }

  // 关闭串口
  serialPort.close();

  return 0;
}