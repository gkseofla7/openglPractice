#include <GL/glew.h> 
#include <GL/glut.h> // or <GL/freeglut.h>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <glm/gtc/matrix_transform.hpp>
#include<iostream>

GLuint LoadShaders(const char* vertex_file_path, const char* fragment_file_path);

GLuint programID;
GLuint VertexBufferID;
GLuint ColorBufferID;
GLuint NormalBufferID;

GLuint MatrixID;
GLuint ViewMatrixID;
GLuint ModelMatrixID;
GLuint LightID;

glm::mat4 Projection;
glm::mat4 View;
glm::mat4 Model;
glm::mat4 mvp;
std::vector<glm::vec3> v_vertices;
float angle;
int s = 2;
float radius;
float dist_x;
float dist_y;
float dist_z;
void timer(int value) {
    angle += glm::radians(5.0f);
    radius += 0.013f;
    dist_x -= 0.0002f;
    dist_y -= 0.0002f;
    dist_z -= 0.0002f;
    glutPostRedisplay();
    glutTimerFunc(20, timer, 0);
}
bool loadOBJ(const char* path, std::vector<glm::vec3>& vertices)
{
    std::vector<unsigned int> vertexIndices;
    std::vector<glm::vec3> temp_vertices;

    FILE* file = NULL;

    if (fopen_s(&file, path, "r") != NULL) {
        printf("File can't be opened\n");
        return false;
    }

    while (true) {
        char lineHeader[128];
        // read the first word of the line
        if (fscanf_s(file, "%s", lineHeader, 128) == EOF)
            break; // EOF = End Of File. Quit the loop.

        // else : parse lineHeader
        if (strcmp(lineHeader, "v") == 0) {
            glm::vec3 vertex;
            fscanf_s(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
            temp_vertices.push_back(vertex);
        }

        else if (strcmp(lineHeader, "f") == 0) {
            unsigned int vertexIndex[3];
            int matches = fscanf_s(file, "%d %d %d\n", &vertexIndex[0], &vertexIndex[1], &vertexIndex[2]);
            if (matches != 3) {
                printf("File can't be read\n");
                return false;
            }
            vertexIndices.push_back(vertexIndex[0]);
            vertexIndices.push_back(vertexIndex[1]);
            vertexIndices.push_back(vertexIndex[2]);
        }
    }

    fclose(file);

    for (unsigned int i = 0; i < vertexIndices.size(); i++) {
        unsigned int vertexIndex = vertexIndices[i];
        glm::vec3 vertex = temp_vertices[vertexIndex - 1];
        vertices.push_back(vertex);
    }

}


void init()
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);
    
    if(s==0)
        loadOBJ("tetrahedron.obj", v_vertices);
    else if(s==1)
        loadOBJ("teapot.obj", v_vertices);
    else if (s == 2)
        loadOBJ("bones.obj", v_vertices);
    glm::vec3* vertices = new glm::vec3[v_vertices.size()];
    
    for (int i = 0; i < v_vertices.size(); i++)
    {
        vertices[i] = v_vertices[i];
    }
    
    glGenBuffers(1, &VertexBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0])*v_vertices.size(), vertices, GL_STATIC_DRAW);

    glm::vec3* colors = new glm::vec3[v_vertices.size()];
    for (int i = 0; i < v_vertices.size(); i++)
    {
        colors[i] = glm::vec3(1.0f,1.0f,1.0f);
    }
    glGenBuffers(1, &ColorBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, ColorBufferID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colors[0])*v_vertices.size(), colors, GL_STATIC_DRAW);

    auto getNormal = [](const glm::vec3& point1, const glm::vec3& point2, const glm::vec3& point3)
    {
        glm::vec3 edge1 = point2 - point1;
        glm::vec3 edge2 = point3 - point2;
        return glm::normalize(glm::cross(edge1, edge2));
    };

    glm::vec3 * normals = new glm::vec3[v_vertices.size()];
    for (int i = 0; i < v_vertices.size()/3; i++)
    {
        glm::vec3 n = getNormal(vertices[i], vertices[i + 1], vertices[i + 2]);
        for (int j = 0; j < 3; j++)
        {
            normals[i + j] = n;
        }
    }

    glGenBuffers(1, &NormalBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, NormalBufferID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(normals[0])*v_vertices.size(), normals, GL_STATIC_DRAW);

    programID = LoadShaders("simple.vshader", "simple.fshader");

    MatrixID = glGetUniformLocation(programID, "MVP");
    ViewMatrixID = glGetUniformLocation(programID, "V");
    ModelMatrixID = glGetUniformLocation(programID, "M");
    LightID = glGetUniformLocation(programID, "LightPosition");
    glUseProgram(programID);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
}

void transform() {
    Model = glm::mat4(1.0f);
    Model = glm::rotate(Model, angle, glm::vec3(0, 1, 0));
    Model = glm::translate(Model, glm::vec3(radius, 0, 0));
    Model = glm::rotate(Model, angle, glm::vec3(0, 1, 0));

    View = glm::translate(View, glm::vec3(dist_x, dist_y, dist_z));
    mvp = Projection * View * Model;

    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);
    glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &View[0][0]);
    glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &Model[0][0]);
    glm::vec3 lightPos = glm::vec3(100, 100, 100);
    glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
}

void myreshape(int w, int h) {
    glViewport(0, 0, w, h);

    Projection = glm::perspective(glm::radians(45.0f),
        (float)w / (float)h, 0.1f, 100.0f);

    View = glm::lookAt(
        glm::vec3(3, 4, 3), // Camera is at (3,4,3), in World Space
        glm::vec3(0, 0, 0), // and looks at the origin
        glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
    );
    transform();
}
void mydisplay() {
    transform();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, ColorBufferID);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, NormalBufferID);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glDrawArrays(GL_TRIANGLES, 0, v_vertices.size());
    glDisableVertexAttribArray(0);

    glutSwapBuffers();
}


int main(int argc, char** argv)
{
    std::cout << "type 0 to terhedron, type 1 to teapot, type 2 to bones" << std::endl;
    std::cin >> s;
    glutInit(&argc, argv);
    glutInitWindowSize(400, 400);
    glutInitWindowPosition(500, 500);
    glutCreateWindow("16100420 �Ѵ븲");
    glutInitDisplayMode(GLUT_DEPTH | GLUT_SINGLE | GLUT_RGBA);
    glutTimerFunc(0, timer, 0);
    glutDisplayFunc(mydisplay);
    glutReshapeFunc(myreshape);

    GLenum err = glewInit();
    if (err == GLEW_OK) {
        init();
        glutMainLoop();
    }
}





GLuint LoadShaders(const char* vertex_file_path, const char* fragment_file_path) {

    // Create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    // Read the Vertex Shader code from the file
    std::string VertexShaderCode;
    std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
    if (VertexShaderStream.is_open()) {
        std::stringstream sstr;
        sstr << VertexShaderStream.rdbuf();
        VertexShaderCode = sstr.str();
        VertexShaderStream.close();
    }
    else {
        printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertex_file_path);
        getchar();
        return 0;
    }

    // Read the Fragment Shader code from the file
    std::string FragmentShaderCode;
    std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
    if (FragmentShaderStream.is_open()) {
        std::stringstream sstr;
        sstr << FragmentShaderStream.rdbuf();
        FragmentShaderCode = sstr.str();
        FragmentShaderStream.close();
    }
    GLint Result = GL_FALSE;
    int InfoLogLength;

    // Compile Vertex Shader
    printf("Compiling shader : %s\n", vertex_file_path);
    char const* VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
    glCompileShader(VertexShaderID);

    // Check Vertex Shader
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0) {
        std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
        glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
        printf("%s\n", &VertexShaderErrorMessage[0]);
    }

    // Compile Fragment Shader
    printf("Compiling shader : %s\n", fragment_file_path);
    char const* FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
    glCompileShader(FragmentShaderID);

    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0) {
        std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
        glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
        printf("%s\n", &FragmentShaderErrorMessage[0]);
    }
    // Link the program
    printf("Linking program\n");
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);

    // Check the program
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0) {
        std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
        glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
        printf("%s\n", &ProgramErrorMessage[0]);
    }

    glDetachShader(ProgramID, VertexShaderID);
    glDetachShader(ProgramID, FragmentShaderID);

    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);

    return ProgramID;
}
