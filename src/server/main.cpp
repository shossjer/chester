
#include <config.h>

#include <common/Header.hpp>
#include <common/Reader.hpp>
#include <common/Writer.hpp>
#include <common/messages.hpp>

#include <utility/debug.hpp>
#include <utility/string.hpp>

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <fstream>

int main(const int argc, const char *const argv[])
{
	chester::common::Writer writer(STDOUT_FILENO);
	chester::common::Reader reader(STDIN_FILENO);

	writer(chester::common::msg::ping_t{});

	chester::common::Header header;
	while (reader(header, sizeof(chester::common::Header)))
	{
		switch (header.id)
		{
		case chester::common::msg::id_of<chester::common::msg::ping_t>::value:
		{
			writer(chester::common::msg::ping_t{});
			break;
		}
		case chester::common::msg::id_of<chester::common::msg::pull_t>::value:
		{
			chester::common::msg::pull_t pull;
			reader(pull, header.size);

			const auto filename = chester::utility::to_string("res/", pull.code);
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
			push.code = pull.code;
			push.size = size;
			writer(push);

			uint8_t buffer[1024]; // arbitrary

			do
			{
				file.read(reinterpret_cast<char *>(buffer), sizeof(buffer));
				writer(buffer, file.gcount());
			}
			while (!file.eof());
			break;
		}
		case chester::common::msg::id_of<chester::common::msg::push_t>::value:
		{
			chester::common::msg::push_t push;
			reader(push, header.size);
			debug_printline("pushing ", push.size, " bytes into \"res/", push.code, "\"");

			const auto filename = chester::utility::to_string("res/", push.code);
			std::ofstream file(filename, std::ifstream::binary);
			if (!file)
			{
				std::cerr << "cannot open \""
				          << filename
				          << "\"\n";
				return -1;
			}

			uint8_t buffer[1024]; // arbitrary

			std::size_t count = 0;
			while (count < push.size)
			{
				const auto amount = std::min(push.size - count, sizeof(buffer));
				reader(buffer, amount);
				file.write(reinterpret_cast<const char *>(buffer), amount);
				count += amount;
			}

			writer(chester::common::msg::ping_t{});
			break;
		}
		case chester::common::msg::id_of<chester::common::msg::query_t>::value:
		{
			chester::common::msg::query_t query;
			reader(query, header.size);

			chester::common::msg::query_t answer;
			answer.codes.reserve(query.codes.size());
			for (auto && code : query.codes)
			{
				const auto filename = chester::utility::to_string("res/", code);
				struct stat buffer;
				if (!stat(filename.c_str(), &buffer))
				{
					answer.codes.push_back(code);
				}
			}
			writer(answer);
			break;
		}
		default:
			debug_assert(false);
		}
	}

	return 0;
}
