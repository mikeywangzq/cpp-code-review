#pragma once

#include "report/reporter.h"
#include <fstream>
#include <string>

namespace cpp_review {

class HTMLReporter {
public:
    static void generateHTMLReport(const Reporter& reporter,
                                   const std::string& output_file);

private:
    static std::string getHTMLHeader();
    static std::string getHTMLFooter();
    static std::string severityToHTML(Severity severity);
    static std::string severityToColor(Severity severity);
    static std::string escapeHTML(const std::string& input);
};

} // namespace cpp_review
