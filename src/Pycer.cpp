#include<fstream>
#include<string>
#include<vector>
#include<iostream>

using std::string;
using std::vector;

#define RETRACT "    "

const size_t BUFFER_STORAGE = 1024;

char getBracket(const char c) {
    switch (c)
    {
    case ')':
        return '(';
        break;
    
    case ']':
        return '[';
        break;
    
    case '}':
        return '{';
        break;

    default:
        return c;
        break;
    }
}

class Stack {
    private:
        char _stack[1024];
        size_t cur_ptr;

    public:
        Stack() {
            cur_ptr = 0;
            for (size_t i = 0; i < 1024; ++i) {
                _stack[i] = char(0);
            }
        }

        void push(char c) {

            if (cur_ptr >= 1023) {
                std::cerr << "Stack Buffer Overload." << std::endl;
                return;
            }

            _stack[cur_ptr++] = c;
        }

        bool match(char c) {
            if (c == _stack[cur_ptr-1]) {
                _stack[--cur_ptr] = char(0);
                return true;
            }
            return false;
        }

        void clear() {
            for (size_t i = 0; i < 1024; ++i) {
                _stack[i] = char(0);
            }
            cur_ptr = 0;
        }

        bool empty() {
            return cur_ptr == 0;
        }

        size_t deepth() {
            return cur_ptr;
        }

        char top() {
            if (cur_ptr > 0) {
                return _stack[cur_ptr-1];
            } else {
                return char(0);
            }
        }
};

class FileHandler {

    private:
        std::ifstream fin;
        std::ofstream fout;

        bool in_string;
        bool in_no_parse_area;
        bool is_after_bracket_keyword;
        bool is_after_explicit_brafree_keyword;
        size_t retract;

        Stack parse_stack;

        bool isSeperateKeyword(char c) {
            return c == '{' || c == '}' ||
                   c == '[' || c == ']' ||
                   c == '(' || c == ')' ||
                   c == '=' || c == '+' || c == '-' || c == '*' || c == '/' || c== '%';
        }

        bool isDoubleKeyword(string &line, string::iterator ptr_c) {
            return ((*ptr_c == '+' || *ptr_c == '-' || *ptr_c == '*' || *ptr_c == '/' || *ptr_c == '%' || *ptr_c == '=') &&
                    (ptr_c+1 != line.end() && *(ptr_c+1) == '=')) ||
                    (*ptr_c == '/' && ptr_c+1 != line.end() && *(ptr_c+1) == '/');
        }

        bool matchTripleSingleDocString(string &line, string::iterator ptr_c) {
            return ptr_c != line.end() && ptr_c+1 != line.end() && ptr_c+2 != line.end() &&
                    *ptr_c == '\'' && *(ptr_c+1) == '\'' && *(ptr_c+2) == '\'';
        }

        bool matchTripleDoubleDocString(string &line, string::iterator ptr_c) {
            return ptr_c != line.end() && ptr_c+1 != line.end() && ptr_c+2 != line.end() &&
                    *ptr_c == '\"' && *(ptr_c+1) == '\"' && *(ptr_c+2) == '\"';
        }
        
        // 将一行的字符串分解成单词列表  (一个函数写一百来行怎么了？)
        vector<string> lineSplit(string &line) {
            string cur_word;
            vector<string> res;
            bool is_fstring = false;
            bool is_double_keyword = false;
            bool is_escape_character = false;
            for (auto ptr_c = line.begin(); ptr_c != line.end(); ++ptr_c) {
                // 注释
                if (*ptr_c == '#') {
                    cur_word.clear();
                    break;
                }
                // 处理字符串内部
                if (in_string) {
                    // 跳过转义字符的所有可能解析操作
                    if (is_escape_character) {
                        cur_word += *ptr_c;
                        is_escape_character = false;
                        continue;
                    }
                    // 转义字符
                    if (*ptr_c == '\\') {
                        cur_word += '\\';
                        is_escape_character = true;
                        continue;
                    }


                    // 三单引号
                    if (matchTripleSingleDocString(line, ptr_c) && parse_stack.match('`')){
                        ptr_c += 2;
                        in_string = false;
                        res.push_back(cur_word);
                        cur_word.clear();

                        cur_word += '`';
                        res.push_back(cur_word);
                        cur_word.clear();
                        continue;
                    }
                    // 三双引号
                    if (matchTripleDoubleDocString(line, ptr_c) && parse_stack.match('~')) {
                        ptr_c += 2;
                        in_string = false;
                        res.push_back(cur_word);
                        cur_word.clear();

                        cur_word += '~';
                        res.push_back(cur_word);
                        cur_word.clear();
                        continue;
                    }
                    // 其他引号
                    if (parse_stack.match(*ptr_c)) {
                        in_string = false;
                        cur_word += *ptr_c;
                        res.push_back(cur_word);
                        cur_word.clear();
                        continue;
                    }

                    cur_word += *ptr_c;
                    continue;
                }
                // f-string
                if (*ptr_c == 'f' && ptr_c+1 != line.end() &&
                    (*(ptr_c+1) == '\'' || *(ptr_c+1) == '\"')) {
                        is_fstring = true;
                        cur_word += *ptr_c;
                        continue;
                }

                // 普通空格分词
                if (*ptr_c == ' ') {
                    if (cur_word.empty()) {
                        continue;
                    }
                    res.push_back(cur_word);
                    cur_word.clear();
                    continue;
                }
                // 三单引号进入字符串
                if (matchTripleSingleDocString(line, ptr_c)) {
                    // 如果buffer里有字符就先分词
                    if (!is_fstring && !cur_word.empty()) {
                        res.push_back(cur_word);
                        cur_word.clear();
                    } else if (is_fstring) {
                        is_fstring = false;
                    }
                    
                    cur_word += '`';
                    res.push_back(cur_word);
                    cur_word.clear();

                    parse_stack.push('`');
                    in_string = true;
                    ptr_c += 2; // 跳过三引号
                    continue;
                }
                // 三双引号进入字符串
                if (matchTripleDoubleDocString(line, ptr_c)) {
                    // 如果buffer里有字符就先分词
                    if (!is_fstring && !cur_word.empty()) {
                        res.push_back(cur_word);
                        cur_word.clear();
                    } else if (is_fstring) {
                        is_fstring = false;
                    }
                    
                    cur_word += '~';
                    res.push_back(cur_word);
                    cur_word.clear();
                    
                    parse_stack.push('~');
                    in_string = true;
                    ptr_c += 2; // 跳过三引号
                    continue;
                }
                // 普通引号进入字符串
                if (*ptr_c == '\'' || *ptr_c == '\"') {
                    // 如果buffer里有字符就先分词
                    if (!is_fstring && !cur_word.empty()) {
                        res.push_back(cur_word);
                        cur_word.clear();
                    } else if (is_fstring) {
                        is_fstring = false;
                    }

                    parse_stack.push(*ptr_c);
                    in_string = true;
                    cur_word += *ptr_c;
                    continue;
                }
              
                // 非空格的分词

                // 先考虑诸如 == += 的运算符
                if (is_double_keyword) {
                    is_double_keyword = false;
                    cur_word += *ptr_c;
                    res.push_back(cur_word);
                    cur_word.clear();
                    continue;
                }

                is_double_keyword = isDoubleKeyword(line, ptr_c);

                if (!is_double_keyword && !cur_word.empty() && isSeperateKeyword(*ptr_c)) {

                    res.push_back(cur_word);
                    cur_word.clear();

                    cur_word += *ptr_c;

                    res.push_back(cur_word);
                    cur_word.clear();

                } else if (is_double_keyword) {

                    res.push_back(cur_word);
                    cur_word.clear();

                    cur_word += *ptr_c;

                } else if (isSeperateKeyword(*ptr_c)) {

                    cur_word += *ptr_c;

                    res.push_back(cur_word);
                    cur_word.clear();

                } 
                
                else {

                    cur_word += *ptr_c;

                }

            }

            if (!cur_word.empty()) {
                res.push_back(cur_word);
            }

            return res;
        }

        void compileSplittedWordsToFile(vector<string> &cur_line_words) {
            // assert lambda return yeild : expected non-grammarly parsers
            // inside "()", "[]", "\"\"", "\'\'" is no parser area
            // if elif for while except : require "()" after these keywords

            // 先缩进
            for (int i = 0; i < retract; ++i) {
                fout << RETRACT;
            }
            
            // 将文本分为解析区域和非解析区域，用布尔变量in_no_parse_areat控制;
            // 在解析区域中需要：
            // 1. 查找大括号，以此控制缩进;
            // 2. 查找"(", "["字符，来进入非解析区域.
            // 3. 特别地，查找需要额外语法小括号的关键词if elif for while except, 在输出文件中删去语法小括号
            // 4. 查找可能显式写出大括号(集合、字典)的关键字，包括assert lambda return yield
            // 在非解析区域中，只需要监控解析栈顶字符来控制是否退出非解析区域

            for (auto ptr_word = cur_line_words.begin(); 
                 ptr_word != cur_line_words.end(); 
                 ++ptr_word) {
                    // 非解析区域直接写入
                    if (in_no_parse_area) {
                        // 显式无括号关键字后, 遇到分号结束非解析区域
                        if (is_after_explicit_brafree_keyword && *ptr_word == ";") {
                            fout << "\n";
                            in_no_parse_area = false;
                            is_after_explicit_brafree_keyword = false;
                            continue;
                        }
                        // 在额外括号关键词后的小括号, 不需要被输出到文件中, 退出非解析区域
                        if (is_after_bracket_keyword && 
                            ptr_word->length() == 1 &&
                            parse_stack.match(getBracket(*(ptr_word->c_str())))) {
                                fout << ' ';
                                in_no_parse_area = false;
                                is_after_bracket_keyword = false;
                                continue;
                        }
                        // 普通括号, 退出非解析区域
                        if (ptr_word->length() == 1 &&
                            parse_stack.match(getBracket(*(ptr_word->c_str())))) {

                                if (*ptr_word == "`") {
                                    fout << "\'\'\'" << ' ';
                                } else if (*ptr_word == "~") {
                                    fout << "\"\"\"" << ' ';
                                } else {
                                    fout << *ptr_word << ' ';
                                }

                                in_no_parse_area = false;
                                continue;
                        }

                        fout << *ptr_word << " ";
                        continue;
                    }
                    // 解析区域
                    if (*ptr_word == ";") {
                        fout << "\n";
                        continue;
                    }

                    if (*ptr_word == "{") { // 这意味着大括号不能换行(好耶!)
                        fout << ": ";
                        ++retract;
                        continue;
                    }

                    if (*ptr_word == "}") {
                        --retract;
                        continue;
                    }

                    if (*ptr_word == "if" || *ptr_word == "elif" ||
                        *ptr_word == "while" || *ptr_word == "for" ||
                        *ptr_word == "except") {
                            fout << *ptr_word << ' ';
                            is_after_bracket_keyword = true;
                            continue;
                    }

                    if (*ptr_word == "assert" || *ptr_word == "lambda" ||
                        *ptr_word == "return" || *ptr_word == "yield") {
                            fout << *ptr_word << ' ';
                            in_no_parse_area = true;
                            is_after_explicit_brafree_keyword = true;
                            continue;
                    }

                    if (is_after_bracket_keyword && 
                        (*ptr_word == "(" || *ptr_word == "[")) {
                            fout << ' ';
                            parse_stack.push(*(ptr_word->c_str()));
                            in_no_parse_area = true;
                            continue;
                    }

                    if (*ptr_word == "(" || *ptr_word == "[") {
                        fout << *ptr_word << ' ';
                        parse_stack.push(*(ptr_word->c_str()));
                        in_no_parse_area = true;
                        continue;
                    }

                    if (*ptr_word == "`") {
                        fout << "\'\'\'" << ' ';
                        parse_stack.push(*(ptr_word->c_str()));
                        in_no_parse_area = true;
                        continue;
                    }

                    if (*ptr_word == "~") {
                        fout << "\"\"\"" << ' ';
                        parse_stack.push(*(ptr_word->c_str()));
                        in_no_parse_area = true;
                        continue;
                    }

                    fout << *ptr_word << ' ';
            }
            fout << "\n";
        }

    public:
        FileHandler() {
            in_string = false;
            in_no_parse_area = false;
            is_after_bracket_keyword = false;
            is_after_explicit_brafree_keyword = false;
            retract = 0;
        }

        void operator>>(std::string &file_path) {
            if (file_path.length() < 5 ||  // 文件名至少要5个字符, 如1.pyp
                file_path.substr(file_path.length()-4) != ".pyp") {
                std::cerr << "Invalid File Name." << file_path << std::endl;
            }
            
            fin.open(file_path, std::ios::in);
            fout.open(file_path.substr(0, file_path.length()-4)+".py");
            
        }
        
        void parserCompile() {

            char buffer[BUFFER_STORAGE] = {0};
            string str_buffer;
            vector<string> cur_line_words;
            while (fin.getline(buffer, BUFFER_STORAGE)) {
                str_buffer = buffer;
                cur_line_words = lineSplit(str_buffer);

                compileSplittedWordsToFile(cur_line_words);

                cur_line_words.clear();
            }

            if (!parse_stack.empty()) {
                std::cerr << "Unclosed Bracket or Docstring" << std::endl;
            }
        }
};

int main(int argc, char* argv[]) {
    
    FileHandler fh;
    string file_path(argv[1]);
    fh >> file_path;
    fh.parserCompile();
    return 0;

}