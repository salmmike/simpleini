/*
Copyright (c) 2023 Mike Salmela

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef _SIMPLEINI_H
#define _SIMPLEINI_H

#include <exception>
#include <filesystem>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace simpleini {

class INIException : public std::runtime_error
{
  public:
    explicit INIException(const std::string& msg)
      : std::runtime_error(msg){};
    using std::runtime_error::runtime_error;
};

static bool
string_is_valid(const std::string& str)
{
    if (str.empty() || str.starts_with(';') || str.starts_with('#') ||
        str.find_first_not_of(' ') == str.npos) {
        return false;
    }
    return true;
}

static std::string
parse_section_value(const std::string& str)
{
    std::size_t start = 1;
    std::size_t end = str.find(']');
    return str.substr(start, end - 1);
}

/// TODO: strip tabs
static std::string
strip_trailing(const std::string& str)
{
    std::size_t last_char = str.find_last_not_of(' ');
    return str.substr(0, last_char + 1);
}

/// TODO: strip tabs
static std::string
strip_leading(const std::string& str)
{
    std::size_t first_char = str.find_first_not_of(' ');
    return str.substr(first_char, str.length());
}

static std::string
strip(const std::string& str)
{
    return strip_leading(strip_trailing(str));
}

static std::pair<std::string, std::string>
parse_key_value(const std::string& str)
{
    std::size_t equalpos = str.find('=');
    std::string key = str.substr(0, equalpos);
    std::string value = str.substr(equalpos + 1, str.length());

    return { strip(key), strip(value) };
}

class INISection
{
  public:
    INISection(){};

    explicit INISection(const std::string& name,
                        const std::map<std::string, std::string>& content)
      : m_name(name)
      , m_contents(content){};

    ~INISection(){};

    /// @brief Returns true if the INISection is empty
    /// @return boolean
    [[nodiscard]] bool empty() { return m_contents.empty(); };

    /// @brief Return value of key @key
    /// @param key the key for the value
    /// @return string value of key
    /// @throws std::out_of_range if key doesn't exist
    std::string operator[](const std::string& key) const { return get(key); };

    /// @brief Return value of key @key
    /// @param key the key for the value
    /// @return string value of key
    /// @throws std::out_of_range if key doesn't exist
    std::string get(const std::string& key) const
    {
        if (!m_contents.contains(key)) {
            throw std::out_of_range("No key '" + key + "' in section '" +
                                    m_name + "'");
        }
        return m_contents.at(key);
    };

    /// @brief Get as type T
    /// @throws INIException if conversion to type T fails.
    /// @throws std::out_of_range if key doesn't exist.
    template<typename T>
    T get_as(const std::string& key)
    {
        auto val = get(key);
        std::stringstream ss(val);
        T retval;
        ss >> retval;
        if (ss.fail()) {
            throw INIException("Conversion failed from value '" + val + "'");
        }
        return retval;
    }

    /// @brief Get the stored values as std::map
    /// @return the key value map for this ini configuration section
    std::map<std::string, std::string> get_map() { return m_contents; };

  private:
    std::string m_name;
    std::map<std::string, std::string> m_contents;
};

class SimpleINI
{
  public:
    SimpleINI(){};

    /// @brief Create a SimpleINI object
    /// @param configfilepath path to the used configuration .ini file.
    /// @throws INIException if the file at @configfilepath isn't in valid .ini
    /// format.
    explicit SimpleINI(std::filesystem::path configfilepath)
      : m_path(configfilepath)
    {
        read_content();
        parse_sections();
    };

    ~SimpleINI(){};

    /// @brief Read a configuration file.
    /// @param path Path to the configuration file.
    void set_config_file(std::filesystem::path path)
    {
        m_path = path;
        read_content();
        parse_sections();
    }

    /// @brief Get the configuration file path.
    /// @return std::filesystem::path to the file.
    std::filesystem::path get_config_path() { return m_path; };

    /// @brief Return section with key @key
    /// @param key the key for the section
    /// @return INISection for key
    /// @throws std::out_of_range if section doesn't exist
    INISection operator[](const std::string& key) const
    {
        if (!m_sections.contains(key)) {
            throw std::out_of_range("No section '" + key + "'");
        }
        return m_sections.at(key);
    };

    /// @brief Get the section name - INISection map
    /// @return the stored std::map
    std::map<std::string, INISection> get_map() const { return m_sections; };

  private:
    std::filesystem::path m_path;
    std::vector<std::string> m_content;
    std::map<std::string, INISection> m_sections;

    void read_content()
    {
        if (!std::filesystem::exists(m_path)) {
            throw INIException("File not found:" + m_path.string());
        }

        m_content.clear();
        std::ifstream configstream(m_path);

        std::string line;
        while (std::getline(configstream, line)) {
            if (string_is_valid(line)) {
                m_content.push_back(line);
            }
        }
    };

    void parse_sections()
    {
        m_sections.clear();
        std::map<std::string, std::string> config_items;
        std::string current_section;

        for (const auto& line : m_content) {
            if (line.starts_with('[')) {
                if (!current_section.empty()) {
                    m_sections.insert(
                      { current_section,
                        INISection{ current_section, config_items } });
                    config_items.clear();
                }
                current_section = parse_section_value(line);
            } else if (line.find('=') != line.npos) {
                auto configs = parse_key_value(line);
                config_items.insert(configs);
            } else {
                throw INIException{ "Failure when parsing line " + line };
            }
        }
        if (!current_section.empty()) {
            m_sections.insert(
              { current_section, INISection{ current_section, config_items } });
            config_items.clear();
        }
    };
};
}

#endif
