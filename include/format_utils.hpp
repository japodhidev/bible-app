#pragma once

#include <string>
#include <vector>

/**
 * Gets the terminal width in characters
 * @return Terminal width, or default 80 if detection fails
 */
int getTerminalWidth();

/**
 * Wraps text to fit within a maximum width, breaking at word boundaries
 * @param text The text to wrap
 * @param maxWidth Maximum width for each line
 * @return Vector of wrapped lines
 */
std::vector<std::string> wrapText(const std::string& text, int maxWidth);

/**
 * Formats a verse in prose style with margins, indentation, and verse number
 * @param verseText The verse content
 * @param verseNum The verse number
 * @param terminalWidth Total terminal width
 * @param leftMargin Left margin in characters
 * @param firstLineIndent Additional indent for first line
 * @return Formatted verse string
 */
std::string formatVerseProse(const std::string& verseText, int verseNum, 
                             int terminalWidth, int leftMargin, int firstLineIndent);

