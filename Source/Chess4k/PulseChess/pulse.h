// Copyright 2013-2023 Phokham Nonava
//
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.
#pragma once
#include <memory>
#include <string>
#include "search.h"
#include "notation.h"

namespace pulse {
	class Pulse final : public Protocol {
	public:
		void run();
		void sendBestMove(int bestMove, int ponderMove) override;
		void sendStatus(
			int currentDepth, int currentMaxDepth, uint64_t totalNodes, int currentMove,
			int currentMoveNumber) override;
		void sendStatus(
			bool force, int currentDepth, int currentMaxDepth, uint64_t totalNodes, int currentMove,
			int currentMoveNumber) override;
		void sendMove(RootEntry entry, int currentDepth, int currentMaxDepth, uint64_t totalNodes) override;
		void sendInfo(const std::string& message) override;
		static std::string fromMove(int move);
		void sendDebug(const std::string& message) override;

		// Public UE5 interface - call these directly from game code
		void receiveCommand(const std::string& line);

	private:
		bool debug = false;
		std::unique_ptr<Search> search = std::make_unique<Search>(*this);
		//std::chrono::system_clock::time_point startTime;
		//std::chrono::system_clock::time_point statusStartTime;
		std::unique_ptr<Position> currentPosition = std::make_unique<Position>(
			notation::toPosition(notation::STANDARDPOSITION));
		void receiveInitialize();
		void receiveDebug(const std::string& token);
		static void receiveReady();
		void receiveNewGame();
		void receivePosition(const std::string& args);
		void receiveGo(const std::string& args);
		void receivePonderHit();
		void receiveStop();
		void receiveQuit();
		int lastBestMove = 0;
		int lastPonderMove = 0;
	};
}