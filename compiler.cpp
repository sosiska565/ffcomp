#include<iostream>
#include<string>
#include<vector>
#include<fstream>
#include<clocale>
#include<cstdlib>
#include<sstream>

std::vector<std::string> cfile = {
    "#include<iostream>\n",
    "#include<vector>\n",
    "#include<string>\n",
    "int main(){\n",
};

bool DEBUG_MODE = false;

void debugLog(const std::string& message) {
    if (DEBUG_MODE) {
        std::cout << "[DEBUG] " << message << std::endl;
    }
}

std::string checkCommand(int line, int symbol, std::vector<std::string> *lines);
std::string putVar(int line, int symbol, std::vector<std::string> *lines);
std::string printVar(int line, int symbol, std::vector<std::string> *lines);
std::string inputVar(int line, int symbol, std::vector<std::string> *lines);
std::string conditionIfVar(int line, int symbol, std::vector<std::string> *lines);
std::string conditionElseVar(int line, int symbol, std::vector<std::string> *lines);
std::string outputVar(int line, int symbol, std::vector<std::string> *lines);
std::string comment(int line, int symbol, std::vector<std::string> *lines);
std::string whileRound(int line, int symbol, std::vector<std::string> *lines);

std::string trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) return "";
    size_t end = str.find_last_not_of(" \t\n\r");
    return str.substr(start, end - start + 1);
}

// Находит соответствующую закрывающую скобку с учётом вложенности
size_t findMatchingBracket(const std::string& str, size_t openPos) {
    int level = 0;
    bool inString = false;
    
    for(size_t i = openPos; i < str.size(); i++) {
        if(str[i] == '\"') {
            inString = !inString;
        }
        
        if(!inString) {
            if(str[i] == '[') {
                level++;
            } else if(str[i] == ']') {
                level--;
                if(level == 0) {
                    return i;
                }
            }
        }
    }
    
    return std::string::npos;
}

std::vector<std::string> splitCommands(const std::string& line) {
    std::vector<std::string> commands;
    std::string current;
    bool inString = false;
    int bracketLevel = 0;
    
    for(size_t i = 0; i < line.size(); i++) {
        if(line[i] == '\"') {
            inString = !inString;
        }
        
        if(!inString) {
            if(line[i] == '[') {
                bracketLevel++;
            } else if(line[i] == ']') {
                bracketLevel--;
            }
        }
        
        if(line[i] == ';' && !inString && bracketLevel == 0) {
            if(!current.empty()) {
                std::string trimmed = trim(current);
                if(!trimmed.empty()) {
                    commands.push_back(trimmed);
                }
                current.clear();
            }
        } else {
            current += line[i];
        }
    }
    
    if(!current.empty()) {
        std::string trimmed = trim(current);
        if(!trimmed.empty()) {
            commands.push_back(trimmed);
        }
    }
    
    debugLog("splitCommands: line='" + line + "' -> " + std::to_string(commands.size()) + " commands");
    for(size_t i = 0; i < commands.size(); i++) {
        debugLog("  Command " + std::to_string(i) + ": '" + commands[i] + "'");
    }
    
    return commands;
}

std::string parseBracketCommands(const std::string& bracketContent) {
    std::string result;
    std::vector<std::string> commands = splitCommands(bracketContent);
    
    debugLog("parseBracketCommands: bracketContent='" + bracketContent + "'");
    
    for(const auto& cmd : commands) {
        std::string commandWithSemicolon = cmd;
        if(!commandWithSemicolon.empty() && commandWithSemicolon.back() != ';') {
            commandWithSemicolon += ';';
        }
        
        debugLog("  Processing command: '" + commandWithSemicolon + "'");
        
        std::vector<std::string> tmpCommand = {commandWithSemicolon};
        std::string processedCommand = checkCommand(0, 0, &tmpCommand);
        
        if(!processedCommand.empty()) {
            result += "\t" + processedCommand;
            debugLog("  Result: '" + processedCommand + "'");
        }
    }
    
    return result;
}

std::string checkCommand(int line, int symbol, std::vector<std::string> *lines){
    if(symbol >= (*lines)[line].size()) return "";
    
    char firstChar = (*lines)[line][symbol];
    debugLog("checkCommand: line=" + std::to_string(line) + ", symbol=" + std::to_string(symbol) + ", char='" + std::string(1, firstChar) + "'");
    
    if(firstChar == '>') {
        return putVar(line, symbol, lines);
    }
    if(firstChar == '#') {
        return printVar(line, symbol, lines);
    }
    if(firstChar == '$') {
        return inputVar(line, symbol, lines);
    }
    if(firstChar == '?') {
        return conditionIfVar(line, symbol, lines);
    }
    if(firstChar == ':') {
        return conditionElseVar(line, symbol, lines);
    }
    if(firstChar == '<'){
        return outputVar(line, symbol, lines);
    }
    if(firstChar == '/'){
        return comment(line, symbol, lines);
    }
    if(firstChar == '@'){
        return whileRound(line, symbol, lines);
    }
    return "";
}

std::string putVar(int line, int symbol, std::vector<std::string> *lines){
    std::string outStr;
    std::string nameVar;
    bool isStr = false;
    bool isStrOpen = false;
    bool isNameFind = false;
    
    const std::string& current_line = (*lines)[line];
    debugLog("putVar: processing line='" + current_line + "'");
    
    try{
        for(int i = 1; symbol + i < current_line.size() && current_line[symbol + i] != ';'; i++){
            if(isNameFind) break;
            if(current_line[symbol + i] == '\"'){
                isStrOpen = !isStrOpen;
                isStr = true;
            }
            if(!isStrOpen && current_line[symbol + i] == '='){
                isNameFind = true;
                for(int j = i + 1; symbol + j < current_line.size() && current_line[symbol + j] != ';'; j++){
                    if(current_line[symbol + j] == '=') continue;
                    nameVar += current_line[symbol + j];
                }
                break;
            }
            if(current_line[symbol + i] == '=') continue;
            outStr += current_line[symbol + i];
        }
        
        std::string result;
        if(isStr){
            result = "std::string " + nameVar + " = " + outStr + ";\n";
        } else {
            result = "int " + nameVar + " = " + outStr + ";\n";
        }
        
        debugLog("putVar result: '" + result + "'");
        return result;
    } catch(const std::exception& e){
        debugLog("putVar ERROR: exception caught - " + std::string(e.what()));
        return "";
    }
}

std::string outputVar(int line, int symbol, std::vector<std::string> *lines){
    std::string outStr;
    std::string nameVar;
    bool isStr = false;
    bool isStrOpen = false;
    bool isNameFind = false;
    
    const std::string& current_line = (*lines)[line];
    debugLog("outputVar: processing line='" + current_line + "'");
    
    try{
        for(int i = 1; symbol + i < current_line.size() && current_line[symbol + i] != ';'; i++){
            if(isNameFind) break;
            if(current_line[symbol + i] == '\"'){
                isStrOpen = !isStrOpen;
                isStr = true;
            }
            if(!isStrOpen && current_line[symbol + i] == '='){
                isNameFind = true;
                for(int j = i + 1; symbol + j < current_line.size() && current_line[symbol + j] != ';'; j++){
                    if(current_line[symbol + j] == '=') continue;
                    nameVar += current_line[symbol + j];
                }
                break;
            }
            if(current_line[symbol + i] == '=') continue;
            outStr += current_line[symbol + i];
        }
        
        std::string result = nameVar + " = " + outStr + ";\n";
        debugLog("outputVar result: '" + result + "'");
        return result;
    } catch(const std::exception& e){
        debugLog("outputVar ERROR: exception caught - " + std::string(e.what()));
        return "";
    }
}

std::string printVar(int line, int symbol, std::vector<std::string> *lines){
    std::string outStr;
    const std::string& current_line = (*lines)[line];
    debugLog("printVar: processing line='" + current_line + "'");
    
    try{
        for(int i = 1; symbol + i < current_line.size() && current_line[symbol + i] != ';'; i++){
            outStr += current_line[symbol + i];
        }
        std::string result = "std::cout << " + outStr + ";\n";
        debugLog("printVar result: '" + result + "'");
        return result;
    } catch (const std::exception& e){
        debugLog("printVar ERROR: exception caught - " + std::string(e.what()));
        return "";
    }
}

std::string inputVar(int line, int symbol, std::vector<std::string> *lines){
    std::string outStr;
    const std::string& current_line = (*lines)[line];
    debugLog("inputVar: processing line='" + current_line + "'");
    
    try{
        for(int i = 1; symbol + i < current_line.size() && current_line[symbol + i] != ';'; i++){
            outStr += current_line[symbol + i];
        }
        std::string result = "std::cin >> " + outStr + ";\n";
        debugLog("inputVar result: '" + result + "'");
        return result;
    } catch (const std::exception& e){
        debugLog("inputVar ERROR: exception caught - " + std::string(e.what()));
        return "";
    }
}

std::string conditionIfVar(int line, int symbol, std::vector<std::string> *lines){
    std::string condition;
    
    const std::string& current_line = (*lines)[line];
    debugLog("conditionIfVar: processing line='" + current_line + "'");
    
    size_t open_paren = current_line.find('(', symbol);
    if (open_paren == std::string::npos) {
        debugLog("conditionIfVar ERROR: no opening parenthesis");
        return "";
    }
    
    size_t close_paren = current_line.find(')', open_paren);
    if (close_paren == std::string::npos) {
        debugLog("conditionIfVar ERROR: no closing parenthesis");
        return "";
    }
    
    condition = current_line.substr(open_paren + 1, close_paren - open_paren - 1);
    
    size_t open_bracket = current_line.find('[', close_paren);
    if (open_bracket == std::string::npos) {
        debugLog("conditionIfVar ERROR: no opening bracket");
        return "";
    }
    
    size_t close_bracket = findMatchingBracket(current_line, open_bracket);
    if (close_bracket == std::string::npos) {
        debugLog("conditionIfVar ERROR: no closing bracket");
        return "";
    }
    
    std::string bracketContent = current_line.substr(open_bracket + 1, close_bracket - open_bracket - 1);
    
    debugLog("conditionIfVar: bracketContent='" + bracketContent + "'");
    
    std::string result = "if(" + condition + ") {\n";
    result += parseBracketCommands(bracketContent);
    result += "}\n";
    
    debugLog("conditionIfVar result: '" + result + "'");
    return result;
}

std::string conditionElseVar(int line, int symbol, std::vector<std::string> *lines){
    const std::string& current_line = (*lines)[line];
    debugLog("conditionElseVar: processing line='" + current_line + "'");
    
    size_t open_bracket = current_line.find('[', symbol);
    if (open_bracket == std::string::npos) {
        debugLog("conditionElseVar ERROR: no opening bracket");
        return "";
    }
    
    size_t close_bracket = findMatchingBracket(current_line, open_bracket);
    if (close_bracket == std::string::npos) {
        debugLog("conditionElseVar ERROR: no closing bracket");
        return "";
    }
    
    std::string bracketContent = current_line.substr(open_bracket + 1, close_bracket - open_bracket - 1);
    
    debugLog("conditionElseVar: bracketContent='" + bracketContent + "'");
    
    std::string result = "else {\n";
    result += parseBracketCommands(bracketContent);
    result += "}\n";
    
    debugLog("conditionElseVar result: '" + result + "'");
    return result;
}

std::string comment(int line, int symbol, std::vector<std::string> *lines){
    std::string outStr;
    const std::string& current_line = (*lines)[line];
    debugLog("comment: processing line='" + current_line + "'");
    
    try{
        for(int i = 1; symbol + i < current_line.size() && current_line[symbol + i] != ';'; i++){
            outStr += current_line[symbol + i];
        }
        std::string result = "//" + outStr + "\n";
        debugLog("comment result: '" + result + "'");
        return result;
    } catch (const std::exception& e){
        debugLog("comment ERROR: exception caught - " + std::string(e.what()));
        return "";
    }
}

std::string whileRound(int line, int symbol, std::vector<std::string> *lines){
    std::string condition;
    
    const std::string& current_line = (*lines)[line];
    debugLog("whileRound: processing line='" + current_line + "'");
    
    size_t open_paren = current_line.find('(', symbol);
    if (open_paren == std::string::npos) {
        debugLog("whileRound ERROR: no opening parenthesis");
        return "";
    }
    
    size_t close_paren = current_line.find(')', open_paren);
    if (close_paren == std::string::npos) {
        debugLog("whileRound ERROR: no closing parenthesis");
        return "";
    }
    
    condition = current_line.substr(open_paren + 1, close_paren - open_paren - 1);
    
    size_t open_bracket = current_line.find('[', close_paren);
    if (open_bracket == std::string::npos) {
        debugLog("whileRound ERROR: no opening bracket");
        return "";
    }
    
    size_t close_bracket = findMatchingBracket(current_line, open_bracket);
    if (close_bracket == std::string::npos) {
        debugLog("whileRound ERROR: no closing bracket");
        return "";
    }
    
    std::string bracketContent = current_line.substr(open_bracket + 1, close_bracket - open_bracket - 1);
    
    debugLog("whileRound: bracketContent='" + bracketContent + "'");
    
    std::string result = "while(" + condition + ") {\n";
    result += parseBracketCommands(bracketContent);
    result += "}\n";
    
    debugLog("whileRound result: '" + result + "'");
    return result;
}

int main(int argc, char *argv[])
{
    std::setlocale(LC_ALL, "RU");
    
    if(argc < 2){
        std::cerr << "Usage: " << argv[0] << " <filename> [-deb]" << std::endl;
        return 1;
    }
    
    // Check for debug flag
    if(argc >= 3) {
        std::string arg2 = argv[2];
        if(arg2 == "-deb") {
            DEBUG_MODE = true;
            debugLog("Debug mode enabled");
        }
    }
    
    std::ifstream file(argv[1]);
    std::vector<std::string> lines;
    std::string line;
    
    if(!file.is_open()){
        std::cerr << "Error: Cannot open file " << argv[1] << std::endl;
        return 1;
    }
    
    while(std::getline(file, line)){
        lines.push_back(line);
    }
    
    std::cout << "Processing file..." << std::endl;
    
    for(int i = 0; i < lines.size(); i++){
        debugLog("Processing line " + std::to_string(i) + ": '" + lines[i] + "'");
        std::vector<std::string> commands = splitCommands(lines[i]);
        
        for(const auto& cmd : commands) {
            debugLog("  Processing command: '" + cmd + "'");
            std::vector<std::string> tmpLine = {cmd};
            
            if(!tmpLine[0].empty()) {
                std::string result = checkCommand(0, 0, &tmpLine);
                if(!result.empty()) {
                    cfile.push_back(result);
                    debugLog("  Added to cfile: '" + result + "'");
                }
            }
        }
    }
    
    cfile.push_back("return 0;\n");
    cfile.push_back("}\n");
    
    std::system("touch a.cpp");
    std::system("echo \"\" > a.cpp");
    
    for(int i = 0; i < cfile.size(); i++){
        std::system(("echo '" + cfile[i] + "' >> a.cpp").c_str());
    }
    
    std::cout << "Compiling..." << std::endl;
    int compileResult = std::system("g++ a.cpp -o output.out");
    
    if(compileResult == 0) {
        std::cout << "Compilation successful! Run: ./output.out" << std::endl;
    } else {
        std::cout << "Compilation failed!" << std::endl;
    }
    
    return 0;
}