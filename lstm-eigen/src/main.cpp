#include <AudioFile.h>
#include <Eigen/Dense>
#include <nlohmann/json.hpp>

#include <chrono>
#include <iostream>

float sigmoid(float x)
{
    return 1.0f / (1.0f + expf(-x));
}

Eigen::MatrixXf to_eigen(const std::vector<std::vector<float>>& values)
{
    auto rows = values.size();
    auto cols = values[0].size();
    auto mat = Eigen::MatrixXf(rows, cols);
    for (auto row = 0; row < rows; row++)
        for (auto col = 0; col < cols; col++)
            mat(row, col) = values[row][col];

    return mat;
}

Eigen::RowVectorXf to_eigen(const std::vector<float>& values)
{
    auto rows = values.size();
    auto vec = Eigen::RowVectorXf(rows);
    for (auto row = 0; row < rows; row++)
        vec(row) = values[row];

    return vec;
}

int main()
{
    // Load the model
    std::ifstream model_json_file("../model/dds.json");
    nlohmann::json model_json;
    model_json_file >> model_json;

    // Model
    auto lstm_weight_ih = to_eigen(model_json["/lstm.weight_ih_l0"_json_pointer].get<std::vector<std::vector<float>>>());
    auto lstm_weight_hh = to_eigen(model_json["/lstm.weight_hh_l0"_json_pointer].get<std::vector<std::vector<float>>>());
    auto lstm_bias_ih = to_eigen(model_json["/lstm.bias_ih_l0"_json_pointer].get<std::vector<float>>());
    auto lstm_bias_hh = to_eigen(model_json["/lstm.bias_hh_l0"_json_pointer].get<std::vector<float>>());
    auto linear_weight = to_eigen(model_json["/linear.weight"_json_pointer].get<std::vector<std::vector<float>>>());
    auto linear_bias = to_eigen(model_json["/linear.bias"_json_pointer].get<std::vector<float>>());

    auto input_size = lstm_weight_ih.cols();
    auto output_size = linear_weight.cols();
    auto gate_size = lstm_weight_ih.rows();
    auto hidden_size = gate_size / 4;

    // Load audio file
    AudioFile<float> input_audio("../process/input.wav");
    auto sample_rate = input_audio.getSampleRate();
    auto bit_depth = input_audio.getBitDepth();
    auto num_samples = input_audio.getNumSamplesPerChannel();
    auto num_channels = input_audio.getNumChannels();
    auto length_in_seconds = input_audio.getLengthInSeconds();
    std::cout << "Audio: " << bit_depth << "-bit @ " << sample_rate << " Hz | " << num_channels
              << " channel(s) | " << length_in_seconds << " sec | " << num_samples << " samples\n";

    constexpr auto channel { 0 };
    constexpr auto sf { 1.0f };
    constexpr auto delay_fine { 0.0f };

    AudioFile<float> output_file;
    output_file.setNumChannels(num_channels);
    output_file.setNumSamplesPerChannel(num_samples);
    output_file.setBitDepth(bit_depth);
    output_file.setSampleRate(sample_rate);

    auto input = Eigen::RowVectorXf(input_size).setZero();
    auto output = Eigen::RowVectorXf(output_size).setZero();
    auto gates = Eigen::RowVectorXf(gate_size).setZero();
    auto c = Eigen::RowVectorXf(hidden_size).setZero();
    auto h = Eigen::RowVectorXf(hidden_size).setZero();

    std::cout << "Processing...\n";
    auto start = std::chrono::high_resolution_clock::now();
    auto idx = 0;
    for (const auto& sample : input_audio.samples[channel]) {
        input[0] = sample;
        input[1] = sf;
        input[2] = delay_fine;

        // LSTM
        gates = input * lstm_weight_ih.transpose() + lstm_bias_ih + h * lstm_weight_hh.transpose() + lstm_bias_hh;
        for (auto i = 0; i < hidden_size; i++) {
            c[i] = sigmoid(gates[hidden_size + i]) * c[i] + sigmoid(gates[i]) * tanhf(gates[2 * hidden_size + i]);
            h[i] = sigmoid(gates[3 * hidden_size + i]) * tanhf(c[i]);
        }

        // Linear
        output = h * linear_weight.transpose() + linear_bias;
        output_file.samples[channel][idx++] = output.value();
    }
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(stop - start);

    output_file.save("../process/output.wav");
    std::cout << "Finished in " << duration.count() << " seconds\n";
}