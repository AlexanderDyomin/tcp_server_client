#pragma once

#include <string>
#include <functional>
#include <filesystem>

namespace Utils {
/*
 *  File and containing it directory must be owned by user who run executable
 *  (having rights 0600)
 */
    class FileReaderWriter {
    public:
        bool init(std::string filename) noexcept;

/*
 *  Trying to read file as follow:
 *  1. Read from file m_file
 *  2. If no such file, try read from m_backupFile
 *  Returns true if success read, false if readContentFunc returned false or encountered error on read.
 */
        bool read(const std::function<bool(std::ifstream &)> &readContentFunc) noexcept;

/*
 *  Trying to write file as follow:
 *  1. Write data to m_backupFile
 *  2. Set m_backupFile permissions to 0600
 *  3. Delete m_file
 *  4. Rename m_backupFile to m_file
 *  Returns true if success read, false if readContentFunc returned false or encountered error on read.
 *
 *  This way we always will have previous version available if there will be problems on writing
 *  actual version of data.
 */
        bool write(const std::function<void(std::ofstream &)> &writeContentFunc) noexcept;

    private:
        std::filesystem::path m_file;
        std::filesystem::path m_backupFile;
    };
}