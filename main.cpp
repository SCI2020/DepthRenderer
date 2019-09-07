#include <iostream>
#include <iomanip>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <opencv2/opencv.hpp>
#include <boost/program_options.hpp>

#include "Renderer.h"
#include "common/CommonIO.hpp"

using namespace std;

namespace {
	// window object
	GLFWwindow *gWindow;
	// window dimensions
	int WINDOW_WIDTH = 0;
	int WINDOW_HEIGHT = 0;
	const string OUTPUT_DIR = "output\\image";

	// create opengl context and a window of given size
	void InitGLContext(int window_width, int window_height);
	
	// write image
	void PrepareOutputDir(void);
	void WriteImage(unsigned char *data, int width, int height, string filename);
}

int main(int argc, char **argv)
{
	/* Parse command line options */
	namespace po = boost::program_options;
	std::string GeometryFileName = "";
	std::string IntrinsicFileName = "";
	std::string ExtrinsicFileName = "";

	// Declare the supported options
	po::options_description desc("Allowed options");
	desc.add_options()
		("help,h", "display help message")
		("geometry,g", po::value<string>(&GeometryFileName), "geometry file (*.obj)")
		("extrinsic,e", po::value<string>(&ExtrinsicFileName), "extrinsics (*.txt)")
		("intrinsic,i", po::value<string>(&IntrinsicFileName), "intrinsics (*.txt)")
		;

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);

	if (vm.count("help")) {
		cout << desc << endl;
		return 1;
	}
	if (!vm.count("geometry")) {
		cerr << "Missing geometry" << endl << desc << endl;;
		return 1;
	}
	if (!vm.count("extrinsic")) {
		cerr << "Missing extrinsic" << endl << desc << endl;;
		return 1;
	}
	if (!vm.count("intrinsic")) {
		cerr << "Missing intrinsic" << endl << desc << endl;;
		return 1;
	}

	cout << "geometry  : " << GeometryFileName << endl;
	cout << "extrinsic : " << ExtrinsicFileName << endl;
	cout << "intrinsic : " << IntrinsicFileName << endl;

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
		Renderer renderer;
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
			oss << OUTPUT_DIR << "/" << setw(4) << camIdx << ".png";
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

	return 0;
}

namespace {
	void InitGLContext(int width, int height)
	{
		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
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
		system( (string("mkdir ") + OUTPUT_DIR).c_str());
	}

	void WriteImage(unsigned char *data, int width, int height, string filename)
	{
		if (data == nullptr) {
			return;
		}

		cv::Mat image(height, width, CV_16U);
		unsigned char *src = data;
		uint16_t *dst = image.ptr<uint16_t>();

		// decode color-coded depth
		for (int i = 0; i < width * height; ++i) {
			uint8_t cr = *src;
			uint8_t cg = *(src + 1);
			//uint8_t cb = *(src + 2);

			*dst = static_cast<uint16_t>((int)cr + ((int)cg << 8));
			dst++;
			src += 3;
		}

		cv::flip(image, image, 0);
		cv::imwrite(filename, image);
	}
}