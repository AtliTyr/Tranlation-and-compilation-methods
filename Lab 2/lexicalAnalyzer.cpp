#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <unordered_set>
#include <iomanip>
#include <fstream>
#include <sstream>

using namespace std;

// Перечисление типов лексем
enum TokenType {
    KEYWORD,
    IDENTIFIER,
    CONSTANT_INT,
    CONSTANT_REAL,
    CONSTANT_STRING,
    OPERATOR,
    DELIMITER,
    UNKNOWN
};

// Структура для хранения информации о лексеме
struct Token {
    TokenType type;
    string lexeme;
};

// Функция для получения строкового представления типа лексемы
string getTypeName(TokenType type) {
    switch (type) {
        case KEYWORD: return "KEYWORD";
        case IDENTIFIER: return "IDENTIFIER";
        case CONSTANT_INT: return "CONSTANT_INT";
        case CONSTANT_REAL: return "CONSTANT_REAL";
        case CONSTANT_STRING: return "CONSTANT_STRING";
        case OPERATOR: return "OPERATOR";
        case DELIMITER: return "DELIMITER";
        default: return "UNKNOWN";
    }
}

unordered_set<string> keywords = {
    "int", "if", "else", "for", "return", "void", "char", "float", "double", "include"
};

unordered_set<string> operators = {
    "+", "-", "*", "/", "=", "<", ">", "<=", ">=", "==", "!=", "++", "--"
};

unordered_set<string> delimiters = {
    "(", ")", "{", "}", ";", ",", "#", "."
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cout << "Usage: " << argv[0] << " <filename>" << endl;
        return 1;
    }

    ifstream inputFile(argv[1]);

    if (!inputFile.is_open()) {
        cout << "Error opening file: " << argv[1] << endl;
        return 1;
    }

    // Читаем весь файл
    stringstream buffer;
    buffer << inputFile.rdbuf();
    string code = buffer.str();

    vector<Token> tokens;
    vector<string> errors;
    size_t i = 0;

    while (i < code.length()) {
        char c = code[i];

        // 1. Пропуск пробельных символов
        if (isspace(c)) {
            i++;
            continue;
        }

        // 2. Обработка директив препроцессора (#include <stdio.h>)
        if (c == '#') {
            tokens.push_back({DELIMITER, "#"});
            i++;
            
            // Пропускаем пробелы после # (если есть)
            while (i < code.length() && isspace(code[i]) && code[i] != '\n') i++;
            
            // Читаем директиву (например, include)
            string directive = "";
            while (i < code.length() && isalpha(code[i])) {
                directive += code[i];
                i++;
            }
            if (!directive.empty() && keywords.count(directive)) {
                tokens.push_back({KEYWORD, directive}); 
            } else {
                errors.push_back("Error: Unknown preprocessor directive: " + directive);
                continue;
            }

            // Пропускаем пробелы перед <
            while (i < code.length() && isspace(code[i]) && code[i] != '\n') i++;

            // Читаем блок <stdio.h>
            if (i < code.length() && code[i] == '<') {
                tokens.push_back({OPERATOR, "<"});
                i++;
                
                string header = "";
                // Читаем всё до '>', разрешая точки и буквы
                while (i < code.length() && code[i] != '>' && code[i] != '\n') {
                    header += code[i];
                    i++;
                }
                
                if (!header.empty()) {
                    tokens.push_back({IDENTIFIER, header}); // stdio.h как единый идентификатор
                }
                
                if (i < code.length() && code[i] == '>') {
                    tokens.push_back({OPERATOR, ">"});
                    i++;
                }
            }
            continue;
        }

        // 3. Обработка строковых констант
        if (c == '"') {
            string str = "\"";
            i++;
            bool closed = false;
            while (i < code.length()) {
                str += code[i];
                if (code[i] == '"' && code[i-1] != '\\') {
                    closed = true;
                    i++;
                    break;
                }
                // Защита от перехода на новую строку внутри строки
                if (code[i] == '\n') break; 
                i++;
            }
            if (!closed) {
                errors.push_back("Error: Unclosed string literal -> " + str);
            } else {
                tokens.push_back({CONSTANT_STRING, str});
            }
            continue;
        }

        // 4. Обработка идентификаторов и ключевых слов
        if (isalpha(c) || c == '_') {
            string id = "";
            while (i < code.length() && (isalnum(code[i]) || code[i] == '_')) {
                id += code[i];
                i++;
            }
            
            // Если нашли в списке ключевых слов - это KEYWORD, иначе - любой новый IDENTIFIER
            if (keywords.count(id)) {
                tokens.push_back({KEYWORD, id});
            } else {
                tokens.push_back({IDENTIFIER, id});
            }
            continue;
        }

        // 5. Обработка числовых констант
        if (isdigit(c)) {
            string num = "";
            int dots = 0;
            bool hasLetters = false;
            
            while (i < code.length() && (isalnum(code[i]) || code[i] == '.' || code[i] == ',')) {
                if (code[i] == '.' || code[i] == ',') {
                    dots++;
                } else if (isalpha(code[i])) {
                    hasLetters = true;
                }

                if (code[i] == ',' || code[i] == '.')
                    num += '.';
                else
                    num += code[i];
                i++;
            }
            
            if (hasLetters) {
                errors.push_back("Error: Invalid character (letters in numeric constant) -> " + num);
            } else if (dots > 1) {
                errors.push_back("Error: Invalid character (two or more dots) -> " + num);
            } else if (dots == 1) {
                tokens.push_back({CONSTANT_REAL, num});
            } else {
                tokens.push_back({CONSTANT_INT, num});
            }
            continue;
        }

        // Проверка на ошибочный закрывающий комментарий без открывающего
        if (c == '*' && i + 1 < code.length() && code[i + 1] == '/') {
            errors.push_back("Error: Unmatched closing comment '*/' found without opening '/*'");
            i += 2; // Пропускаем ошибочный */
            continue;
        }

        // 6. Обработка операторов
        bool matchedOperatorOrDelimiter = false;

        if (i + 1 < code.length()) {
            string op2 = code.substr(i, 2);
            if (operators.count(op2)) {
                tokens.push_back({OPERATOR, op2});
                i += 2;
                continue;
            }
        }
        
        string single_char = string(1, c);
        
        if (operators.count(single_char)) {
            tokens.push_back({OPERATOR, single_char});
            i++;
            continue;
        }

        // 7. Обработка разделителей
        if (delimiters.count(single_char)) {
            tokens.push_back({DELIMITER, single_char});
            i++;
            continue;
        }

        // 8. Неизвестные символы
        errors.push_back("Error: Invalid character in source code -> " + string(1, c));
        i++;
    }

    cout << left << setw(15) << "Lexeme" << " | " << "Type" << endl;
    cout << "----------------+----------------------" << endl;
    
    for (const auto& token : tokens) {
        cout << left << setw(15) << token.lexeme << " | " << getTypeName(token.type) << endl;
    }
    cout << endl;

    if (errors.empty()) {
        cout << "Lexical analyzer finished successfully. Found " << tokens.size() << " tokens." << endl;
        cout << "No errors detected." << endl;

        // Формируем имя выходного файла
        string outFileName = string(argv[1]) + "_tokens.txt";
        ofstream outFile(outFileName);

        if (outFile.is_open()) {
            outFile << "[\n";
            for (size_t j = 0; j < tokens.size(); j++) {
                outFile << "  (" << getTypeName(tokens[j].type) << ", " << tokens[j].lexeme << ")";
                if (j != tokens.size() - 1) outFile << ",\n";
                else outFile << "\n";
            }
            outFile << "]\n";
            outFile.close();
            cout << "Token sequence successfully written to: " << outFileName << endl;
        } else {
            cout << "Error: Could not create output file " << outFileName << endl;
        }
    } else {
        cout << "Lexical analyzer finished. Found " << tokens.size() << " tokens." << endl;
        cout << "Detected errors (" << errors.size() << "):" << endl;
        for (const string& err : errors) {
            cout << "- " << err << endl;
        }
        cout << "Token file was NOT generated due to errors." << endl;
    }

    return 0;
}