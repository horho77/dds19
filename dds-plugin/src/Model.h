#pragma once

#include <Eigen/Dense>
#include <string>
#include <vector>

class Model {
public:
    explicit Model();
    float process(float sample, float sf, float delayFine);

private:
    using EigMatrix = Eigen::MatrixXf;
    using EigVector = Eigen::RowVectorXf;
    using EigIndex = Eigen::Index;
    using StdMatrix = std::vector<std::vector<float>>;
    using StdVector = std::vector<float>;

    static EigMatrix stdToEigen(const StdMatrix& values);
    static EigVector stdToEigen(const StdVector& values);
    static float sigmoid(float x);

    EigMatrix lstmWeight_ih;
    EigMatrix lstmWeight_hh;
    EigVector lstmBias_ih;
    EigVector lstmBias_hh;
    EigMatrix linearWeight;
    EigVector linearBias;

    EigIndex inputSize;
    EigIndex outputSize;
    EigIndex gateSize;
    EigIndex hiddenSize;

    EigVector input;
    EigVector output;
    EigVector gates;
    EigVector c_t;
    EigVector h_t;
};
