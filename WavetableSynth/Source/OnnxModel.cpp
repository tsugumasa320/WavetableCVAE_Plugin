#include "OnnxModel.h"

OnnxModel::OnnxModel()
{

}

OnnxModel::~OnnxModel()
{

}

void OnnxModel::setup(File modelPath) // ONNXの初期設定.後でデバッガで詳細を確認
{
    // ONNXの環境オブジェクトを生成．ログレベルを設定
    env = Ort::Env(Ort::Env(ORT_LOGGING_LEVEL_WARNING, "Default"));
    // onnxruntime session start
    session = Ort::Session(env, modelPath.getFullPathName().toRawUTF8() , Ort::SessionOptions{nullptr});
    
    auto type_info = session.GetInputTypeInfo(0);
    // auto tensor_info = type_info.GetTensorTypeAndShapeInfo();

    // モデルの入力ノード数を取得
    const size_t num_input_nodes = session.GetInputCount();
    
    input_names_ptr.reserve(num_input_nodes);
    input_node_names.reserve(num_input_nodes);
    
    for (size_t i = 0; i < num_input_nodes; ++i)
    {
        auto input_name = session.GetInputNameAllocated(i, allocator);
        input_node_names.push_back(input_name.get());
        input_names_ptr.push_back(std::move(input_name));
        // print input node types
        auto type_info = session.GetInputTypeInfo(i);
        auto tensor_info = type_info.GetTensorTypeAndShapeInfo();
        // ONNXTensorElementDataType type = tensor_info.GetElementType();
        // print input shapes/dims
        input_node_dims = tensor_info.GetShape();
    }

    const size_t num_output_nodes = session.GetOutputCount();
    for (size_t i = 0; i < num_output_nodes; ++i)
    {
        auto output_name = session.GetOutputNameAllocated(i, allocator);
        output_node_names.push_back(output_name.get());
        output_names_ptr.push_back(std::move(output_name));
    }
}

void OnnxModel::process(AudioBuffer<float>& buffer) // 推論部分
{
    bool isStereo = (buffer.getNumChannels()>=2)? true:false;
    
    // convert to mono if stereo
    if (isStereo)
    {
        // add the right (1) to the left (0)
        buffer.addFrom(0, 0, buffer, 1, 0, buffer.getNumSamples());
    }
    /*
     参考にできそう：https://github.com/WuYiming6526/HARD/blob/146ee18fa72fc40b80c179ae105ddf53e4ad73c1/Source/ONNXInferenceThread.cpp#L59
     
    auto memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    // send 2 blocks for inference!!
    auto input_tensor = Ort::Value::CreateTensor<float>(memory_info,
                                                        buffer,
                                                        600,
                                                        input_node_dims.data(),
                                                        input_node_dims.size());
    
    auto output_tensors =
    session.Run(Ort::RunOptions{nullptr}, input_node_names.data(), &buffer, 1, output_node_names.data(), 1);
    
    float* floatarr = output_tensors.front().GetTensorMutableData<float>();
     */
}
