/*
  Stockfish, a UCI chess playing engine derived from Glaurung 2.1
  Copyright (C) 2004-2023 The Stockfish developers (see AUTHORS file)

  Stockfish is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Stockfish is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <cstddef>
#include <array>
#include <functional>
#include <sstream>

#include "bitboard.h"
#include "evaluate.h"
#include "misc.h"
#include "position.h"
#include "search.h"
#include "thread.h"
#include "tune.h"
#include "types.h"
#include "uci.h"

#include "kvm_api.hpp"
namespace Stockfish {
	std::function<void(std::stringstream&)> search_complete;
}
using namespace Stockfish;

static void on_get(const char *commands, const char *config)
{
	(void)config;

    search_complete = [] (std::stringstream& ss) {
		Backend::response(200, "text/plain", ss.str());
	};

	printf("Commands: %s\n", commands);
	fflush(stdout);
	char * arguments[] = {"stockfish", (char *)commands};

	UCI::loop(sizeof(arguments) / sizeof(arguments[0]), arguments);

    Threads.set(0);
}
static void
on_post(const char *, const char *,
	const char *content_type, const uint8_t* data, const size_t len)
{
    search_complete = [] (std::stringstream& ss) {
		Backend::response(200, "text/plain", ss.str());
	};

	std::string commands {(const char *)data, len};
	char * arguments[] = {"stockfish", commands.data()};

	UCI::loop(sizeof(arguments) / sizeof(arguments[0]), arguments);

    Threads.set(0);
}

int main(int argc, char* argv[]) {

  std::cout << engine_info() << std::endl;

  CommandLine::init(argc, argv);
  UCI::init(Options);
  Tune::init();
  Bitboards::init();
  Position::init();
  Threads.set(size_t(Options["Threads"]));
  Search::clear(); // After threads are up
  Eval::NNUE::init();

  if (IS_LINUX_MAIN())
  {
	/* Command-line test */
    search_complete = [] (std::stringstream& ss) {
		std::string result(ss.str());
    	printf("%s", result.c_str());
	};

    std::array<char *, 3> arguments = {
      "stockfish",
	  "position startpos moves e2e4\n",
	  "go movetime 1000\n"
    };

    UCI::loop(arguments.size(), arguments.data());

    Threads.set(0);
    return 0;
  }
  else
  {
	/* Compute callback */
    set_backend_get(on_get);
    set_backend_post(on_post);
    wait_for_requests();
  }
}
