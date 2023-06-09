#include "OnnxModel.h"

OnnxModel::OnnxModel()
{

}

OnnxModel::~OnnxModel()
{

}

void OnnxModel::setup(File modelPath) // ONNXの初期設定
{
    // ONNXの環境オブジェクトを生成．ログレベルを設定
    env = Ort::Env(Ort::Env(ORT_LOGGING_LEVEL_WARNING, "Default"));
    // onnxruntime session start
    session = Ort::Session(env, modelPath.getFullPathName().toRawUTF8() , Ort::SessionOptions{nullptr});
    
    auto type_info = session.GetInputTypeInfo(0);

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

std::vector<float> OnnxModel::process(std::vector<float> x, std::vector<float> label, int WAVETABLE_SIZE) // 推論部分
{
    /*
    bool isStereo = (buffer.getNumChannels()>=2)? true:false;
    
    // convert to mono if stereo
    if (isStereo)
    {
        // add the right (1) to the left (0)
        buffer.addFrom(0, 0, buffer, 1, 0, buffer.getNumSamples());
    }
    */
    DBG("OnnxModel::process");
         
    auto memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);

    std::vector<int64_t> input_node_dims1 = {1, 1, WAVETABLE_SIZE};
    std::vector<int64_t> input_node_dims2 = {1, static_cast<int>(label.size())};

    std::vector<Ort::Value> ort_inputs;
    ort_inputs.emplace_back(Ort::Value::CreateTensor<float>(memory_info,
                                                            x.data(),
                                                            x.size(),
                                                            input_node_dims1.data(),
                                                            input_node_dims1.size()));
    ort_inputs.emplace_back(Ort::Value::CreateTensor<float>(memory_info,
                                                            label.data(),
                                                            label.size(),
                                                            input_node_dims2.data(),
                                                            input_node_dims2.size()));
    Ort::RunOptions run_options;
    auto output_tensors = session.Run(run_options,
                                      input_node_names.data(),
                                      ort_inputs.data(),
                                      2,
                                      output_node_names.data(),
                                      output_node_names.size());
    
    //float* floatarr = output_tensors.front().GetTensorMutableData<float>();
    float* floatarr =output_tensors[2].GetTensorMutableData<float>();
    std::vector<float> fetchedData(WAVETABLE_SIZE);
    for (int i = 0; i < WAVETABLE_SIZE; ++i)
    {
        fetchedData[i] = (floatarr[i]);
    }
    return fetchedData;
}
