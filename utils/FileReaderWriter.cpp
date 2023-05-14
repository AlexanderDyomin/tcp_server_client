#include "FileReaderWriter.h"

#include <fstream>
#include <iostream>

namespace {
    constexpr auto RW_OWNER = std::filesystem::perms::owner_read | std::filesystem::perms::owner_write;
    bool isRWOwner(const std::filesystem::path& path) {
        return std::filesystem::status(path).permissions() != RW_OWNER;
    }
}

namespace Utils {
    bool FileReaderWriter::init(std::string filename) noexcept {
        std::error_code ec;
        auto backupFilename = filename + ".backup";
        m_file = std::filesystem::absolute({std::move(filename)}, ec);
        if (ec) {
            std::cerr << "Failed to transform path " << m_file << " to absolute path" << std::endl;
            return false;
        }
        m_backupFile = {std::move(backupFilename)};

        if (!std::filesystem::is_regular_file(m_file)) {
            std::cerr << "File " << m_file << " is not a regular file" << std::endl;
            return false;
        }


        if (isRWOwner(m_file)) {
            std::cerr << "File " << m_file << " must be readable and writable only by owner" << std::endl;
            return false;
        }

        auto dir = m_file.parent_path();
        if (!std::filesystem::is_directory(dir)) {
            std::cerr << "Parent path of " << m_file << " is not a directory" << std::endl;
            return false;
        }

        if (!isRWOwner(dir)) {
            std::cerr << "Dir " << dir << " must be readable and writable only by owner" << std::endl;
            return false;
        }

        return true;
    }

    bool FileReaderWriter::read(const std::function<bool(std::ifstream &)> &readContentFunc) noexcept {
        std::ifstream input(m_file);
        if (input.is_open()) {
            return readContentFunc(input) && !input.bad();
        } else {
            std::cerr << "Failed open " << m_file << " on read" << std::endl;
        }

        std::ifstream inputBackup(m_backupFile);
        if (inputBackup.is_open()) {
            std::cout << "Found backup file, will use it" << std::endl;
            bool res = readContentFunc(inputBackup);
            inputBackup.close();
            if (!res || inputBackup.fail()) {
                return false;
            }

            std::error_code ec;
            std::filesystem::rename(m_backupFile, m_file, ec);
            if (ec) {
                std::cerr << "Error on renaming file " << m_backupFile << " to " << m_file << std::endl;
            }

            return true;
        }

        return false;
    }

    bool FileReaderWriter::write(const std::function<void(std::ofstream &)> &writeContentFunc) noexcept {
        std::ofstream out(m_backupFile);
        writeContentFunc(out);
        out.close();
        if (out.fail()) {
            std::cerr << "Error on writing file " << m_backupFile  << std::endl;
            return false;
        }

        std::error_code ec;
        std::filesystem::permissions(m_backupFile, RW_OWNER, ec);
        if (ec) {
            std::cerr << "Error on setting permissions on file " << m_backupFile << std::endl;
            return false;
        }

        std::filesystem::remove(m_file, ec);
        if (ec) {
            std::cerr << "Error removing file " << m_file << std::endl;
            return false;
        }

        std::filesystem::rename(m_backupFile, m_file, ec);
        if (ec) {
            std::cerr << "Error on renaming file " << m_backupFile << " to " << m_file << std::endl;
            return false;
        }

        return true;
    }
}