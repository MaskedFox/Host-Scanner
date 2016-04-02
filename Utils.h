#pragma once
#include "Stdafx.h"
#include <string>
#include <tuple>
#include <functional>
#include <boost/optional.hpp>

#if HAVE_CURL
	#include <curl/curl.h>
#else
	typedef void CURL;
#endif

/*!
 * Executes a command and returns its output.
 *
 * \param cmd Command to execute.
 */
std::string execute(const char* cmd);

/*!
 * Gets the path to the executable.
 *
 * \return Path to executable.
 */
std::string getAppPath();

/*!
 * Gets the path to the current working directory.
 *
 * \return Path to working directory.
 */
std::string getWorkDir();

/*!
 * Gets the value of the requested environment variable.
 *
 * \param env Variable name.
 *
 * \return Environment variable, or empty string.
 */
std::string getEnvVar(const std::string& env);

/*!
 * Splits the path, second item will be top-level file or
 * directory, while the first will be the rest of the path.
 *
 * \param path Path to split.
 *
 * \return Rest of the path, top-level file or directory.
 */
std::tuple<std::string, std::string> splitPath(const std::string& path);

/*!
 * Pluralizes the specified unit based on the quantity.
 *
 * \param quantity Quantity of unit.
 * \param unit Unit to pluralize.
 * \param addIs Append `is` or `are`.
 *
 * \return String with quantity and unit, pluralized if needed.
 */
std::string pluralize(int quantity, const std::string& unit, bool addIs = false);

/*!
 * Fetches the content behind the specified URL.
 *
 * \param url Location to download.
 * \param opts Optional callback function called right after setting up curl,
 *             and before performing the request. Within this function, you may
 *             manipulate any aspect of the request by calling `curl_easy_setopt`
 *             on the passed `CURL` pointer.
 *
 * \return Tuple containing two strings and the response code:
 *         the downloaded string, if any, and the error message, if any.
 */
std::tuple<std::string, std::string, int> getURL(const std::string& url, const std::function<void(CURL*)>& opts = nullptr);

/*!
 * Retrieves the error message for the last I/O error.
 * 
 * On Windows, it uses `WSAGetLastError()` with `FormatMessage()`.
 * On Linux, it uses `errno` with `strerror()`.
 *
 * \param code Optional error code. If not specified, will retrieve
 * 			   the last code from the operating system.
 * 
 * \return The net error string.
 */
std::string getNetErrStr(boost::optional<int> code = boost::none);
