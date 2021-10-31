#include "md5.hh"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#include <sys/stat.h>

namespace fs = std::filesystem;

int main(int argc, char * * argv) {
	
	if (argc <= 1) {
		std::cout << "usage: x4encat <archive name>" << std::endl;
		exit(1);
	}
	
	std::string archive_name = argv[1];
	fs::path dir = archive_name;
	
	if (!fs::is_directory(dir)) {
		std::cout << dir << " is not a directory" << std::endl;
		exit(1);
	}
	
	fs::path cat { archive_name + ".cat" }, dat { archive_name + ".dat" };
	std::ofstream cat_f { cat };
	std::filebuf dat_f;
	
	if (!cat_f) {
		std::cout << "could not open " << cat << " for writing" << std::endl;
		exit(1);
	}
	
	if (!dat_f.open(dat, std::ios_base::out | std::ios_base::binary)) {
		std::cout << "could not open " << dat << " for writing" << std::endl;
		exit(1);
	}
	
	for ( auto const & iter : fs::recursive_directory_iterator {dir} ) {
		if (!fs::is_regular_file(iter)) continue;
		
		std::vector<char> buffer;
		buffer.resize(fs::file_size(iter));
		
		std::filebuf file;
		if (!file.open(iter.path(), std::ios_base::in | std::ios_base::binary)) {
			std::cout << "could not open " << iter << " for reading" << std::endl;
			exit(1);
		}
		
		file.sgetn(buffer.data(), buffer.size());
		
		MD5 md5;
		md5.update(buffer.data(), buffer.size());
		md5.finalize();
		
		dat_f.sputn(buffer.data(), buffer.size());
		
		struct stat attr;
		stat(iter.path().c_str(), &attr);
		cat_f << iter.path().c_str() + archive_name.size() + 1 << " " << fs::file_size(iter) << " " << attr.st_mtime << " " << md5.hexdigest() << "\n";
		
		std::cout << iter << std::endl;
	}
	
	return 0;
}
