#include "Model.h"
#include "BinaryData.h"

#include <nlohmann/json.hpp>
#include <sstream>

Model::Model()
{
    // Load json model
    nlohmann::json model_json;
    std::istringstream(BinaryData::dds19_lstm32_json) >> model_json;

    // Load model parameters
    lstmWeight_ih = stdToEigen(model_json["/lstm.weight_ih_l0"_json_pointer].get<StdMatrix>());
    lstmWeight_hh = stdToEigen(model_json["/lstm.weight_hh_l0"_json_pointer].get<StdMatrix>());
    lstmBias_ih = stdToEigen(model_json["/lstm.bias_ih_l0"_json_pointer].get<StdVector>());
    lstmBias_hh = stdToEigen(model_json["/lstm.bias_hh_l0"_json_pointer].get<StdVector>());
    linearWeight = stdToEigen(model_json["/linear.weight"_json_pointer].get<StdMatrix>());
    linearBias = stdToEigen(model_json["/linear.bias"_json_pointer].get<StdVector>());

    // Get model sizes
    inputSize = lstmWeight_ih.cols();
    outputSize = linearWeight.cols();
    gateSize = lstmWeight_ih.rows();
    hiddenSize = gateSize / 4;

    // Initialise
    input = EigVector(inputSize).setZero();
    output = EigVector(outputSize).setZero();
    gates = EigVector(gateSize).setZero();
    c_t = EigVector(hiddenSize).setZero();
    h_t = EigVector(hiddenSize).setZero();
}

float Model::process(float sample, float sf, float delayFine)
{
    // Input
    input[0] = sample;
    input[1] = sf;
    input[2] = delayFine;

    // LSTM
    gates = input * lstmWeight_ih.transpose() + lstmBias_ih + h_t * lstmWeight_hh.transpose() + lstmBias_hh;
    for (auto i = 0; i < hiddenSize; i++) {
        c_t[i] = sigmoid(gates[hiddenSize + i]) * c_t[i] + sigmoid(gates[i]) * tanhf(gates[2 * hiddenSize + i]);
        h_t[i] = sigmoid(gates[3 * hiddenSize + i]) * tanhf(c_t[i]);
    }

    // Linear
    output = h_t * linearWeight.transpose() + linearBias;
    return output.value();
}

Model::EigMatrix Model::stdToEigen(const Model::StdMatrix& values)
{
    auto rows = values.size();
    auto cols = values[0].size();
    auto mat = EigMatrix(rows, cols);
    for (size_t row = 0; row < rows; row++)
        for (size_t col = 0; col < cols; col++)
            mat(static_cast<EigIndex>(row), static_cast<EigIndex>(col)) = values[row][col];

    return mat;
}

Model::EigVector Model::stdToEigen(const Model::StdVector& values)
{
    auto rows = values.size();
    auto vec = EigVector(rows);
    for (size_t row = 0; row < rows; row++)
        vec(static_cast<EigIndex>(row)) = values[row];

    return vec;
}

float Model::sigmoid(float x)
{
    return 1.0f / (1.0f + expf(-x));
}
