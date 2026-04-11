#include <iostream>
#include <regex>
#include <string>
#include <fstream>
#include <format>
#include <sstream>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << std::format("Usage: {} <filename>", argv[0]) << std::endl;
        return 1;
    }

    std::ifstream inputFile(argv[1]);
    std::ofstream outputFile(std::format("preprocessed_{}", argv[1]));

    if (!inputFile.is_open()) {
        std::cout << std::format("Error opening file: {}", argv[1]) << std::endl;
        return 1;
    }

    // Читаем весь файл
    std::stringstream buffer;
    buffer << inputFile.rdbuf();
    std::string content = buffer.str();

    // Удаляем комментарии
    std::regex commentRegex(R"(//.*|/\*[\s\S]*?\*/)");
    content = std::regex_replace(content, commentRegex, "");

    // Удаляем пустые строки
    std::regex emptyLines(R"(^\s*\n)", std::regex_constants::multiline);
    content = std::regex_replace(content, emptyLines, "");

    outputFile << content;

    std::cout << std::format("Result saved in preprocessed_{}", argv[1]) << std::endl;

    return 0;
}