// Swayam Sachdeva
// March 21st 2025
// A marching cubes algo implementation
#include "Main.hpp"
using namespace std;

unsigned int vao, vbo, vboN, ebo, axes, axesbo, wireVAO, wireVBO, wireEBO, shaderProgram, axesShaderProgram, wireShaderProgram; // for all the vaos, shaders, vbos, ebos
glm::vec3 eye = glm::vec3(5.0f, 5.0f, 5.0f); // cam
glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f); // cam looking

// spherical coords
float r = glm::length(15.0f);
float theta = glm::radians(180.0f);
float phi = glm::radians(90.0f);

// cam properties
float speed = 5.0f;
double lastTime = glfwGetTime();
glm::mat4 viewMatrix;

// mouse properties
static double mouseDownX;
static double mouseDownY;
static bool firstPress = true;
double dx = 0.0, dy = 0.0;

// funct to output filename file with verticies and normals
void writePLY(
	const std::vector<float>& vertices,
	const std::vector<float>& normals,
	const std::string& fileName) {

	// making file
	ofstream fileToWrite(fileName);
	if (!fileToWrite) {
		cout << "error opening file: " << fileName << std::endl;
		return;
	}
	// header
	int numVertices = vertices.size() / 3;
	int numFaces = numVertices / 3;

	fileToWrite << "ply\n";
	fileToWrite << "format ascii 1.0\n";
	fileToWrite << "element vertex " << numVertices << "\n";
	fileToWrite << "property float x\n";
	fileToWrite << "property float y\n";
	fileToWrite << "property float z\n";
	fileToWrite << "property float nx\n";
	fileToWrite << "property float ny\n";
	fileToWrite << "property float nz\n";
	fileToWrite << "element face " << numFaces << "\n";
	fileToWrite << "property list int vertex_indices\n";
	fileToWrite << "end_header\n";

	// writing to file
	for (int i = 0; i < vertices.size(); i += 3) {
		fileToWrite << vertices[i] << " "      // x
			<< vertices[i + 1] << " "  // y
			<< vertices[i + 2] << " "; // z

		fileToWrite
			<< normals[i] << " "      // nx
			<< normals[i + 1] << " "  // ny
			<< normals[i + 2] << "\n"; // nz
	}
	// output faces
	for (int i = 0; i < numFaces; i++) {
		fileToWrite << "3 " << (i * 3) << " " << (i * 3 + 1) << " " << (i * 3 + 2) << "\n";
	}

	fileToWrite.close();
}


// keyboard movement handler
void keys(GLFWwindow* window) {

	// based on lecture slides
	double currentTime = glfwGetTime();
	float deltaTime = static_cast<float>(currentTime - lastTime);

	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
		r = glm::max(r - deltaTime * speed, 1.0f);
	}
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		r += deltaTime * speed;
	}


	// calculate new position and modify accordingly
	eye.x = r * sin(phi) * cos(theta);
	eye.y = r * cos(phi);
	eye.z = r * sin(phi) * sin(theta);

	viewMatrix = glm::lookAt(eye, target, glm::vec3(0, 1, 0));

	lastTime = currentTime;
}

// mouse input handler
void handleMouseInput(GLFWwindow* window) {
	// based on lecture slides
	int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
	if (state == GLFW_PRESS) {
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		if (firstPress) {
			// initialize the previous mouse position
			mouseDownX = xpos;
			mouseDownY = ypos;
			firstPress = false;
		}
		dx = xpos - mouseDownX;
		dy = ypos - mouseDownY;

		mouseDownX = xpos;
		mouseDownY = ypos;
		// update based on deltas
		theta += static_cast<float>(dx) * 0.01f;
		phi -= static_cast<float>(dy) * 0.01f;
		if (phi < 0.01f)        phi = 0.01f;
		if (phi > 3.13f)        phi = 3.13f;

	}
	else {
		firstPress = true; // reset for the next drag event
	}
}


// scalar field 1
float sf1(float x, float y, float z) {
	return y - sin(x) * cos(z);
}

// scalar field 2
float sf2(float x, float y, float z) {
	return x * x - y * y - z * z;
}

vector<float> marching_cubes(function<float(float, float, float)> f, float isovalue, float min, float max, float stepsize)
{
	std::vector<float> vertices;
	for (float x = min; x < max; x += stepsize) {
		for (float y = min; y < max; y += stepsize) {
			for (float z = min; z < max; z += stepsize) {
				// check each corner of the cube at x, y, z with function f()
				float results[8] = {
					f(x, y, z),                     // front bottom-left
					f(x + stepsize, y, z),           // front bottom-right
					f(x, y, z + stepsize),           // back bottom-left
					f(x + stepsize, y, z + stepsize), // back bottom-right
					f(x, y + stepsize, z),           // front top-left
					f(x + stepsize, y + stepsize, z), // front top-right
					f(x + stepsize, y + stepsize, z + stepsize), // back top-right
					f(x, y + stepsize, z + stepsize)  // back top-left
				};

				int config = 0; // which triangle config to use from lut
				if (results[0] < isovalue) {
					config |= FBL; // front bottom-left
				}
				if (results[1] < isovalue) {
					config |= FBR; // front bottom-right
				}
				if (results[2] < isovalue) {
					config |= BBL; // back bottom-left
				}
				if (results[3] < isovalue) {
					config |= BBR; // back bottom-right
				}
				if (results[4] < isovalue) {
					config |= FTL; // front top-left
				}
				if (results[5] < isovalue) {
					config |= FTR; // front top-right
				}
				if (results[6] < isovalue) {
					config |= BTR; // back top-right
				}
				if (results[7] < isovalue) {
					config |= BTL; // back top-left
				}

				int* lutResult = marching_cubes_lut[config]; // get the line segments for this triangle
				// getting verticies
				for (int i = 0; lutResult[i] != -1; i++) {
					vertices.push_back(x + stepsize * vertTable[lutResult[i]][0]);
					vertices.push_back(y + stepsize * vertTable[lutResult[i]][1]);
					vertices.push_back(z + stepsize * vertTable[lutResult[i]][2]);

				}
			}
		}
	}


	return vertices;
}

// compute normal for one triangle
void compute_triangle_normal(
	float x0, float y0, float z0,
	float x1, float y1, float z1,
	float x2, float y2, float z2,
	float& nx, float& ny, float& nz) {

	// compute vectors
	float ux = x1 - x0, uy = y1 - y0, uz = z1 - z0;
	float vx = x2 - x0, vy = y2 - y0, vz = z2 - z0;

	// perform cross product U x V
	nx = uy * vz - uz * vy;
	ny = uz * vx - ux * vz;
	nz = ux * vy - uy * vx;
}

// computing normals for given verticies
std::vector<float> compute_normals(const std::vector<float>& vertices) {
	std::vector<float> normals;

	for (int i = 0; i < vertices.size(); i += 9) { // process each triangle, each triangle has three verticies and each vertice has 3 floats, 3*3 = 9 
		float x0 = vertices[i], y0 = vertices[i + 1], z0 = vertices[i + 2];
		float x1 = vertices[i + 3], y1 = vertices[i + 4], z1 = vertices[i + 5];
		float x2 = vertices[i + 6], y2 = vertices[i + 7], z2 = vertices[i + 8];

		// get normal
		float nx, ny, nz;
		compute_triangle_normal(x0, y0, z0, x1, y1, z1, x2, y2, z2, nx, ny, nz); // passing by reference to store speed and space

		float length = std::sqrt(nx * nx + ny * ny + nz * nz);

		// normalize
		if (length > 0) // division by zero prevention
		{
			nx /= length;
			ny /= length;
			nz /= length;
		}


		// store the same normal for all three vertices
		for (int j = 0; j < 3; j++) {
			normals.push_back(nx);
			normals.push_back(ny);
			normals.push_back(nz);
		}
	}

	return normals;
}

// setup vao, vbo for verticies of mesh and their normals
// also compile and ready shaders
void setup(vector<float> verticies, vector<float> normals) {

	glGenVertexArrays(1, &vao); // create our vao obj
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo); // create and bind vbo
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, verticies.size() * sizeof(float), verticies.data(), GL_STATIC_DRAW);

	// Define vertex attribute (position)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &vboN);
	glBindBuffer(GL_ARRAY_BUFFER, vboN);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), normals.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);

	// vertex shader compilation
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	const char* vertexSource = vertexShaderSource.c_str();
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);

	// fragment shader compilation
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	const char* fragmentSource = fragmentShaderSource.c_str();
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);

	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}

// same as above but for axes lines
void setupAxes(std::vector<Vertex> axesVertices) {
	glGenVertexArrays(1, &axes);
	glGenBuffers(1, &axesbo);

	glBindVertexArray(axes);
	glBindBuffer(GL_ARRAY_BUFFER, axesbo);
	glBufferData(GL_ARRAY_BUFFER, axesVertices.size() * sizeof(Vertex), axesVertices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);                 // Position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float))); // Color
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);



	// axes vertex shader compilation
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	const char* vertexSource = axesVertexShaderSource.c_str();
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);

	// axes fragment shader compilation
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	const char* fragmentSource = axesFragmentShaderSource.c_str();
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);

	axesShaderProgram = glCreateProgram();
	glAttachShader(axesShaderProgram, vertexShader);
	glAttachShader(axesShaderProgram, fragmentShader);
	glLinkProgram(axesShaderProgram);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}
// same as other setup methods -- shaders, vaos, vbos,
void setupWireframeCube(std::vector<glm::vec3> cubeVertices, std::vector<unsigned int> cubeEdges) {
	glGenVertexArrays(1, &wireVAO);
	glBindVertexArray(wireVAO);

	glGenBuffers(1, &wireVBO);
	glBindBuffer(GL_ARRAY_BUFFER, wireVBO);
	glBufferData(GL_ARRAY_BUFFER, cubeVertices.size() * sizeof(glm::vec3), cubeVertices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &wireEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, wireEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, cubeEdges.size() * sizeof(unsigned int), cubeEdges.data(), GL_STATIC_DRAW);

	glBindVertexArray(0);

	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	const char* vertexSource = wireVertexShaderSource.c_str();
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	const char* fragmentSource = wireFragmentShaderSource.c_str();
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);

	wireShaderProgram = glCreateProgram();
	glAttachShader(wireShaderProgram, vertexShader);
	glAttachShader(wireShaderProgram, fragmentShader);
	glLinkProgram(wireShaderProgram);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);


}

// render the generated axes lines
void renderAxes() {
	glUseProgram(axesShaderProgram);
	glBindVertexArray(axes);
	glLineWidth(5.0f);
	glDrawArrays(GL_LINES, 0, 6);
	glBindVertexArray(0);
	glLineWidth(1.0f);

}

void renderMarchingCubes(int size) {
	glUseProgram(shaderProgram);  // use the shader program for cubes
	glBindVertexArray(vao);       // bind vao for the marching cubes

	glDrawArrays(GL_TRIANGLES, 0, size / 3); // draw mesh

	glBindVertexArray(0);  // la fin
}

void renderWireframe() {
	glUseProgram(wireShaderProgram);
	glBindVertexArray(wireVAO);
	glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}


int main() {

	float min = -5.0f, max = 5.0f; // max and min for program

	vector<float> verticies = marching_cubes(sf1, 0, min, max, 0.05); // get cubes
	vector<float> normals = compute_normals(verticies); // their normals
	writePLY(verticies, normals, "sf1.ply"); // write to a file
	// for wireframe cube
	std::vector<glm::vec3> cubeVertices = {
	{min, min, min}, {max, min, min}, {max, max, min}, {min, max, min}, // front face
	{min, min, max}, {max, min, max}, {max, max, max}, {min, max, max}  // back face
	};

	// the wireframe white cube that defines our space
	std::vector<unsigned int> cubeEdges = {
		0, 1, 1, 2, 2, 3, 3, 0,  // front face
		4, 5, 5, 6, 6, 7, 7, 4,  // fack face
		0, 4, 1, 5, 2, 6, 3, 7   // connecting edges
	};
	// axes lines
	std::vector<Vertex> axesVertices = {
	{min, min, min, 1, 0, 0}, {max, min, min, 1, 0, 0},  // x-axis r
	{min, min, min, 0, 1, 0}, {min, max, min, 0, 1, 0},  // y-axis g
	{min, min, min, 0, 0, 1}, {min, min, max, 0, 0, 1}   // z-axis b
	};

	// setup window and other basic stuff
	GLFWwindow* window;
	float screenW = 1920.0f, screenH = 1080.0f;

	if (!glfwInit())
		return -1;

	window = glfwCreateWindow(screenW, screenH, "3388 Assign 5", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}


	glfwMakeContextCurrent(window);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glClearColor(0.2f, 0.2f, 0.3f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glewExperimental = true;
	if (glewInit() != GLEW_OK) {
		std::cerr << "Failed to initialize GLEW" << std::endl;
		glfwTerminate();
		return -1;
	}

	// proj matrix
	glm::mat4 Projection = glm::perspective(glm::radians(45.0f), screenW / screenH, 0.1f, 100.0f);

	glm::mat4 Model = glm::mat4(1.0f);
	glm::vec3 lightDir = glm::normalize(glm::vec3(1.0f, -1.0f, -1.0f));


	// setup all the VAOs and VBOs
	setup(verticies, normals);
	setupAxes(axesVertices);
	setupWireframeCube(cubeVertices, cubeEdges);


	while (!glfwWindowShouldClose(window)) {
		// handling user input
		keys(window);
		handleMouseInput(window);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glm::mat4 Model = glm::mat4(1.0f);
		glm::mat4 MVP = Projection * viewMatrix * Model;

		glUseProgram(shaderProgram); // the mesh
		// for lighting shaders:
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "V"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
		glUniform3fv(glGetUniformLocation(shaderProgram, "LightDir"), 1, glm::value_ptr(lightDir));
		glUniform4f(glGetUniformLocation(shaderProgram, "modelColor"), 1.0f, 0.7f, 0.2f, 1.0f);

		renderMarchingCubes(verticies.size());

		glUseProgram(axesShaderProgram); // axes
		glUniformMatrix4fv(glGetUniformLocation(axesShaderProgram, "MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
		renderAxes();


		glUseProgram(wireShaderProgram); // wireframe cube
		glUniformMatrix4fv(glGetUniformLocation(wireShaderProgram, "MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
		renderWireframe();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwTerminate();

	return 0;
}