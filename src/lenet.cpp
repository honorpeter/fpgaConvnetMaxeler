#include <iostream>
#include "mnist.h"
#include "lenet.h"
#include "feedforward.h"

template<typename T>
static T max(T a, T b) {
    return a > b ? a : b ;
}

void inplace_relu(int N, float *x) {
    for (int i = 0 ; i < N ; i++) {
        x[i] = max(0.0f, x[i]);
    }
}

float *reshape_lenet_conv_out(int N, float *a) {
    int height = 4;
    int width = 4;
    int channels = 50;
    float* ret = new float[N * height * width * channels];
    // a is now  height * width * channels
    // we want to make it channels * height * width

    for (int t = 0 ; t < N ; t++) {
        for (int c = 0 ; c < channels ; c++) {
            for (int i = 0 ; i < height ; i++) {
                for (int j = 0 ; j < width ; j++) {
                    int x = t * (height * width * channels) + c * (height * width) + i * width + j;
                    int y = t * (height * width * channels) + i * (width * channels) + j * channels + c;
                    ret[x] = a[y];
                }
            }
        }
    }
   

    return ret;
}

int main() {
#ifdef __SIM__
    int N = 3;
#else
    int N = 100;
#endif

    float *x, *conv_out;
    std::vector<std::vector<double> > images;
    std::vector<int> labels;
    layer_t layers[N_LAYERS];
    x = new float[N * 784];
    conv_out = new float[N * 800];

    for (int i = 0 ; i < N * 800 ; i++) {
        conv_out[i] = 0;
    }

    try {
        std::cout << "Reading images ..." << std::endl;
        read_mnist_images(images, "./mnist/t10k-images-idx3-ubyte");
        read_mnist_labels(labels, "./mnist/t10k-labels-idx1-ubyte");

        std::cout << "Initializing arrays ..." << std::endl;
        for (int i = 0 ; i < N ; i++) {
            for (int j = 0 ; j < 784 ; j++) {
                x[i * 784 + j] = (float) images[i][j];
            }
        } 

        std::cout << "Initializing fully connected weights ..." << std::endl;
        fully_connected_layers_init(layers);

        /*
        for (int i = 0 ; i < N ; i++) {
            for (int j = 0 ; j < 784 ; j++) {
                std::cout << x[i * 784 + j] << " ";
            }
            std::cout << std::endl;
            std::cout << "_________________________" << std::endl;
        }
        */

        std::cout << "Running CNN ... " << std::endl;
        lenet(N, x, conv_out);

        /*
        for (int i = 0 ; i < N ; i++) {
            for (int j = 0 ; j < layers[0].in ; j++) {
                std::cout << conv_out[i * layers[0].in + j] << " ";
            }
            std::cout << std::endl;
            std::cout << "_________________________" << std::endl;
        }
        */

        std::cout << "HERE" << std::endl;
        float *a = feed_forward(N, reshape_lenet_conv_out(N, conv_out), layers[0]);
        for (int i = 0 ; i < 100 ; i++) {
            std::cout << a[i] << " ";
        }

        inplace_relu(N * layers[0].out, a);
        float *b = feed_forward(N, a, layers[1]);
        float *c = softmax(N, 10, b);
        int *klasses = get_row_max_index(N, 10, c);

        for (int i = 0 ; i < N ; i++) {
            std::cout << "Example " << i << ": " << std::endl;
            std::cout << "Softmax: " << std::endl;
            for (int j =0 ; j < 10 ; j++) {
                std::cout << c[i * 10 + j] << " ";
            }
            std::cout << std::endl;
            std::cout << "Class: " << klasses[i] << std::endl;
            std::cout << "Expected class: " << labels[i] << std::endl;
            std::cout << std::endl;
        }

    } catch (const std::string & s) {
        std::cerr << s << std::endl;
    }

    return 0;
}