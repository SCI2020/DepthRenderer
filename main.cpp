#include <iostream>
#include <iomanip>
#include <filesystem>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <opencv2/opencv.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

#include "Renderer.h"
#include "common/CommonIO.hpp"

using namespace std;
namespace fs = boost::filesystem;

namespace {
	// window object
	GLFWwindow *gWindow;
	// window dimensions
	int CAMERA_WIDTH = 0;
	int CAMERA_HEIGHT = 0;

	const string EXTRINSIC_PATH = "world2depth.xml";
	const string INTRINSIC_PATH = "depth_intrinsic.xml";

	// create opengl context and a window of given size
	void InitGLContext(int window_width, int window_height);
	
	// write image
	void ReadExtrinsicAndIntrinsic(const string &root, vector<string> &e, vector<string> &i,
		vector<string> &sns);
	void PrepareOutputDir(const string &output);
	void WriteImage(unsigned char *data, int width, int height, string filename);
}

int main(int argc, char **argv)
{
	/* Parse command line options */
	namespace po = boost::program_options;
	std::string GeometryFileName = "";
	std::string CameraFolder = "";
	std::string OUTPUT_DIR = "";
	bool save = false;

	// Declare the supported options
	po::options_description desc("Allowed options");
	desc.add_options()
		("help,h", "display help message")
		("geometry,g", po::value<string>(&GeometryFileName), "geometry file (*.obj)")
		("camera,c", po::value<string>(&CameraFolder), "camera folder containing RT and K")
		("output,o", po::value<string>(&OUTPUT_DIR), "output folder")
		("width,w", po::value<int>(&CAMERA_WIDTH), "camera width")
		("height,t", po::value<int>(&CAMERA_HEIGHT), "camera height")
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
	if (!vm.count("camera")) {
		cerr << "Missing camera" << endl << desc << endl;;
		return 1;
	}
	if (!vm.count("width") || !vm.count("height")) {
		cerr << "Missing camera width/height" << endl << desc << endl;
		return 1;
	}
	if (vm.count("output")) {
		save = true;
	}

	cout << "geometry  : " << GeometryFileName << endl;
	cout << "camera    : " << CameraFolder << endl;
	cout << "width     : " << CAMERA_WIDTH << endl;
	cout << "height    : " << CAMERA_HEIGHT << endl;
	cout << "output    : " << OUTPUT_DIR << endl;

	try {
		/* Get a list of camera parameters */
		vector<string> exfiles, infiles, sns;
		std::vector<Intrinsic> intrinsics;
		std::vector<Extrinsic> extrinsics; 
		
		ReadExtrinsicAndIntrinsic(CameraFolder, exfiles, infiles, sns);

		if (exfiles.size() == 0) {
			throw runtime_error("No parameters found");
		}

		// Read camera parameters
		for (size_t i = 0; i < exfiles.size(); ++i) {
			intrinsics.push_back(CommonIO::ReadIntrinsic(infiles[i], CAMERA_WIDTH, CAMERA_HEIGHT));
			extrinsics.push_back(CommonIO::ReadExtrinsic(exfiles[i]));
		}

		// set up mesh and texture
		Geometry geometry = Geometry::FromObj(GeometryFileName);

		// Initialize OpenGL context
		InitGLContext(CAMERA_WIDTH, CAMERA_HEIGHT);

		// set up renderer
		Renderer renderer;
		renderer.SetGeometries(std::vector<Geometry>(1, geometry));

		// screen shot buffer
		unsigned char *buffer = new unsigned char[CAMERA_HEIGHT*CAMERA_WIDTH*3]();

		// create directories
		if (save) {
			PrepareOutputDir(OUTPUT_DIR);
		}

		// render loop
		int camIdx = 0;

		while (!glfwWindowShouldClose(gWindow)) {
			if (camIdx >= extrinsics.size()) {
				glfwSetWindowShouldClose(gWindow, GLFW_TRUE);
				continue;
			}

			renderer.SetCamera(Camera(extrinsics[camIdx], intrinsics[camIdx]));
			renderer.Render();

			if (save) {
				// sava screen shot
				renderer.ScreenShot(buffer, 0, 0, CAMERA_WIDTH, CAMERA_HEIGHT);
				std::ostringstream oss;
				oss.fill('0');
				oss << OUTPUT_DIR << "/" << sns[camIdx] << ".png";
				WriteImage(buffer, CAMERA_WIDTH, CAMERA_HEIGHT, oss.str());
			}

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

	void ReadExtrinsicAndIntrinsic(const string & root, vector<string> &extrinsics,
		vector<string> &intrinsics, vector<string> &sns)
	{
		vector<fs::path> subfolder;
		fs::path rootPath(root);

		if (!fs::is_directory(rootPath)) {
			throw std::runtime_error(root + " is not a directory");
		}

		// detect subfolders
		for (auto &p : fs::directory_iterator(rootPath)) {
			if (!fs::is_directory(p.path())) {
				continue;
			}
			subfolder.push_back(p.path());
		}

		// enumerate each subfolder and find world2depth.xml and depth_intrinsic.xml
		for (auto subf : subfolder) {
			string extrinsic = "";
			string intrinsic = "";
			string sn = "";

			for (auto file : fs::directory_iterator(subf)) {
				if (!fs::is_regular_file(file.path())) {
					continue;
				}
				else if (file.path().filename() == EXTRINSIC_PATH) {
					extrinsic = file.path().string();
				}
				else if (file.path().filename() == INTRINSIC_PATH) {
					intrinsic = file.path().string();
				}
			}

			if (!extrinsic.empty() && !intrinsic.empty()) {
				extrinsics.push_back(extrinsic);
				intrinsics.push_back(intrinsic);
				sn = subf.filename().string();
				sns.push_back(sn);
			}
		}
	}

	void PrepareOutputDir(const string &OUTPUT_DIR)
	{
		fs::remove_all(fs::path(OUTPUT_DIR));
		fs::create_directory(fs::path(OUTPUT_DIR));
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