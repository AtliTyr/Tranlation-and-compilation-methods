#include <iostream>
#include <regex>
#include <string>
#include <fstream>
#include <format>
#include <sstream>
#include <vector>

bool checkForErrors(std::regex& pattern, const std::string& content, std::string_view errorType) {
    auto unclosed_begin = std::sregex_iterator(content.begin(), content.end(), pattern);
    auto unclosed_end = std::sregex_iterator();
    for (auto it = unclosed_begin; it != unclosed_end; ++it) {
        std::smatch match = *it;
        int line = 1, col = 1;
        for (int i = 0; i < match.position(); ++i) {
            if (content[i] == '\n') {
                line++;
                col = 1;
            } else {
                col++;
            }
        }
        std::cout << std::format("Line {}, Col {}: {} detected\n", line, col, errorType);
        return true;
    }
    return false;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << std::format("Usage: {} <filename>", argv[0]) << std::endl;
        return 1;
    }

    std::ifstream inputFile(argv[1]);

    if (!inputFile.is_open()) {
        std::cout << std::format("Error opening file: {}", argv[1]) << std::endl;
        return 1;
    }

    // Читаем весь файл
    std::stringstream buffer;
    buffer << inputFile.rdbuf();
    std::string content = buffer.str();

    // Незакрытые комментарии
    std::regex unclosedComment(R"(/\*((?!\*/)[\s\S])*$)");
    if (checkForErrors(unclosedComment, content, "Unclosed comment")) {
        std::cout << "Errors found. Preprocessing aborted.\n";
        return 1;
    }
    // Недопустимые символы
    std::regex badChars(R"([\x00-\x08\x0B\x0C\x0E-\x1F\x7F])");
    if (checkForErrors(badChars, content, "Invalid character")) {
        std::cout << "Errors found. Preprocessing aborted.\n";
        return 1;
    }


    // Удаляем комментарии
    std::regex commentRegex(R"(//.*|/\*[\s\S]*?\*/)");
    content = std::regex_replace(content, commentRegex, " ");

    // Удаляем пустые строки
    std::regex emptyLines(R"(^\s*\n)", std::regex_constants::multiline);
    content = std::regex_replace(content, emptyLines, "");

    // Удаляем конечные пробелы
    std::regex endSpaces(R"(\s*\n)", std::regex_constants::multiline);
    content = std::regex_replace(content, endSpaces, "\n");

    // Удаляем начальные пробелы
    std::regex startSpaces(R"(^\s*)", std::regex_constants::multiline);
    content = std::regex_replace(content, startSpaces, "");


    std::ofstream outputFile(std::format("preprocessed_{}", argv[1]));
    outputFile << content;

    std::cout << std::format("Result saved in preprocessed_{}", argv[1]) << std::endl;

    return 0;
}