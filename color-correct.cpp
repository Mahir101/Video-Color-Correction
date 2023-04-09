#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

int main()
{
    Mat image = imread("input.jpg"); // read image from file

    if (image.empty()) {
        cout << "Could not read image file" << endl;
        return -1;
    }

    // convert to LAB color space
    Mat labImage;
    cvtColor(image, labImage, COLOR_BGR2Lab);

    // split channels
    vector<Mat> labChannels;
    split(labImage, labChannels);

    // normalize L channel
    Mat L;
    labChannels[0].convertTo(L, CV_32F, 1.0 / 255.0);
    normalize(L, L, 0, 1, NORM_MINMAX);

    // apply color correction to a and b channels
    for (int i = 1; i < 3; i++) {
        labChannels[i].convertTo(labChannels[i], CV_32F, 1.0 / 255.0);
        pow(labChannels[i], 0.5, labChannels[i]);
        labChannels[i] *= 255.0;
    }

    // merge channels back
    merge(labChannels, labImage);

    // convert back to BGR color space
    Mat result;
    cvtColor(labImage, result, COLOR_Lab2BGR);

    // write result to file
    imwrite("output.jpg", result);

    return 0;
}
