/**
 * This file is offensive, contains huge amounts of copy pasta and
 * should not be viewed by anyone.
 */
#include <common/Code.hpp>
#include <common/Header.hpp>
#include <common/Reader.hpp>
#include <common/Writer.hpp>
#include <common/messages.hpp>

#include <utility/crypto/Sha256.hpp>
#include <utility/string.hpp>
#include <utility/zlib/Deflate.hpp>
#include <utility/zlib/Inflate.hpp>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

int print_usage()
{
	std::cout << "usage: chester <command>\n";
	std::cout << "\n";
	std::cout << "available commands:\n";
	//std::cout << "  clean  removes any stashed files that have been verified to exist on the remote\n";
	std::cout << "  fetch  scans chester.txt and downloads missing files from the remote\n";
	std::cout << "  open   scans chester.txt for files to decompress\n";
	std::cout << "    WARNING: this command will replace any already-existing files with the same name\n";
	std::cout << "  pull   fetch followed by open\n";
	std::cout << "  push   transfers any stashed files to the remote\n";
	std::cout << "  stash  scans chester.txt for files to compress and store locally\n";
	std::cout << "  status displays the status of chester.txt\n";
	//std::cout << "  sync   push followed by pull\n";

	return 0;
}

bool compute_hash_of_file(const std::string & filename,
                          chester::common::Code & code)
{
	std::ifstream file(filename, std::ifstream::binary);
	if (!file)
	{
		std::cerr << "cannot open \""
		          << filename
		          << "\"\n";
		return false;
	}

	chester::utility::crypto::Sha256 sha_code;

	uint8_t buffer[1024]; // arbitrary

	do
	{
		file.read(reinterpret_cast<char *>(buffer), sizeof(buffer));
		sha_code.update(buffer, file.gcount());
	}
	while (!file.eof());

	sha_code.finalize(code.data());
	return true;
}

int run_fetch(const int argc, const char *const argv[])
{
	debug_printline("reading \".chester.d/remote\"...");
	// read .chester.d/remote
	std::string ipaddress;
	uint16_t portno;
	std::string projectname;
	{
		std::ifstream ifile(".chester.d/remote");
		if (!ifile)
		{
			std::cerr << "cannot open \".chester.d/remote\" for reading\n";
			return -1;
		}

		std::vector<std::string> lines;
		chester::utility::split(ifile, '\n', lines, true);
		std::vector<std::string> words;
		for (auto && line : lines)
		{
			words.clear();
			chester::utility::split(line, '=', words, true);

			switch (words.size())
			{
			case 0:
				continue;
			case 2:
				if (words[0] == "ipaddress") ipaddress = words[1];
				if (words[0] == "portno") portno = std::stoi(words[1]);
				if (words[0] == "projectname") projectname = words[1];
				break;
			default:
				std::cerr << "unexpected number of words in \".chester/remote\" at line ?\n";
				return -1;
			}
		}
	}
	debug_printline("ipaddress=", ipaddress);
	debug_printline("portno=", portno);
	debug_printline("projectname=", projectname);

	// each resource consists of a filename followed by an optional code
	std::vector<std::pair<std::string, std::string>> resources;
	{
		// read chester.txt
		std::ifstream ifile("chester.txt");
		if (!ifile)
		{
			std::cerr << "cannot open \"chester.txt\" for reading\n";
			return -1;
		}

		std::vector<std::string> lines;
		chester::utility::split(ifile, '\n', lines, true);
		std::vector<std::string> words;
		for (auto && line : lines)
		{
			words.clear();
			chester::utility::split(line, ' ', words, true);

			switch (words.size())
			{
			case 0:
				continue;
			case 1:
				resources.emplace_back(words[0], "");
				break;
			case 2:
				resources.emplace_back(words[0], words[1]);
				break;
			default:
				std::cerr << "unexpected number of words in \"chester.txt\" at line ?\n";
				return -1;
			}
		}
	}

	// the query contains a list of codes that needs to be pulled
	chester::common::msg::query_t query;
	query.codes.reserve(resources.size());
	for (auto && resource : resources)
	{
		if (resource.second.empty())
			continue;

		const auto code = chester::utility::from_string<chester::common::Code>(resource.second);
		const auto filename = chester::utility::to_string(".chester.d/resource/", code);
		{
			std::ifstream ifile(filename);
			if (ifile)
				continue;
		}

		query.codes.push_back(code);
	}
	if (query.codes.empty())
		return 0;

	const auto  read_end = 0;
	const auto write_end = 1;
	int  read_pipe[2];
	int write_pipe[2];
	pipe( read_pipe);
	pipe(write_pipe);

	const auto pid = fork();
	if (pid == 0) // child
	{
		dup2(write_pipe[ read_end],  STDIN_FILENO); // read from write
		dup2( read_pipe[write_end], STDOUT_FILENO); // write to read

		close( read_pipe[ read_end]);
		close( read_pipe[write_end]);
		close(write_pipe[ read_end]);
		close(write_pipe[write_end]);

		close(STDERR_FILENO);

		const auto arg_port = chester::utility::to_string("-p", portno);
		const auto arg_host = chester::utility::to_string("chester@", ipaddress);

		execlp("ssh", "ssh", arg_port.c_str(), arg_host.c_str(), "./server", (char *)nullptr);

		debug_assert(false);
	}
	debug_assert(pid != -1);

	close( read_pipe[write_end]);
	close(write_pipe[ read_end]);

	chester::common::Writer writer(write_pipe[write_end]);
	chester::common::Reader reader( read_pipe[ read_end]);

	chester::common::Header header;
	reader(header, sizeof(chester::common::Header));
	debug_assert(header.id == chester::common::msg::id_of<chester::common::msg::ping_t>::value);
	// -----------------
	writer(query);
	std::cout << "queried " << query.codes.size() << " codes:\n";
	for (auto && code : query.codes)
		std::cout << code << "\n";

	reader(header, sizeof(chester::common::Header));
	debug_assert(header.id == chester::common::msg::id_of<chester::common::msg::query_t>::value);
	chester::common::msg::query_t answer;
	reader(answer, header.size);
	std::cout << "got back " << answer.codes.size() << " codes:\n";
	for (auto && code : answer.codes)
		std::cout << code << "\n";
	// -----------------
	std::cout << "pulling " << answer.codes.size() << " files:\n";
	for (auto && code : answer.codes)
	{
		std::cout << code << "\n";

		const auto filename = chester::utility::to_string(".chester.d/resources/", code);
		std::ofstream file(filename, std::ifstream::binary);
		if (!file)
		{
			std::cerr << "cannot open \""
			          << filename
			          << "\"\n";
			return -1;
		}

		chester::common::msg::pull_t pull;
		pull.code = code;
		writer(pull);

		reader(header, sizeof(chester::common::Header));
		debug_assert(header.id == chester::common::msg::id_of<chester::common::msg::push_t>::value);
		chester::common::msg::push_t push;
		reader(push, header.size);

		uint8_t buffer[1024]; // arbitrary

		std::size_t count = 0;
		while (count < push.size)
		{
			const auto amount = std::min(push.size - count, sizeof(buffer));
			reader(buffer, amount);
			file.write(reinterpret_cast<const char *>(buffer), amount);
			count += amount;
		}
	}
	// -----------------
	close( read_pipe[ read_end]);
	close(write_pipe[write_end]);

	return 0;
}

int run_open(const int argc, const char *const argv[])
{
	// each resource consists of a filename followed by an optional code
	std::vector<std::pair<std::string, std::string>> resources;
	{
		// read chester.txt
		std::ifstream ifile("chester.txt");
		if (!ifile)
		{
			std::cerr << "cannot open \"chester.txt\" for reading\n";
			return -1;
		}

		std::vector<std::string> lines;
		chester::utility::split(ifile, '\n', lines, true);
		std::vector<std::string> words;
		for (auto && line : lines)
		{
			words.clear();
			chester::utility::split(line, ' ', words, true);

			switch (words.size())
			{
			case 0:
				continue;
			case 1:
				resources.emplace_back(words[0], "");
				break;
			case 2:
				resources.emplace_back(words[0], words[1]);
				break;
			default:
				std::cerr << "unexpected number of words in \"chester.txt\" at line ?\n";
				return -1;
			}
		}
	}
	// find column width
	std::size_t column = 0;
	for (auto && resource : resources)
		column = std::max(column, resource.first.size());
	column += 1; // minimum amount of space between the columns
	// compute hash codes
	std::vector<chester::common::Code> codes;
	codes.reserve(resources.size());
	for (auto && resource : resources)
	{
		codes.emplace_back();
		if (!compute_hash_of_file(resource.first, codes.back()))
			codes.back() = chester::common::Code::null();
	}

	// the stash contains a list of resources ready to be pushed
	std::vector<chester::common::Code> stash;
	{
		// read stash
		std::ifstream ifile(".chester.d/stash");
		if (ifile)
		{
			std::vector<std::string> lines;
			chester::utility::split(ifile, '\n', lines, true);
			stash.reserve(lines.size());
			for (auto && line : lines)
			{
				stash.emplace_back();
				chester::utility::from_string(line, stash.back());
			}
		}
	}

	// validate
	std::cout << "validating...\n";
	std::vector<int> statuses(resources.size());
	constexpr int status_up_to_date = 0;
	constexpr int status_found = 1;
	constexpr int status_missing = 2;
	for (std::size_t i = 0; i < resources.size(); i++)
	{
		auto && resource = resources[i];
		auto && code = codes[i];
		auto && status = statuses[i];

		const auto padding = std::string(column - resource.first.size(), ' ');
		std::cout << resource.first
		          << padding;

		if (!resource.second.empty())
		{
			const auto codee = chester::utility::from_string<chester::common::Code>(resource.second);
			if (code == codee)
			{
				std::cout << "up to date\n";
				status = status_up_to_date;
				continue;
			}
			const auto filename = chester::utility::to_string(".chester.d/resources/", codee);
			{
				std::ifstream file(filename);
				if (file)
				{
					std::cout << "found\n";
					status = status_found;
					continue;
				}
			}
		}
		std::cout << "missing\n";
		status = status_missing;
	}

	// decompress
	std::cout << "decompressing...\n";
	for (std::size_t i = 0; i < resources.size(); i++)
	{
		auto && resource = resources[i];
		//auto && code = codes[i];
		auto && status = statuses[i];

		if (status != status_found)
			continue;

		const auto codee = chester::utility::from_string<chester::common::Code>(resource.second);

		const auto padding = std::string(column - resource.first.size(), ' ');
		std::cout << resource.first
		          << padding
		          << codee
		          << "\n";

		// open input/output files
		const auto ifilename = chester::utility::to_string(".chester.d/resources/", codee);
		std::ifstream ifile(ifilename);
		if (!ifile)
		{
			std::cerr << "cannot open \""
			          << ifilename
			          << "\" for reading\n";
			return -1;
		}
		std::ofstream ofile(resource.first, std::ifstream::binary);
		if (!ofile)
		{
			std::cerr << "cannot open \""
			          << resource.first
			          << "\" for writing\n";
			return -1;
		}
		// the actual compression
		chester::utility::zlib::Inflate inflate;
		uint8_t ibuffer[1024]; // arbitrary
		uint8_t obuffer[1024]; // arbitrary

		do
		{
			ifile.read(reinterpret_cast<char *>(ibuffer), sizeof(ibuffer));
			inflate.decompress(ibuffer, ifile.gcount());

			std::size_t have;
			do
			{
				have = inflate.extract(obuffer);

				ofile.write(reinterpret_cast<char *>(obuffer), have);
			}
			while (have == sizeof(obuffer));
		}
		while (!inflate.isFinished());
	}
	return 0;
}

int run_pull(const int argc, const char *const argv[])
{
	const auto ret = run_fetch(argc, argv);
	if (ret) return ret;
	return run_open(argc, argv);
}

int run_push(const int argc, const char *const argv[])
{
	debug_printline("reading \".chester.d/remote\"...");
	// read .chester.d/remote
	std::string ipaddress;
	uint16_t portno;
	std::string projectname;
	{
		std::ifstream ifile(".chester.d/remote");
		if (!ifile)
		{
			std::cerr << "cannot open \".chester.d/remote\" for reading\n";
			return -1;
		}

		std::vector<std::string> lines;
		chester::utility::split(ifile, '\n', lines, true);
		std::vector<std::string> words;
		for (auto && line : lines)
		{
			words.clear();
			chester::utility::split(line, '=', words, true);

			switch (words.size())
			{
			case 0:
				continue;
			case 2:
				if (words[0] == "ipaddress") ipaddress = words[1];
				if (words[0] == "portno") portno = std::stoi(words[1]);
				if (words[0] == "projectname") projectname = words[1];
				break;
			default:
				std::cerr << "unexpected number of words in \".chester/remote\" at line ?\n";
				return -1;
			}
		}
	}
	debug_printline("ipaddress=", ipaddress);
	debug_printline("portno=", portno);
	debug_printline("projectname=", projectname);

	debug_printline("reading \".chester.d/stash\"...");
	// the stash contains a list of resources ready to be pushed
	chester::common::msg::query_t stash;
	{
		// read stash
		std::ifstream ifile(".chester.d/stash");
		if (!ifile)
		{
			std::cerr << "cannot open \".chester.d/stash\" for reading\n";
			return -1;
		}
		std::vector<std::string> lines;
		chester::utility::split(ifile, '\n', lines, true);
		stash.codes.reserve(lines.size());
		for (auto && line : lines)
		{
			stash.codes.emplace_back();
			chester::utility::from_string(line, stash.codes.back());
		}
	}
	debug_assert(!stash.codes.empty());

	const auto  read_end = 0;
	const auto write_end = 1;
	int  read_pipe[2];
	int write_pipe[2];
	pipe( read_pipe);
	pipe(write_pipe);

	const auto pid = fork();
	if (pid == 0) // child
	{
		dup2(write_pipe[ read_end],  STDIN_FILENO); // read from write
		dup2( read_pipe[write_end], STDOUT_FILENO); // write to read

		close( read_pipe[ read_end]);
		close( read_pipe[write_end]);
		close(write_pipe[ read_end]);
		close(write_pipe[write_end]);

		close(STDERR_FILENO);

		const auto arg_port = chester::utility::to_string("-p", portno);
		const auto arg_host = chester::utility::to_string("chester@", ipaddress);

		execlp("ssh", "ssh", arg_port.c_str(), arg_host.c_str(), "./server", (char *)nullptr);

		debug_assert(false);
	}
	debug_assert(pid != -1);

	close( read_pipe[write_end]);
	close(write_pipe[ read_end]);

	chester::common::Writer writer(write_pipe[write_end]);
	chester::common::Reader reader( read_pipe[ read_end]);

	chester::common::Header header;
	reader(header, sizeof(chester::common::Header));
	debug_assert(header.id == chester::common::msg::id_of<chester::common::msg::ping_t>::value);
	// -----------------
	writer(stash);
	std::cout << "queried " << stash.codes.size() << " codes:\n";
	for (auto && code : stash.codes)
		std::cout << code << "\n";

	reader(header, sizeof(chester::common::Header));
	debug_assert(header.id == chester::common::msg::id_of<chester::common::msg::query_t>::value);
	chester::common::msg::query_t answer;
	reader(answer, header.size);
	std::cout << "got back " << answer.codes.size() << " codes:\n";
	for (auto && code : answer.codes)
		std::cout << code << "\n";
	// -----------------
	std::size_t npushes = 0;
	for (auto && code : stash.codes)
	{
		if (std::find(std::begin(answer.codes), std::end(answer.codes), code) == std::end(answer.codes))
			++npushes;
	}
	std::cout << "pushing " << npushes << " files:\n";
	for (auto && code : stash.codes)
	{
		if (std::find(std::begin(answer.codes), std::end(answer.codes), code) != std::end(answer.codes))
			continue;
		std::cout << code << "\n";

		const auto filename = chester::utility::to_string(".chester.d/resources/", code);
		std::ifstream file(filename, std::ifstream::binary);
		if (!file)
		{
			std::cerr << "cannot open \""
			          << filename
			          << "\"\n";
			return -1;
		}

		file.seekg(0, std::ifstream::end);
		const auto size = file.tellg();
		file.seekg(0, std::ifstream::beg);

		chester::common::msg::push_t push;
		push.code = code;
		push.size = size;
		writer(push);

		uint8_t buffer[1024]; // arbitrary

		do
		{
			file.read(reinterpret_cast<char *>(buffer), sizeof(buffer));
			writer(buffer, file.gcount());
		}
		while (!file.eof());

		reader(header, sizeof(chester::common::Header));
		debug_assert(header.id == chester::common::msg::id_of<chester::common::msg::ping_t>::value);
	}
	// -----------------
	close( read_pipe[ read_end]);
	close(write_pipe[write_end]);

	// update stash
	debug_printline("updating \".chester.d/stash\"...");
	// assume everything went fine and remove it
	std::remove(".chester.d/stash");

	return 0;
}

int run_stash(const int argc, const char *const argv[])
{
	// each resource consists of a filename followed by an optional code
	std::vector<std::pair<std::string, std::string>> resources;
	{
		// read chester.txt
		std::ifstream ifile("chester.txt");
		if (!ifile)
		{
			std::cerr << "cannot open \"chester.txt\" for reading\n";
			return -1;
		}

		std::vector<std::string> lines;
		chester::utility::split(ifile, '\n', lines, true);
		std::vector<std::string> words;
		for (auto && line : lines)
		{
			words.clear();
			chester::utility::split(line, ' ', words, true);

			switch (words.size())
			{
			case 0:
				continue;
			case 1:
				resources.emplace_back(words[0], "");
				break;
			case 2:
				resources.emplace_back(words[0], words[1]);
				break;
			default:
				std::cerr << "unexpected number of words in \"chester.txt\" at line ?\n";
				return -1;
			}
		}
	}
	// find column width
	std::size_t column = 0;
	for (auto && resource : resources)
		column = std::max(column, resource.first.size());
	column += 1; // minimum amount of space between the columns
	// compute hash codes
	std::vector<chester::common::Code> hash_codes;
	hash_codes.reserve(resources.size());
	for (auto && resource : resources)
	{
		hash_codes.emplace_back();
		if (!compute_hash_of_file(resource.first, hash_codes.back()))
			return -1;
	}

	// the stash contains a list of resources ready to be pushed
	std::vector<chester::common::Code> stash;
	{
		// read stash
		std::ifstream ifile(".chester.d/stash");
		if (ifile)
		{
			std::vector<std::string> lines;
			chester::utility::split(ifile, '\n', lines, true);
			stash.reserve(lines.size());
			for (auto && line : lines)
			{
				stash.emplace_back();
				chester::utility::from_string(line, stash.back());
			}
		}
	}

	// validate
	std::cout << "validating...\n";
	std::vector<int> statuses(resources.size());
	constexpr int status_up_to_date = 0;
	constexpr int status_stashed = 1;
	constexpr int status_collision = 2;
	constexpr int status_new = 3;
	bool panicing = false;
	for (std::size_t i = 0; i < resources.size(); i++)
	{
		auto && resource = resources[i];
		auto && hash_code = hash_codes[i];
		auto && status = statuses[i];

		const auto padding = std::string(column - resource.first.size(), ' ');
		std::cout << resource.first
		          << padding;

		if (std::find(std::begin(stash), std::end(stash), hash_code) != std::end(stash))
		{
			std::cout << "stashed\n";
			status = status_stashed;
			continue;
		}
		if (!resource.second.empty() &&
		    chester::utility::from_string<chester::common::Code>(resource.second) == hash_code)
		{
			std::cout << "up to date\n";
			status = status_up_to_date;
			continue;
		}
		{
			std::ifstream ofile(chester::utility::concat(".chester.d/resources/", hash_code));
			if (ofile)
			{
				std::cout << "COLLISION!\n";
				status = status_collision;
				panicing = true;
				continue;
			}
		}
		std::cout << hash_code
		          << "\n";
		status = status_new;
	}
	if (panicing)
	{
		std::cerr << "panicing...\n";
		return -1;
	}

	// append stash
	std::ofstream ostash(".chester.d/stash", std::ofstream::app);
	if (!ostash)
	{
		std::cerr << "cannot open \""
		          << ".chester.d/stash"
		          << "\" for appending\n";
		return -1;
	}
	// compress
	std::cout << "compressing...\n";
	for (std::size_t i = 0; i < resources.size(); i++)
	{
		auto && resource = resources[i];
		auto && hash_code = hash_codes[i];
		auto && status = statuses[i];

		if (status != status_new)
			continue;

		const auto padding = std::string(column - resource.first.size(), ' ');
		std::cout << resource.first
		          << padding
		          << hash_code
		          << "\n";

		const auto ofilename = chester::utility::concat(".chester.d/resources/", hash_code);
		// open input/output files
		std::ofstream ofile(ofilename);
		if (!ofile)
		{
			std::cerr << "cannot open \""
			          << ofilename
			          << "\" for writing\n";
			return -1;
		}
		std::ifstream ifile(resource.first, std::ifstream::binary);
		if (!ifile)
		{
			std::cerr << "cannot open \""
			          << resource.first
			          << "\" for reading\n";
			return -1;
		}
		// the actual compression
		chester::utility::zlib::Deflate deflate;
		uint8_t ibuffer[1024]; // arbitrary
		uint8_t obuffer[1024]; // arbitrary

		do
		{
			ifile.read(reinterpret_cast<char *>(ibuffer), sizeof(ibuffer));
			deflate.compress(ibuffer, ifile.gcount(), ifile.eof());

			std::size_t have;
			do
			{
				have = deflate.extract(obuffer);

				ofile.write(reinterpret_cast<char *>(obuffer), have);
			}
			while (have == sizeof(obuffer));
		}
		while (!ifile.eof());

		// update stash
		ostash << hash_code
		       << "\n";
	}
	ostash.close();

	// write chester.txt
	std::ofstream ofile("chester.txt");
	if (!ofile)
	{
		std::cerr << "cannot open \"chester.txt\" for writing\n";
		return -1;
	}
	for (std::size_t i = 0; i < resources.size(); i++)
	{
		auto && resource = resources[i];
		auto && hash_code = hash_codes[i];

		ofile << resource.first
		      << std::string(column - resource.first.size(), ' ')
		      << hash_code
		      << "\n";
	}

	return 0;
}

int run_status(const int argc, const char *const argv[])
{
	// each resource consists of a filename followed by an optional code
	std::vector<std::pair<std::string, std::string>> resources;
	{
		// read chester.txt
		std::ifstream ifile("chester.txt");
		if (!ifile)
		{
			std::cerr << "cannot open \"chester.txt\" for reading\n";
			return -1;
		}

		std::vector<std::string> lines;
		chester::utility::split(ifile, '\n', lines, true);
		std::vector<std::string> words;
		for (auto && line : lines)
		{
			words.clear();
			chester::utility::split(line, ' ', words, true);

			switch (words.size())
			{
			case 0:
				continue;
			case 1:
				resources.emplace_back(words[0], "");
				break;
			case 2:
				resources.emplace_back(words[0], words[1]);
				break;
			default:
				std::cerr << "unexpected number of words in \"chester.txt\" at line ?\n";
				return -1;
			}
		}
	}
	// find column width
	std::size_t column = 0;
	for (auto && resource : resources)
		column = std::max(column, resource.first.size());
	column += 1; // minimum amount of space between the columns

	//
	std::vector<chester::common::Code> codes;
	codes.reserve(resources.size());
	for (auto && resource : resources)
	{
		codes.emplace_back();
		if (!compute_hash_of_file(resource.first, codes.back()))
			codes.back() = chester::common::Code::null();
	}
	// the stash contains a list of resources ready to be pushed
	std::vector<chester::common::Code> stash;
	{
		// read stash
		std::ifstream ifile(".chester.d/stash");
		if (ifile)
		{
			std::vector<std::string> lines;
			chester::utility::split(ifile, '\n', lines, true);
			stash.reserve(lines.size());
			for (auto && line : lines)
			{
				stash.emplace_back();
				chester::utility::from_string(line, stash.back());
			}
		}
	}

	// validate
	// std::cout << "validating...\n";
	std::vector<int> statuses(resources.size());
	// constexpr int status_ = 0;
	constexpr int status_up_to_date = 1;
	constexpr int status_found = 2;
	constexpr int status_missing = 3;
	constexpr int status_stashed = 4;
	constexpr int status_needs_restash = 5;
	constexpr int status_new = 6;
	constexpr int status_lost = 7;
	constexpr int status_edited = 8;
	for (std::size_t i = 0; i < resources.size(); i++)
	{
		auto && resource = resources[i];
		auto && code = codes[i];
		auto && status = statuses[i];

		const auto padding = std::string(column - resource.first.size(), ' ');
		std::cout << resource.first
		          << padding;

		if (code == chester::common::Code::null())
		{
			if (resource.second.empty())
			{
				std::cout << "lost\n";
				status = status_lost;
				continue;
			}
			{
				const auto filename = chester::utility::to_string(".chester.d/resources/", chester::utility::from_string<chester::common::Code>(resource.second));
				std::ifstream file(filename);
				if (file)
				{
					std::cout << "found (use 'open' to inflate)\n";
					status = status_found;
					continue;
				}
			}
			std::cout << "missing (use 'fetch' to check with repo)\n";
			status = status_missing;
			continue;
		}
		if (std::find(stash.begin(), stash.end(), code) != stash.end())
		{
			if (resource.second.empty())
			{
				std::cout << "needs restash\n";
				status = status_needs_restash;
				continue;
			}
			if (code == chester::utility::from_string<chester::common::Code>(resource.second))
			{
				std::cout << "stashed\n";
				status = status_stashed;
				continue;
			}
			std::cout << "needs restash?\n";
			status = status_needs_restash;
			continue;
		}
		{
			const auto filename = chester::utility::to_string(".chester.d/resources/", code);
			std::ifstream file(filename);
			if (file)
			{
				if (resource.second.empty())
				{
					std::cout << "needs restash??\n";
					status = status_needs_restash;
					continue;
				}
				if (code == chester::utility::from_string<chester::common::Code>(resource.second))
				{
					std::cout << "up to date\n";
					status = status_up_to_date;
					continue;
				}
				std::cout << "needs restash???\n";
				status = status_needs_restash;
				continue;
			}
		}
		if (resource.second.empty())
		{
			std::cout << "new\n";
			status = status_new;
			continue;
		}
		{
			const auto filename = chester::utility::to_string(".chester.d/resources/", chester::utility::from_string<chester::common::Code>(resource.second));
			std::ifstream file(filename);
			if (file)
			{
				std::cout << "edited\n";
				status = status_edited;
				continue;
			}
		}
		std::cout << "edited?\n";
		status = status_edited;
		continue;
	}
	return 0;
}

int main(const int argc, const char *const argv[])
{
	if (argc <= 1)
		return print_usage();

	if (std::string("fetch") == argv[1])
		return run_fetch(argc, argv);
	if (std::string("open") == argv[1])
		return run_open(argc, argv);
	if (std::string("pull") == argv[1])
		return run_pull(argc, argv);
	if (std::string("push") == argv[1])
		return run_push(argc, argv);
	if (std::string("stash") == argv[1])
		return run_stash(argc, argv);
	if (std::string("status") == argv[1])
		return run_status(argc, argv);

	return 0;
}
