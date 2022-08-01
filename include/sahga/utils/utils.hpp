#pragma once

#include <sahga/core/core.hpp>

namespace Utils {
  namespace constants {
    static inline const double PI = 3.1415926535897932;
    static inline const double RADIUS = 6378.160;  // TODO: check with Adair
  }                                                // namespace constants

  namespace miscellaneous {
    std::vector<std::string> split(std::string str, char delimiter);

    // Calculate the distance between x and y coordinates.
    double distance(double x1, double y1, double x2, double y2);

    // Converts a full string to upper case.
    std::string upperCase(std::string str);
  }  // namespace miscellaneous

  namespace filemanagement {
    std::string currentDirectory();
    std::string getRootDirectory(const std::string &projectName);

    // Converts, removes and then returns the first number occurrence in the
    // string line.
    double getNumber(std::string &line, char separator);

    std::string splitOnce(std::string &line, char separator);
  }  // namespace filemanagement
}  // namespace Utils
