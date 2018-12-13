#include <iostream>
#define GLEW_STATIC
#include <GL\glew.h>
#include <GLFW\glfw3.h>
#include <opencv2/opencv.hpp>
#include <iomanip>
#include "Renderer.h"
#include "common/CommonIO.hpp"

using namespace std;

namespace {
	// window object
	GLFWwindow *gWindow;
	// window dimensions
	int WINDOW_WIDTH = 0;
	int WINDOW_HEIGHT = 0;

	// create opengl context and a window of given size
	void InitGLContext(int window_width, int window_height);
	
	// write image
	void PrepareOutputDir(void);
	void WriteImage(unsigned char *data, int width, int height, string filename);
}

int main(int argc, char **argv)
{
	std::string GeometryFileName = "";
	std::string IntrinsicFileName = "";
	std::string ExtrinsicFileName = "";
	float near = 0.0f;
	float far = 1000.0f;

	std::cout << "Geometry file (*.obj): ";
	std::cin >> GeometryFileName;
	std::cout << "Extrinsic file (extrinsics.txt): ";
	std::cin >> ExtrinsicFileName;
	std::cout << "Intrinsic file (intrinsics.txt): ";
	std::cin >> IntrinsicFileName;
	std::cout << "Near: ";
	std::cin >> near;
	std::cout << "Far: ";
	std::cin >> far;


	try {
		// set up mesh and texture
		Geometry geometry = Geometry::FromObj(GeometryFileName);

		// Read camera parameters
		std::vector<Intrinsic> intrinsics;
		std::vector<Extrinsic> extrinsics;
		int nIntrinsic = CommonIO::ReadIntrinsic(IntrinsicFileName, intrinsics);
		int nExtrinsic = CommonIO::ReadExtrinsic(ExtrinsicFileName, extrinsics);

		if (nIntrinsic <= 0 || nExtrinsic <= 0) {
			throw std::runtime_error("Intrinsics/Extrinsics are empty\n");
			return -1;
		}

		// Initialize OpenGL context
		WINDOW_WIDTH = intrinsics[0].GetWidth();
		WINDOW_HEIGHT = intrinsics[0].GetHeight();
		InitGLContext(WINDOW_WIDTH, WINDOW_HEIGHT);

		// set up renderer
		Renderer renderer(near, far);
		renderer.SetGeometries(std::vector<Geometry>(1, geometry));

		// screen shot buffer
		unsigned char *buffer = new unsigned char[WINDOW_HEIGHT*WINDOW_WIDTH*3]();

		// create directories
		PrepareOutputDir();

		// render loop
		int camIdx = 0;

		while (!glfwWindowShouldClose(gWindow)) {
			if (camIdx >= std::max(nIntrinsic, nExtrinsic)) {
				glfwSetWindowShouldClose(gWindow, GLFW_TRUE);
				continue;
			}

			renderer.SetCamera(Camera(extrinsics[camIdx], intrinsics[camIdx]));
			renderer.Render();
			// sava screen shot
			renderer.ScreenShot(buffer, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
			std::ostringstream oss;
			oss.fill('0');
			oss << "output/images/" << setw(4) << camIdx << ".png";
			WriteImage(buffer, WINDOW_WIDTH, WINDOW_HEIGHT, oss.str());
			// jump to next camera
			camIdx++; 
			
			glfwSwapBuffers(gWindow);
			glfwPollEvents();
		}

		// release resources
		delete[] buffer;
	}
	catch (std::exception &e) {
		std::cerr << "Catch exception: " << e.what() << endl;
	}
}

namespace {
	void InitGLContext(int width, int height)
	{
		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
		gWindow = glfwCreateWindow(width, height, "Lightfield Render Framework", nullptr, nullptr);
		glfwMakeContextCurrent(gWindow);

		glewExperimental = true;        // Needed in core profile
		GLenum err = glewInit();
		std::string error;
		if (err != GLEW_OK) {
			error = (const char *)(glewGetErrorString(err));
			throw runtime_error(error);
		}

		glDisable(GL_MULTISAMPLE);
	}

	void PrepareOutputDir(void)
	{
		system("rmdir output /S/Q");
		system("mkdir output");
		system("mkdir output\\images");
	}

	void WriteImage(unsigned char *data, int width, int height, string filename)
	{
		if (data == nullptr) {
			return;
		}

		cv::Mat image(cv::Size(width, height), CV_8UC3, data);
		cv::cvtColor(image, image, CV_RGB2BGR);
		cv::flip(image, image, 0);
		cv::imwrite(filename, image);
	}
}