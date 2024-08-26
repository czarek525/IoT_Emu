#include "../Headers/helper_functions.hpp"
#include "../Objects/ComponentFactory/ComponentFactory.hpp"
#include "../Objects/Comp_log/comp_log.hpp"
using namespace std;

/**
 * @brief Splits the input string based on matching braces positions and normalised RCR.
 *
 * This function takes an input string representing the description of a single component
 * and converts it into a vector of strings for each of the inner braces.
 *
 * Example:
 * {a,{b,c,{d}}} => {a,{b,c,{d}}}, {b,c,{d}}, {d}
 *
 * Then it normalised braces for every repeat section(correlate with index in vector).
 * Before is need to delete duplicates.
 * 
 * Example:
 * {a,{b,c,{d}}}, {b,c,{d}}, {d} => 0:{a,1!},1: {b,c,!2},2: {d}
 * 
 * Last step is to delete start('{') and end('}') brace, for every index in vector.
 * 0:{a,1!},1: {b,c,!2},2: {d} -> 0:a,1!,1: b,c,!2,2: d
 * 
 * @param input The input string from which substrings will be extracted based on matching braces.
 * @return A normalised vector of substrings extracted from the input string based on matching braces.
 */
std::vector<std::string> processRCR(const std::string &input)
{
    std::vector<size_t> openBracePositions;
    std::vector<size_t> closeBracePositions;

    for (size_t i = 0; i < input.length(); ++i)
    {
        if (input[i] == '{')
            openBracePositions.push_back(i);
        else if (input[i] == '}')
            closeBracePositions.push_back(i);
    }
    std::vector<size_t> openBracesWithoutMatch = {};
    std::map<size_t, size_t> matchingBracePairs;
    // Correlate Brace (start brace with end brace) in numeric form.
    // If openPos(openbracket position) is higher than closePos(close bracket position) previous openPos bracket is correlated with this closePos.
    for (auto closePos : closeBracePositions)
    {
        for (auto openPos : openBracePositions)
        {
            if (openPos < closePos)
            {
                if (find(openBracesWithoutMatch.begin(), openBracesWithoutMatch.end(), openPos) == openBracesWithoutMatch.end() && matchingBracePairs.find(openPos) == matchingBracePairs.end())
                    // Check openPos isn't in openBracesWithoutMatch vector and check that openPos isnt't in matchingBracePairs
                    openBracesWithoutMatch.push_back(openPos);
            }
            else
            {
                matchingBracePairs[openBracesWithoutMatch.back()] = closePos;
                openBracesWithoutMatch.pop_back();
                break;
            }
        }
    }
    // Complement first open bracket with last open brackets
    for (size_t i = 0; i < openBracesWithoutMatch.size(); i++)
        matchingBracePairs[openBracesWithoutMatch.at(i)] = closeBracePositions.at(closeBracePositions.size() - 1 - i);
        
    std::vector<std::string> result;

    // Transformation to String
    for (const auto &pair : matchingBracePairs)
    {
        std::string value = input.substr(pair.first, pair.second - pair.first + 1);
        result.push_back(value);
    }
    //Delete duplications(last one stays)
    for (int i = result.size() - 1; i >= 0; --i) 
        for (int j = i - 1; j >= 0; --j)
            if (result[i] == result[j])
                 result.erase(result.begin() + j);
    
    //Normalised vector
    for (size_t i = 0; i < result.size(); i++)
    {
        for (size_t j = i + 1; j < result.size(); j++)
        {
            size_t pos;
            while ((pos = result[i].find(result[j])) != std::string::npos) {
                std::string replacement = "!" + std::to_string(j);
                result[i].replace(pos, result[j].length(), replacement);
            }
        }
    }
    //Delete open and close braces
    for(auto &line : result)
        Helper_functions::deleteBrace(line);
    return result;
}

#ifdef _WIN32
std::vector<std::string> readRCRFileContents(std::string folderPath) {
    std::vector<std::string> fileContents;
    std::string searchPath = folderPath + "\\*.rcr";
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile(searchPath.c_str(), &findFileData);

    if (hFind == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to open directory: " << folderPath << std::endl;
        return fileContents;
    } 

    do {
        std::string fileName = findFileData.cFileName;
        std::string filePath = folderPath + "\\" + fileName;
        std::ifstream file(filePath);

        if (file) {
            std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            fileContents.push_back(content);
            file.close();
        } else {
            std::cerr << "Failed to open file: " << filePath << std::endl;
        }
    } while (FindNextFile(hFind, &findFileData) != 0);

    FindClose(hFind);
    return fileContents;
}
#else
std::vector<std::string> readRCRFileContents(std::string folderPath) {
    std::vector<std::string> fileContents;
    DIR *dir;
    struct dirent *entry;
    if ((dir = opendir(folderPath.c_str())) == nullptr) {
        std::cerr << "Incorrect rcr directory: " << folderPath << std::endl;
        return fileContents;
    }

    while ((entry = readdir(dir)) != nullptr) {
        std::string fileName = entry->d_name;

        if (fileName.size() > 4 && fileName.substr(fileName.size() - 4) == ".rcr") {
            std::string filePath = folderPath + "/" + fileName;
            std::ifstream file(filePath);

            if (file) {
                std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
                fileContents.push_back(content);
                file.close();
            } else {
                std::cerr << "Failed to open file: " << filePath << std::endl;
            }
        }
    }
    closedir(dir);
    return fileContents;
}
#endif
bool isTimeValidAndGreater(const std::string& time) {
    std::regex time_regex(R"(^([01][0-9]|2[0-3]):([0-5][0-9]):([0-5][0-9])$)");
    std::smatch match;
    if (!std::regex_match(time, match, time_regex)) {
        return false;  
    }
    int h = std::stoi(match[1].str());
    int m = std::stoi(match[2].str());
    int s = std::stoi(match[3].str());

    auto now = std::chrono::system_clock::now();
    auto now_time = std::chrono::system_clock::to_time_t(now);
    auto local_time = std::localtime(&now_time);

    return (h > local_time->tm_hour) ||
           (h == local_time->tm_hour && m > local_time->tm_min) ||
           (h == local_time->tm_hour && m == local_time->tm_min && s > local_time->tm_sec);
}


bool startEmulation(long seconds){
    auto future_time = std::chrono::system_clock::now() + std::chrono::seconds(seconds);
    std::time_t future_time_t = std::chrono::system_clock::to_time_t(future_time);
    std::tm* future_tm = std::localtime(&future_time_t);
    std::ostringstream oss;
    oss << std::put_time(future_tm, "%H:%M:%S");
    std::string time = oss.str();
    for (auto &comp : Component::cnames_components)
    {
            auto compPtr = comp.second;
            compPtr->subscribeEvents();
        {
            std::lock_guard<std::mutex> lock(compPtr->fut_mtx);
            compPtr->futures.push_back(std::async(std::launch::async, &Component::startFlow, compPtr,time));
        }
    }
    return true;
}
bool startEmulation(std::string time){
    if (isTimeValidAndGreater(time)){
        for (auto &comp : Component::cnames_components)
        {
                auto compPtr = comp.second;
                compPtr->subscribeEvents();
            {
                std::lock_guard<std::mutex> lock(compPtr->fut_mtx);
                compPtr->futures.push_back(std::async(std::launch::async, &Component::startFlow, compPtr,time));
            }
        } 
        return true; 
    }else{
        std::cerr<<"Incorrect time format it should be greater than actual time, and in valid form"<<std::endl;
        return false;
    }
}
void createComponent(const std::vector<std::string>&  normalised_rcr){
    //Extract component information for logs
    std::string c_name = Helper_functions::getTokenAtIndex(normalised_rcr.at(0),0);
    std::string pid = Helper_functions::getTokenAtIndex(normalised_rcr.at(0),1);
    //Create elements of component
    for(auto &element: normalised_rcr){
        if (element.substr(0,2) == "E;")
           ComponentFactory::eventsCreator(Helper_functions::getTokenAtIndex(element,1), normalised_rcr,c_name,pid);
        if (element.substr(0,2) == "S;")
           ComponentFactory::fsmsCreator(Helper_functions::getTokenAtIndex(element,1), normalised_rcr,c_name,pid);
        if (element.substr(0,2) == "F;")
           ComponentFactory::flowsCreator(Helper_functions::getTokenAtIndex(element,1), normalised_rcr,c_name,pid);
        if (element.substr(0,2) == "P;")
            ComponentFactory::portsCreator(Helper_functions::getTokenAtIndex(element,1), normalised_rcr,c_name,pid);
        if (element.substr(0,2) == "M;")
            ComponentFactory::MQTTbrokerCreator(Helper_functions::getTokenAtIndex(element,1), normalised_rcr,c_name,pid);
    }
    //Create component(elements will be take from ComponentFactory)
    Component::cnames_components[c_name] = ComponentFactory::componentCreator(c_name,pid);
}
int main(int argc, char const *argv[])
{
    //string input = "{c_name;1234;{E;[{e_name1;io;MQTT_e_name1},{e_name2;l;100s}]};{S;[{m_name1;[{s_name1;[e_name1,e_name2];[e_name1,e_name2];[{e_name1;s_name1;[e_name1,e_name2]},{e_name2;s_name1;[e_name1,e_name2]}]},{s_name2;[e_name1,e_name2];[e_name1,e_name2];[{e_name1;s_name2;[e_name1,e_name2]},{e_name2;s_name2;[e_name1,e_name2]}]}];s_name2},{m_name2;[{s_name3;[e_name1,e_name2];[e_name1,e_name2];[{e_name1;s_name3;[e_name1,e_name2]},{e_name2;s_name3;[e_name1,e_name2]}]},{s_name4;[e_name1,e_name2];[e_name1,e_name2];[{e_name1;s_name4;[e_name1,e_name2]},{e_name2;s_name4;[e_name1,e_name2]}]}];s_name3}]};{F;[{f_name1;{simple;1;1ms}},{f_name2;{simple;1;1ms}}]};{P;[{p_name1;s;T;{100.100.100.100;2};{333.333.333.333;4444};m_name1;[{s_name1;f_name1},{s_name2;{on_off;1;3s}}]},{p_name2;c;U;{123.100.100.100;2};{133.333.333.333;4444};m_name2;[{s_name3;f_name2},{s_name4;{on_off;1;3ms}}]}]};{M;{127.0.0.1;1883}}}";
    //string input = "{c_name;1234;{E;[{e_name1;e;MQTT_e_name1},{e_name2;l;100ms},{e_name3;o;MQTT_e_name3},{e_name4;io;MQTT_e_name4}]};{S;[{m_name1;[{s_name1;;[e_name2];[{e_name1;s_name2;}]},{s_name2;[e_name3];;[{e_name2;s_name1;[e_name4]}]}];s_name1},{m_name2;[{s_name1;;;[{e_name4;s_name2;}]},{s_name2;;;}];s_name1}]};{F;[{f_name1;{on_off;1024;2s;20s;10s}}]};{P;[{p_name1;s;U;{127.0.0.1;8888};;;},{p_name3;s;T;{127.0.0.1;8884};;;},{p_name2;c;T;;{127.0.0.1;8884};m_name1;[{s_name1;f_name1},{s_name2;{simple;1024;1ms}}]},{p_name4;c;T;;{127.0.0.1;8884};m_name2;[{s_name2;{simple;1024;20s}},{s_name1;f_name1}]}]};{M;{127.0.0.1;1883}}}";
    //string input2 = "{c_name3;12341;{E;[{e_name1;e;MQTT_e_name1},{e_name2;l;100ms},{e_name3;o;MQTT_e_name3},{e_name4;io;MQTT_e_name4}]};{S;[{m_name1;[{s_name1;;[e_name2];[{e_name1;s_name2;}]},{s_name2;[e_name3];;[{e_name2;s_name1;[e_name4]}]}];s_name1},{m_name2;[{s_name1;;;[{e_name4;s_name2;}]},{s_name2;;;}];s_name1}]};{F;[{f_name1;{simple;1024;5s}}]};{P;[{p_name1;s;U;{127.0.0.1;8883};;;},{p_name3;s;T;{127.0.0.1;8882};;;},{p_name2;c;U;;{127.0.0.1;8883};m_name1;[{s_name1;f_name1},{s_name2;{simple;1024;1ms}}]},{p_name4;c;U;;{127.0.0.1;8883};m_name2;[{s_name2;{simple;1024;20s}},{s_name1;f_name1}]}]};{M;{127.0.0.1;1883}}}";

    if (argc < 2) {
        std::cout << "WRITE TIME IN SECONDS OR TIME TO START EMULATION IN FORMAT HH:MM:SS AND PATH TO RCR FILE" << std::endl;
        return -1;
    }    
    std::string path = argv[2];

    std::vector<std::string> rcrs = readRCRFileContents(path);
    if (rcrs.size()==0){
        std::cout<<"NO RCR FILES IN FOLDER"<<std::endl;
        return -1;
    }else{
        vector<string> normalisedRCR;
        for (auto& rcr: rcrs)
        {
            normalisedRCR = processRCR(rcr);
            createComponent(normalisedRCR);
        }
    }

    std::string arg = argv[1];
    bool isemulationStart;
    if (arg.find_first_not_of("0123456789") == std::string::npos) 
        isemulationStart=startEmulation(std::strtoul(arg.c_str(), nullptr, 10));
    else 
        isemulationStart=startEmulation(arg);
    while (isemulationStart)
    {
        if (std::cin.get() == 'q')
        {
            Component::terminateFlag.store(true);
            Component::comp_cv.notify_all();

            for (auto &comp : Component::cnames_components)
            {
                for (auto &fsm :comp.second->mnames_fsms)
                    fsm.second->cv.notify_all();
                if (comp.second->client_.is_connected())
                    try
                    {
                        comp.second->client_.disconnect()->wait();
                    }
                    catch(const std::exception& exc)
                    {
                        std::cerr << "Disconnect error: " << exc.what() << std::endl;
                    }
                    std::lock_guard<std::mutex> lock(comp.second->fut_mtx);
                    for (auto &thread : comp.second->futures){
                        thread.get();
                    }
            }
            for (auto &log_be : Comp_log::full_logs)
            {
                if (log_be->ph_value==BE && log_be->end.empty())
                    log_be->end = Comp_log::getFormattedTime(std::chrono::system_clock::now());

            }
            std::ofstream outputFile("output.txt");

            if (!outputFile.is_open())
                std::cerr << "Unable to open file!" << std::endl;

            for (auto &a : Comp_log::full_logs)
            {
                outputFile << "name: " << a->name << std::endl;
                outputFile << "ts: " << a->ts << std::endl;
                outputFile << "pid: " << a->pid << std::endl;

                outputFile << "cat: ";
                for (const auto &catValue : a->cat)
                    outputFile << catValue << " ";
                outputFile << std::endl;

                outputFile << "args: ";
                for (const auto &argValue : a->args)
                    outputFile << argValue << " ";
                outputFile << std::endl;

                outputFile << "end: " << a->end << std::endl;
                outputFile << std::endl;
            }

            outputFile.close();
            break;
        }
    };

    return 0;
}