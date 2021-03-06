#include <iostream>
#include "mnist.h"
#include "lenet.h"
#include "feedforward.h"


/* TODO(ADD_NETWORK): Dimension of output of convolutional neural net */
const int CONV_INPUT_HEIGHT = 28;
const int CONV_INPUT_WIDTH = 28;
const int CONV_INPUT_CHANNELS = 1;
const int CONV_OUTPUT_HEIGHT = 4;
const int CONV_OUTPUT_WIDTH = 4;
const int CONV_OUTPUT_CHANNELS = 50;
/* End of dimension change */


template<typename T>
static T max(T a, T b) {
    return a > b ? a : b ;
}

void inplace_relu(int N, float *x) {
    for (int i = 0 ; i < N ; i++) {
        x[i] = max(0.0f, x[i]);
    }
}


float *reshape_conv_out(int N, float *a) {
    int height = CONV_OUTPUT_HEIGHT;
    int width = CONV_OUTPUT_WIDTH;
    int channels = CONV_OUTPUT_CHANNELS;
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
    int N = 1;
#else
    int N = 10000;
#endif

    float *x, *conv_out;
    std::vector<std::vector<double> > images;
    std::vector<int> labels;
    layer_t layers[N_LAYERS];
    x = new float[N * CONV_INPUT_HEIGHT * CONV_INPUT_WIDTH * CONV_INPUT_CHANNELS];
    conv_out = new float[N * CONV_OUTPUT_HEIGHT * CONV_OUTPUT_WIDTH * CONV_OUTPUT_CHANNELS];

    try {
        std::cout << "Reading images ..." << std::endl;
        /**
         * TODO(ADD_NETWORK): Code to read images and labels
         * Unfotunately, there is no simple template code
         * For lenet, it'd look like:
         *   read_mnist_images(images, "./mnist/t10k-images-idx3-ubyte");
         *   read_mnist_labels(labels, "./mnist/t10k-labels-idx1-ubyte");
         * It doesn't really matter how the images and labels variables are written,
         * so they are actually assigned at the end of the execution.
         **/

        std::cout << "Initializing arrays ..." << std::endl;
        for (int i = 0 ; i < N ; i++) {
            for (int j = 0 ; j < 784 ; j++) {
                x[i * 784 + j] = (float) images[i][j];
            }
        } 

        std::cout << "Running CNN ... " << std::endl;
        max_file_t *max_file = lenet_init();
        max_engine_t *dfe = max_load(max_file, "*");
        lenet_actions_t action;

        action.param_N = N;
        action.instream_x = x;
        action.outstream_y = conv_out;

        timeval t_begin, t_end;
        gettimeofday(&t_begin, NULL);
        lenet_run(dfe, &action);
        gettimeofday(&t_end, NULL);
		std::cout << "Completed feature extraction!" << std::endl;
        max_unload(dfe);
        double begin = double(t_begin.tv_sec) * 1000000 + double(t_begin.tv_usec);
        double end = double(t_end.tv_sec) * 1000000 + double(t_end.tv_usec);
        double delta = end - begin;


        /* TODO(ADD_NETWORK): Change code for joining doing fully connected weights here.
         * If you do not intend to change the fully connected layers' code, simply
         * remove the code snippet below.
         *
         * In the code snipper below, you are not allowed to overwrite:
         *  - t_begin
         *  - t_end
         *  - N
         *  - delta
         *  - throughput
         **/
        std::cout << "Initializing fully connected weights ..." << std::endl;
        fully_connected_layers_init(layers);
		std::cout << "Computing feedforward layers ..." << std::endl;
        float *a = feed_forward(N, reshape_conv_out(N, conv_out), layers[0]);
        inplace_relu(N * layers[0].out, a);
        float *b = feed_forward(N, a, layers[1]);
        float *c = softmax(N, 10, b);
        int *klasses = get_row_max_index(N, 10, c);
        int total_correct = 0;
        for (int i = 0 ; i < N ; i++) {
            if (klasses[i] == labels[i]) {
                total_correct++;
            }
        }
        std::cout << "Proportion correct " << (double(total_correct) / double(N)) << std::endl;
        /* End of fully connected layers. */

		double throughput = double(N) / delta * 1000000;
		std::cout << "t_begin.tv_sec = " << t_begin.tv_sec << std::endl;
		std::cout << "t_begin.tv_usec = " << t_begin.tv_usec << std::endl;
		std::cout << "t_end.tv_sec = " << t_end.tv_sec << std::endl;
		std::cout << "t_end.tv_usec = " << t_end.tv_usec << std::endl;
        std::cout << "Total time = " << delta << std::endl;
        std::cout << "Throughput (images per second) = " << throughput << std::endl;
        /* TODO(ADD_NETWORK): Change GOOPS calculation here. */
		std::cout << "GOps = " << throughput * 0.0038 << std::endl;

    } catch (const std::string & s) {
        std::cerr << s << std::endl;
    }

    return 0;
}
