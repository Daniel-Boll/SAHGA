#include <sahga/utils/utils.hpp>
#include <sstream>

namespace Utils {
  namespace miscellaneous {
    /*
     * Split string with delimiter and return vector of strings
     *
     * @param { std::string } str - string to split
     * @param { char } delimiter - delimiter to split string
     *
     * @return { std::vector<std::string> } - vector of strings
     *
     * # Example
     *
     * `std::cout << Utils::miscellaneous::split("Hello, World!", ',') <<
     * std::endl;`
     * > ["Hello", " World!"]
     */
    std::vector<std::string> split(std::string str, char delimiter) {
      std::vector<std::string> result;
      int8_t pos = 0;
      std::string token;
      while ((pos = str.find(delimiter)) != std::string::npos) {
        token = str.substr(0, pos);
        result.push_back(token);
        str.erase(0, pos + 1);
      }
      result.push_back(str);
      return result;
    }

    double distance(double x1, double y1, double x2, double y2) {
      double distgraus = sqrt(pow(fabs(x2 - x1), 2) + pow(fabs(y2 - y1), 2));
      return (distgraus * Utils::constants::PI * Utils::constants::RADIUS / 180);
    }

    /*
     * Converts a string to full upper case.
     *
     * @param { std::string } **str** - The origin string.
     *
     * @return { std::string } - The string in upper case.
     *
     * # Example
     *
     * `std::cout << Utils::miscellaneous::upperCase("Non upper case phrase") <<
     * std::endl;`
     * > "NON UPPER CASE PHRASE"
     * */
    std::string upperCase(std::string str) {
      std::transform(str.begin(), str.end(), str.begin(), ::toupper);
      return str;
    }
  }  // namespace miscellaneous

  namespace filemanagement {
    std::string currentDirectory() { return std::filesystem::current_path().string(); }

    std::string getRootDirectory(const std::string &projectName) {
      std::string cd = Utils::filemanagement::currentDirectory();
      // Use the current directory only up after the project name.
      std::string rootDirectory = cd.substr(0, cd.find(projectName) + projectName.length());
      return rootDirectory;
    }

    double getNumber(std::string &line, char separator) {
      int32_t i = 0;
      while ((line[i] != separator) && (line[i] != '\0')) ++i;
      std::string buffer = line.substr(0, i);
      line.erase(0, i + 1);

      return std::atof(buffer.c_str());
    }

    std::string splitOnce(std::string &line, char separator) {
      int32_t i = 0;
      while ((line[i] != separator) && (line[i] != '\0')) ++i;
      std::string buffer = line.substr(0, i);
      line.erase(0, i + 1);
      return (Utils::miscellaneous::upperCase(buffer));
    }
  }  // namespace filemanagement
}  // namespace Utils
