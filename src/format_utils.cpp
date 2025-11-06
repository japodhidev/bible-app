#include "../include/format_utils.hpp"

#include <sstream>
#include <algorithm>
#include <cctype>

#if defined(__unix__) || defined(__APPLE__)
#include <sys/ioctl.h>
#include <unistd.h>
#include <cstdlib>
#endif

#if defined(_WIN32)
#include <windows.h>
#endif

// Constants
constexpr int DEFAULT_TERMINAL_WIDTH = 80;
constexpr double MARGIN_PERCENTAGE = 0.10;  // 10%
constexpr int MIN_MARGIN = 4;
constexpr int MAX_MARGIN = 15;
constexpr int FIRST_LINE_INDENT = 4;

int getTerminalWidth() {
#if defined(__unix__) || defined(__APPLE__)
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0 && w.ws_col > 0) {
        return w.ws_col;
    }
    
    // Try environment variable
    const char* columns = std::getenv("COLUMNS");
    if (columns) {
        try {
            int width = std::stoi(columns);
            if (width > 0) return width;
        } catch (...) {
            // Ignore conversion errors
        }
    }
#endif

#if defined(_WIN32)
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        return csbi.dwSize.X;
    }
#endif

    return DEFAULT_TERMINAL_WIDTH;
}

std::vector<std::string> wrapText(const std::string& text, int maxWidth) {
    if (maxWidth <= 0) {
        return {text};
    }

    std::vector<std::string> lines;
    std::istringstream iss(text);
    std::string word;
    std::string currentLine;

    while (iss >> word) {
        if (currentLine.empty()) {
            currentLine = word;
        } else if (static_cast<int>(currentLine.length()) + 1 + static_cast<int>(word.length()) <= maxWidth) {
            currentLine += " " + word;
        } else {
            lines.push_back(currentLine);
            currentLine = word;
        }

        // Handle words longer than maxWidth
        while (static_cast<int>(currentLine.length()) > maxWidth) {
            lines.push_back(currentLine.substr(0, maxWidth));
            currentLine = currentLine.substr(maxWidth);
        }
    }

    if (!currentLine.empty()) {
        lines.push_back(currentLine);
    }

    return lines;
}

std::string formatVerseProse(const std::string& verseText, int verseNum, 
                             int terminalWidth, int leftMargin, int firstLineIndent) {
    int effectiveWidth = terminalWidth - leftMargin * 2;

    // Calculate verse number prefix (e.g., "1. ", "42. ")
    std::string verseNumStr = std::to_string(verseNum);
    std::string versePrefix = verseNumStr + ". ";
    int versePrefixLen = static_cast<int>(versePrefix.length());

    // First line width accounts for indent and verse number prefix
    int firstLineWidth = effectiveWidth - firstLineIndent - versePrefixLen;
    int subsequentLineWidth = effectiveWidth;

    // Wrap entire text to first line width
    auto allLines = wrapText(verseText, firstLineWidth);

    std::string result;

    if (!allLines.empty()) {
        // First line: indent + verse number + text
        result += std::string(leftMargin + firstLineIndent, ' ') 
                + versePrefix 
                + allLines[0] + "\n";

        // Re-wrap remaining text for subsequent lines (if any)
        if (allLines.size() > 1) {
            // Reconstruct remaining text
            std::string remainingText;
            for (size_t i = 1; i < allLines.size(); ++i) {
                if (i > 1) remainingText += " ";
                remainingText += allLines[i];
            }

            // Wrap remaining text for subsequent lines
            auto subsequentLines = wrapText(remainingText, subsequentLineWidth);
            for (const auto& line : subsequentLines) {
                result += std::string(leftMargin, ' ') + line + "\n";
            }
        }
    }

    return result;
}

