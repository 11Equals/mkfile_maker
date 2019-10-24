
#include <cassert>
#include <filesystem>
#include <iostream>
#include <fstream>

bool createDirectory(std::string const & path) {
	if (!std::filesystem::exists(path)) {
		auto created = std::filesystem::create_directory(path);
		if (created) {
			std::cout << "Successfully created directory: " << path << std::endl;
			assert(std::filesystem::exists(path));
			return true;
		}
		else {
			std::cout << "Failed to create directory: " << path << std::endl;
			return false;
		}
	}
	else {
		std::cout << "Directory exists: " << path << std::endl;
		return true;
	}
}

void createMkFile(std::filesystem::path const & folderPath,
				  std::string const & filename,
				  std::string const & folderName,
				  std::string const & source) {

	std::string relativePath(folderPath.string(), source.length());
	std::replace(relativePath.begin(), relativePath.end(), '\\', '/');

	std::string relativePathUppercase(relativePath, 1);
	std::for_each(relativePathUppercase.begin(), relativePathUppercase.end(), [](char & c) {
		c = ::toupper(c);
	});

	std::cout << "Writing mkfile..." << std::endl;
	std::ofstream ofs(filename);
	ofs << "#" << relativePath.substr(1, relativePath.length()) << std::endl << std::endl;
	ofs << "LOCAL_PATH:= $(call my-dir)" << std::endl << std::endl;
	ofs << "SRC_" << relativePathUppercase << " := $(LOCAL_PATH)" << std::endl << std::endl;
	ofs << "SRC_" << relativePathUppercase << "_FILES := $(call SRC_FILES)" << relativePath << std::endl << std::endl;
	ofs << "LOCAL_EXPORT_C_INCLUDES := $(call TOP_PATH)../" << std::endl << std::endl;
	ofs << "LOCAL_SRC_FILES += \\" << std::endl;
	for (auto& p : std::filesystem::directory_iterator(folderPath)) {
		if (!p.is_directory()) {
			ofs << "	$(SRC_" << relativePathUppercase << "_FILES)/" << p.path().filename().string() << "	\\" << std::endl;
		}
	}

	ofs << std::endl;

	for (auto& p : std::filesystem::directory_iterator(folderPath)) {
		if (p.is_directory() && !p.is_symlink()) {
			ofs << "include $(SRC_" << relativePathUppercase << ")/" << p.path().filename().string() << "/android.mk" << std::endl;
		}
	}

	ofs.close();
	std::cout << "Finished writing mkfile" << std::endl;
}

std::string getMkFilename(std::string const & path) {
	constexpr auto* const NAME = "/android.mk";
	constexpr auto const NAME_LENGTH = 10;

	std::string filename;
	filename.reserve(path.length() + NAME_LENGTH);
	filename.append(path).append(NAME);
	std::cout << "Filename: " << filename << std::endl;
	

	return std::move(filename);
}

int main()
{
	std::string const SOURCE("source");
	constexpr auto* const OUTPUT = "output";
	constexpr auto const OUTPUT_LENGTH = 6;
	createDirectory(OUTPUT);

	for (auto& p : std::filesystem::recursive_directory_iterator(SOURCE)) {
		if (p.is_directory() && !p.is_symlink()) {
			std::string folderToCreate;
			folderToCreate.reserve(OUTPUT_LENGTH + p.path().string().length());
			folderToCreate.append(OUTPUT).append(p.path().string(), SOURCE.length());
			auto directoryExists = createDirectory(folderToCreate);
			
			if (directoryExists) {
				createMkFile(p.path(),
							 getMkFilename(folderToCreate),
							 p.path().stem().string(),
							 SOURCE);
			}
		}
	}

	std::cout << std::endl << "Finished creating mkfiles" << std::endl;
	auto i = 0;
	std::cin >> i;
}
