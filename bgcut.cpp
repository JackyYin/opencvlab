#include <iostream>
#include <sstream>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

const cv::String params
    = "{input|original.jpg|Path to image to be cut}";

class CutAreaHandler {
public:
    CutAreaHandler(const char *wdname, cv::Mat img) : wdname(wdname), img(img) {}

    void show(int x, int y) {
        cv::Mat cloned = img.clone();
        cv::rectangle(cloned, p0, cv::Point(x,y), cv::Scalar(255,0,0), 3);
        cv::imshow(wdname, cloned);
    }

    cv::Rect getRect() {
        return (p0.x >= p1.x) ?
            ((p0.y >= p1.y) ?
                cv::Rect(p1.x , p1.y, p0.x - p1.x , p0.y - p1.y)
                : cv::Rect(p1.x , p0.y, p0.x - p1.x , p1.y - p0.y))
            : ((p1.y >= p0.y) ?
                cv::Rect(p0.x , p0.y, p1.x - p0.x , p1.y - p0.y)
                : cv::Rect(p0.x , p1.y, p1.x - p0.x , p0.y - p1.y));
    }
private:
    const char *wdname;
    cv::Mat img;
public:
    cv::Point p0;
    cv::Point p1;
};

class FgHandler {
public:
    FgHandler(const char *wdname, cv::Mat &img) : wdname(wdname), img(img) {};

    void output(const char *filename) {
        cv::Mat canvas = cv::Mat(img.size(),CV_8UC4);
        cv::Mat channels[3];
        cv::split(img, channels);
        cv::Mat dst[4] = { channels[0], channels[1], channels[2], mask };
        cv::merge(dst, 4, canvas);
        /* img.copyTo(canvas, mask); */
        cv::imwrite(cv::String(filename), canvas);
    }

    void show() {
        cv::Mat canvas = cv::Mat(img.size(),CV_8UC3, cv::Scalar(255, 255, 255));
        img.copyTo(canvas, mask);
        auto num = std::to_string(radius);
        cv::putText(canvas, cv::String(num), cv::Point(20, 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 0), 3);
        cv::imshow(wdname, canvas);
    }

    void showPointMasked(int x, int y) {
        cv::circle(mask, cv::Point(x, y), radius, 0, -1);
        show();
    }

    void radiusUp() {
        radius += 1;
        show();
    }

    void radiusDown() {
        radius -= 1;
        show();
    }

private:
    const char *wdname;
    cv::Mat img;
    cv::Mat canvas;
    int radius = 20;
public:
    cv::Mat mask;
};

void mouse_callback(int event, int x, int y, int flags, void *userdata) {
    class CutAreaHandler *handler = (class CutAreaHandler *)userdata;

    switch (event) {
    case cv::EVENT_LBUTTONDOWN:
        handler->p0 = cv::Point(x, y);
        break;
    case cv::EVENT_LBUTTONUP:
        handler->p1 = cv::Point(x, y);
        break;
    case cv::EVENT_MOUSEMOVE:
        if (flags & cv::EVENT_FLAG_LBUTTON) {
            handler->show(x, y);
        }
        break;
    }
}

void fg_mouse_callback(int event, int x, int y, int flags, void *userdata) {
    class FgHandler *handler = (class FgHandler *)userdata;

    switch (event) {
    case cv::EVENT_LBUTTONDOWN:
        handler->showPointMasked(x, y);
        break;
    }
}

int main(int argc, char* argv[])
{
    cv::Mat image, bgModel, fgModel, result;

    cv::CommandLineParser parser(argc, argv, params);
    if (!parser.has("input")) {
        parser.printMessage();
        return -1;
    }

    image = cv::imread(parser.get<cv::String>("input"));
    if (!image.data) {
        std::cout << "Could not open or find the image" << std::endl;
        return -1;
    }
    cv::resize(image, image, cv::Size(), 0.6, 0.6, cv::INTER_AREA);

    // Generate output image
    cv::Mat background(image.size(),CV_8UC3,cv::Scalar(255,255,255));

    CutAreaHandler caHandler("Image", image);
    FgHandler fgHandler("Foreground", image);

    cv::imshow("Image", image);
    cv::setMouseCallback("Image", mouse_callback, &caHandler);

    while (1) {
        int key = cv::waitKey();

        switch (key) {
        // Up
        case 0:
            fgHandler.radiusUp();
            break;
        // Down
        case 1:
            fgHandler.radiusDown();
            break;
        case 27:
            goto EXIT;
        case 'c': {
            cv::Rect rectangle = caHandler.getRect();
            cv::grabCut(image,    // input image
                        result,   // segmentation result
                        rectangle,// rectangle containing foreground
                        bgModel,fgModel, // models
                        1,        // number of iterations
                        cv::GC_INIT_WITH_RECT); // use rectangle

            // Get the pixels marked as likely foreground
            cv::compare(result,cv::GC_PR_FGD, fgHandler.mask, cv::CMP_EQ);

            /* // draw rectangle on original image */
            cv::rectangle(image, rectangle, cv::Scalar(255,255,255),1);

            fgHandler.show();
            /* image.copyTo(background, ~(fgHandler.mask)); */
            /* cv::imshow("Background", background); */
            cv::setMouseCallback("Foreground", fg_mouse_callback, &fgHandler);
            break;
        }
        case 's': {
            char filename[32] = {};
            std::time_t t = std::time(0);
            snprintf(filename, sizeof(filename), "output_%ld.png", t);
            fgHandler.output(filename);
            goto EXIT;
        }
        }
    }
EXIT:
    cv::destroyAllWindows();
    return 0;
}
