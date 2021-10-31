#include "md5.hh"

#include <charconv>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

namespace fs = std::filesystem;

std::vector<std::string_view> tokenize(std::string_view str, char delim) {
	std::vector<std::string_view> tokens;
	
	auto i_b = str.begin();
	for(auto i = str.begin(); i != str.end(); i++) {
		if (*i == delim) {
			tokens.emplace_back(i_b, i);
			i_b = i + 1;
		}
	}
	
	if (i_b < str.end()) tokens.emplace_back(i_b, str.end());
	
	return tokens;
}

int main(int argc, char * * argv) {
	
	if (argc <= 1) {
		std::cout << "an argument is required" << std::endl;
		exit(1);
	}
	
	std::string archive_name = argv[1];
	fs::path cat { archive_name + ".cat" }, dat { archive_name + ".dat" };
	
	auto cat_stat = fs::status(cat), dat_stat = fs::status(dat);
	
	if (cat_stat.type() != fs::file_type::regular) {
		std::cout << "could not find " << cat << std::endl;
		exit(1);
	};
	
	if (dat_stat.type() != fs::file_type::regular) {
		std::cout << "could not find " << dat << std::endl;
		exit(1);
	};
	
	std::filebuf cat_f, dat_f;
	
	if (!cat_f.open(cat, std::ios_base::in | std::ios_base::binary)) {
		std::cout << "could not open " << cat << " for reading" << std::endl;
		exit(1);
	}
	
	if (!dat_f.open(dat, std::ios_base::in | std::ios_base::binary)) {
		std::cout << "could not open " << dat << " for reading" << std::endl;
		exit(1);
	}
	
	auto cat_size = fs::file_size(cat), dat_size = fs::file_size(dat);
	
	std::string cat_buf;
	cat_buf.resize(cat_size);
	cat_f.sgetn(cat_buf.data(), cat_size);
	
	auto entry_lines = tokenize(cat_buf, '\n');
	
	size_t size_accum = 0;
	
	for (auto const & e : entry_lines) {
		
		auto tokens = tokenize(e, ' ');
		if (tokens.size() < 4) {
			std::cout << "parsing cat file failed, incomplete entry line" << std::endl;
			exit(1);
		}
		
		std::string_view const &
			t_path { tokens[0].begin(), tokens[tokens.size() - 4].end() },
			t_size = tokens[tokens.size() - 3],
			t_time = tokens[tokens.size() - 2],
			t_chck = tokens[tokens.size() - 1];
			
		fs::path path = fs::current_path() / archive_name / t_path;
		
		size_t size, offs = size_accum;
		std::from_chars(t_size.begin(), t_size.end(), size);
		size_accum += size;
		
		std::time_t time;
		std::from_chars(t_time.begin(), t_time.end(), time);
		
		std::cout << path << std::endl;
		
		fs::create_directories(path.parent_path());
		
		std::filebuf out;
		if (!out.open(path, std::ios_base::out | std::ios_base::binary)) {
			std::cout << "could not open " << path << " for writing" << std::endl;
			exit(1);
		}
		
		std::vector<char> buffer;
		buffer.resize(size);
		dat_f.sgetn(buffer.data(), size);
		
		MD5 md5;
		md5.update(buffer.data(), buffer.size());
		md5.finalize();
		auto digest = md5.hexdigest();
		
		if (digest != t_chck) {
			std::cout << "WARNING: checksum failed for \"" << t_path << "\" (\"" << t_chck << "\" vs \"" << digest << "\")" << std::endl;
		}
		
		out.sputn(buffer.data(), size);
		
		fs::last_write_time(path, std::chrono::file_clock::from_sys(std::chrono::system_clock::from_time_t(time)));
	}
	
	return 0;
}
