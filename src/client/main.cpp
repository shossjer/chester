
#include <common/network/Client.hpp>

#include <utility/string.hpp>
#include <utility/zlib/Deflate.hpp>
#include <utility/zlib/Inflate.hpp>

#include <wolfssl/wolfcrypt/rsa.h>
#include <wolfssl/wolfcrypt/sha256.h>

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
	std::cout << "  clean  removes any stashed files that have been verified to exist on the remote repository\n";
	std::cout << "  fetch  scans your .chester for files and downloads any such files from the remote repository\n";
	std::cout << "  open   scans your .chester for files to decompress\n";
	std::cout << "  pull   fetch followed by open\n";
	std::cout << "  push   transfers any stashed files to the remote repository\n";
	std::cout << "  stash  scans your .chester for files to compress and store locally\n";
	std::cout << "  sync   push followed by pull\n";

	return 0;
}

int run_push(const int argc, const char *const argv[])
{
	// read .chester.d/remote
	std::string ipaddress;
	uint16_t portno;
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
				break;
			default:
				std::cerr << "unexpected number of words in \".chester/remote\" at line ?\n";
				return -1;
			}
		}
	}

	// the stash contains a list of resources ready to be pushed
	std::vector<std::string> stash;
	{
		// read stash
		std::ifstream ifile(".chester.d/stash");
		if (!ifile)
		{
			std::cerr << "cannot open \".chester.d/stash\" for reading\n";
			return -1;
		}
		chester::utility::split(ifile, '\n', stash, true);
	}

	// connect to remote
	chester::common::network::Client client;
	client.connect(ipaddress.c_str(), portno);

	// query stash


	// try
	// {
	// 	std::string line;

	// 	while (std::getline(std::cin, line))
	// 	{
	// 		std::size_t written = 0;
	// 		do
	// 			written += client.send(reinterpret_cast<const uint8_t *>(line.data()) + written, line.size() - written);
	// 		while (written < line.size());

	// 		std::size_t received = 0;
	// 		while (received < written)
	// 		{
	// 			uint8_t buffer[1];
	// 			received += client.receive(buffer, 1);
	// 			std::cout << static_cast<char>(buffer[0]);
	// 		}
	// 		std::cout << "\n";
	// 	}
	// }
	// catch (chester::common::network::recv_closed & x)
	// {
	// }
	return 0;
}

bool compute_hash_of_file(const std::string & filename,
                          std::array<unsigned char, 32> & hash_code)
{
	std::ifstream file(filename, std::ifstream::binary);
	if (!file)
	{
		std::cerr << "cannot open \""
		          << filename
		          << "\"\n";
		return false;
	}

	Sha256 sha_code[1];
	const auto ret_init = wc_InitSha256(sha_code);
	debug_assert(ret_init == 0);

	char buffer[1024]; // arbitrary

	while (file.read(buffer, sizeof(buffer)))
	{
		const auto ret_update = wc_Sha256Update(sha_code, reinterpret_cast<const byte *>(buffer), file.gcount());
		debug_assert(ret_update == 0);
	}
	const auto ret_final = wc_Sha256Final(sha_code, hash_code.data());
	debug_assert(ret_final == 0);
	return true;
}

int run_stash(const int argc, const char *const argv[])
{
	// each resource consists of a filename followed by an optional key (the hash code)
	std::vector<std::pair<std::string, std::string>> resources;
	{
		// read .chester
		std::ifstream ifile(".chester");
		if (!ifile)
		{
			std::cerr << "cannot open \".chester\" for reading\n";
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
				std::cerr << "unexpected number of words in \".chester\" at line ?\n";
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
	std::vector<std::string> hash_codes;
	hash_codes.reserve(resources.size());
	for (auto && resource : resources)
	{
		std::array<unsigned char, 32> hash_code;
		if (!compute_hash_of_file(resource.first, hash_code))
			return -1;

		std::ostringstream stream;
		for (auto && byte : hash_code)
			stream << std::hex
			       << std::setw(2)
			       << std::setfill('0')
			       << int(byte);
		hash_codes.emplace_back(stream.str());
	}

	// the stash contains a list of resources ready to be pushed
	std::vector<std::string> stash;
	{
		// read stash
		std::ifstream ifile(".chester.d/stash");
		if (ifile)
		{
			chester::utility::split(ifile, '\n', stash, true);
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
		if (resource.second == hash_code)
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
		bool finished;

		do
		{
			finished = !ifile.read(reinterpret_cast<char *>(ibuffer), sizeof(ibuffer));
			deflate.compress(ibuffer, ifile.gcount(), finished);

			std::size_t have;
			do
			{
				have = deflate.extract(obuffer);

				ofile.write(reinterpret_cast<char *>(obuffer), have);
			}
			while (have == sizeof(obuffer));
		}
		while (!finished);

		// update stash
		ostash << hash_code
		       << "\n";
	}
	ostash.close();

	// write .chester
	std::ofstream ofile(".chester");
	if (!ofile)
	{
		std::cerr << "cannot open \".chester\" for writing\n";
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

// void run()
// {
// 	auto client = chester::common::network::Client{};
// 	// client.connect("192.168.0.50", 27960);
// 	client.connect("127.0.0.1", 27960);

// 	try
// 	{
// 		std::string line;

// 		while (std::getline(std::cin, line))
// 		{
// 			std::size_t written = 0;
// 			do
// 				written += client.send(reinterpret_cast<const uint8_t *>(line.data()) + written, line.size() - written);
// 			while (written < line.size());

// 			std::size_t received = 0;
// 			while (received < written)
// 			{
// 				uint8_t buffer[1];
// 				received += client.receive(buffer, 1);
// 				std::cout << static_cast<char>(buffer[0]);
// 			}
// 			std::cout << "\n";
// 		}
// 	}
// 	catch (chester::common::network::recv_closed & x)
// 	{
// 	}
// }

int main(const int argc, const char *const argv[])
{
	if (argc <= 1)
		return print_usage();

	if (std::string("push") == argv[1])
		return run_push(argc, argv);
	if (std::string("stash") == argv[1])
		return run_stash(argc, argv);

	std::ifstream ifile(".chester.d/resources/b5aa2a493c7d93a2a8644f3e356b450a6d4f4e5d69e7fe46ea2a9adb51f3c9e4");
	std::ofstream ofile("out.cpp");
	chester::utility::zlib::Inflate inflate;
	uint8_t ibuffer[1024];
	uint8_t obuffer[1024];
	while (true)
	{
		ifile.read((char *)ibuffer, sizeof(ibuffer));
		inflate.decompress(ibuffer, ifile.gcount());

		std::size_t amount;
		do
		{
			amount = inflate.extract(obuffer);
			ofile.write((const char *)obuffer, amount);
		}
		while (amount == sizeof(obuffer));
	}


	// chester::common::network::initiate();
	// run();
	// chester::common::network::cleanup();

	return 0;
}
