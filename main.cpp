#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

const int WINDOW_WIDTH = 640;
const int WINDOW_HEIGHT = 480;
const float AXIS_LENGTH = 1.0f;
const float CONTROL_POINT_SIZE = 5.0f;
const float CURVE_COLOR[3] = { 0.0f, 1.0f, 0.0f }; // Cor verde para a curva original
const float TRANSFORMED_CURVE_COLOR[3] = { 1.0f, 0.0f, 0.0f }; // Cor vermelha para a curva transformada
bool displayControlPolygon = false; // Flag para controlar a exibição do polígono de controle

GLFWwindow* window;
std::vector<std::pair<float, float>> controlPoints;
std::vector<std::pair<char, std::pair<float, float>>> transformations;
std::vector<std::pair<float, float>> desenhoTransformado;
std::vector<std::pair<float, float>> desenhoAntesDeTransformar;

int transformationIndex = 0; // Índice da última transformação aplicada

// Função para calcular um ponto na curva de Bezier
std::pair<float, float> calculateBezierPoint(const std::vector<std::pair<float, float>>& controlPoints, float t) {
    int n = controlPoints.size() - 1;
    float x = 0.0f;
    float y = 0.0f;

    // Cálculo do ponto na curva de Bezier usando a fórmula de Bernstein
    for (int i = 0; i <= n; ++i) {
        float coefficient = static_cast<float>(std::pow(1 - t, n - i) * std::pow(t, i));
        x += coefficient * controlPoints[i].first;
        y += coefficient * controlPoints[i].second;
    }

    return { x, y };
}

// Função para aplicar a translação a um conjunto de pontos
void applyTranslation(std::vector<std::pair<float, float>>& points, float translationX, float translationY) {
    for (auto& point : points) {
        // Aplicar a translação a cada ponto
        point.first += translationX;
        point.second += translationY;
    }
}


// Função para desenhar os eixos X e Y
void drawAxes() {
    // Eixo X (verde)
    glColor3f(0.0f, 1.0f, 0.0f);
    glBegin(GL_LINES);
    glVertex2f(-AXIS_LENGTH, 0.0f);
    glVertex2f(AXIS_LENGTH, 0.0f);
    glEnd();

    // Eixo Y (azul)
    glColor3f(0.0f, 0.0f, 1.0f);
    glBegin(GL_LINES);
    glVertex2f(0.0f, -AXIS_LENGTH);
    glVertex2f(0.0f, AXIS_LENGTH);
    glEnd();
}

// Função para desenhar os pontos de controle
void drawControlPoints() {
    glColor3f(1.0f, 1.0f, 1.0f); // Cor branca
    glPointSize(CONTROL_POINT_SIZE);
    glBegin(GL_POINTS);
    for (const auto& point : controlPoints) {
        glVertex2f(point.first, point.second);
    }
    glEnd();
}

// Função para desenhar a curva de Bezier
void drawBezierCurve(const std::vector<std::pair<float, float>>& points, const float color[3]) {
    glColor3fv(color);
    glBegin(GL_LINE_STRIP);
    for (float t = 0; t <= 1; t += 0.01f) {
        auto point = calculateBezierPoint(points, t);
        glVertex2f(point.first, point.second);
    }
    glEnd();
}

// Função para aplicar uma transformação 2D a um conjunto de pontos
void applyTransformation(const std::vector<std::pair<float, float>>& drawForTransfomation, std::vector<std::pair<char, std::pair<float, float>>> vectorTransformation) {
    if (vectorTransformation[transformationIndex].first == 't') {
        applyTranslation();
    }
    for (const auto& point : points) {
        // Aplicar a transformação de translação
        float translatedX = point.first + translation.first;
        float translatedY = point.second + translation.second;

        // Aplicar a transformação de rotação
        float rotatedX = translatedX * cos(rotationAngle) - translatedY * sin(rotationAngle);
        float rotatedY = translatedX * sin(rotationAngle) + translatedY * cos(rotationAngle);

        transformedPoints.emplace_back(rotatedX, rotatedY);


    }
}

// Função para carregar pontos de controle e transformações de um arquivo .obj
void loadControlPointsAndTransformations(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Erro ao abrir o arquivo: " << filename << std::endl;
        return;
    }
    



    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        char type;
        float x, y;
        std::istringstream iss(line);
        iss >> type;

        if (type == 'v') {
            iss >> x >> y;
            controlPoints.emplace_back(x, y);
        }
        else if (type == 't') {
            std::vector<std::pair<float, float>> transformation;
            float translationX, translationY, rotationAngle;
            iss >> translationX >> translationY >> rotationAngle;
            transformation.emplace_back(translationX, translationY);
            transformation.emplace_back(rotationAngle, 0.0f); // O segundo componente é o ângulo de rotação
            transformations.push_back(transformation);
        }
    }

    file.close();

    if (controlPoints.empty()) {
        std::cerr << "Nenhum ponto de controle foi carregado do arquivo." << std::endl;
    }
    else {
        std::cout << "Pontos de controle carregados com sucesso." << std::endl;
    }
}

// Função de callback para redimensionar a janela
void resizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// Função de callback para o pressionamento de teclas
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_P && action == GLFW_PRESS) {
        displayControlPolygon = !displayControlPolygon;
    }if((key == GLFW_KEY_Q || key == GLFW_KEY_ESCAPE) && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        applyTransformation(desenhoTransformado, transformations);
    }
}


// Função principal de renderização
void render(GLFWwindow* window) {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    glClear(GL_COLOR_BUFFER_BIT);

    // Ajuste da projeção perspectiva
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if (width <= height) {
        glOrtho(-AXIS_LENGTH, AXIS_LENGTH, -AXIS_LENGTH * height / width, AXIS_LENGTH * height / width, -1.0, 1.0);
    }
    else {
        glOrtho(-AXIS_LENGTH * width / height, AXIS_LENGTH * width / height, -AXIS_LENGTH, AXIS_LENGTH, -1.0, 1.0);
    }
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Desenhar os eixos X e Y
    drawAxes();

    // Desenhar os pontos de controle
    drawControlPoints();

    // Desenhar a curva de Bezier original
    drawBezierCurve(controlPoints, CURVE_COLOR);

    // Desenhar a curva transformada (se houver)
    if (!transformedPoints.empty()) {
        drawBezierCurve(transformedPoints, TRANSFORMED_CURVE_COLOR);
    }

    // Exibir o polígono de controle, se necessário
    if (displayControlPolygon) {
        glColor3f(1.0f, 1.0f, 0.0f); // Cor amarela
        glBegin(GL_LINE_LOOP);
        for (const auto& point : controlPoints) {
            glVertex2f(point.first, point.second);
        }
        glEnd();
    }

    glfwSwapBuffers(window);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Uso: " << argv[0] << "pontos_Gato.obj" << std::endl;
        return -1;
    }
 
    loadControlPointsAndTransformations(argv[1]);

    if (controlPoints.empty()) {
        std::cerr << "Arquivo de pontos de controle vazio." << std::endl;
        return -1;
    }

    if (!glfwInit()) {
        std::cerr << "Erro ao inicializar GLFW" << std::endl;
        return -1;
    }

    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Visualizador de Curvas de Bezier", NULL, NULL);
    if (!window) {
        std::cerr << "Erro ao criar janela GLFW" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, resizeCallback);
    glfwSetKeyCallback(window, keyCallback);

    while (!glfwWindowShouldClose(window)) {
        render(window);
        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
}