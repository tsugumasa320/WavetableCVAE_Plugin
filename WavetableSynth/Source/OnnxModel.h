# pragma once

#include <JuceHeader.h>
#include "onnxruntime_cxx_api.h"
#include <type_traits>

class OnnxModel
{
public:
    OnnxModel();
    ~OnnxModel();
    
    void setup(File modelPath);
    std::vector<float> process(std::vector<float> x, std::vector<float> label, int WAVETABLE_SIZE);
    
private:
    Ort::Env env;
    Ort::Session session{nullptr};
    
    std::vector<Ort::AllocatedStringPtr> input_names_ptr;
    std::vector<Ort::AllocatedStringPtr> output_names_ptr;
    std::vector<const char*> input_node_names;
    std::vector<const char*> output_node_names;
    Ort::AllocatorWithDefaultOptions allocator;
    std::vector<int64_t>input_node_dims;
    
};
