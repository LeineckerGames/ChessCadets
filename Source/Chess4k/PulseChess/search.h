// Copyright 2013-2023 Phokham Nonava
//
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.
#pragma once
#include "protocol.h"
#include "position.h"
#include "movegenerator.h"
#include "evaluation.h"
#include <memory>
#include "HAL/ThreadingBase.h"
#include "HAL/PlatformProcess.h"
#include "Misc/ScopeLock.h"

namespace pulse {

	class Search final {
	public:
		explicit Search(Protocol& protocol);
		void newDepthSearch(Position& _position, int _searchDepth);
		void newNodesSearch(Position& _position, uint64_t _searchNodes);
		void newTimeSearch(Position& _position, uint64_t _searchTime);
		void newInfiniteSearch(Position& _position);
		void newClockSearch(Position& _position,
			uint64_t whiteTimeLeft, uint64_t whiteTimeIncrement, uint64_t blackTimeLeft,
			uint64_t blackTimeIncrement, int movesToGo);
		void newPonderSearch(Position& _position,
			uint64_t whiteTimeLeft, uint64_t whiteTimeIncrement, uint64_t blackTimeLeft,
			uint64_t blackTimeIncrement, int movesToGo);
		void reset();
		void start();
		void stop();
		void ponderhit();
		void quit();
		void run();

	private:
		class Timer final {
		public:
			Timer(bool& timerStopped, bool& doTimeManagement, int& currentDepth, const int& initialDepth, bool& abort);
			void start(uint64_t _searchTime);
			void stop();
		private:
			FCriticalSection mutex;
			FEvent* condition = nullptr;
			FRunnableThread* thread = nullptr;
			bool& timerStopped;
			bool& doTimeManagement;
			int& currentDepth;
			const int& initialDepth;
			bool& abort;
			void run(uint64_t _searchTime);

			// UE5 runnable wrapper
			class TimerRunnable : public FRunnable {
			public:
				TimerRunnable(Timer* owner, uint64_t searchTime)
					: owner(owner), searchTime(searchTime) {
				}
				uint32 Run() override {
					owner->run(searchTime);
					return 0;
				}
			private:
				Timer* owner;
				uint64_t searchTime;
			};
		};

		class Semaphore final {
		public:
			explicit Semaphore(int permits);
			~Semaphore();
			void acquire();
			void release();
			void drainPermits();
		private:
			int permits;
			FCriticalSection mutex;
			FEvent* condition = nullptr;
		};

		// UE5 runnable wrapper for Search
		class SearchRunnable : public FRunnable {
		public:
			SearchRunnable(Search* owner) : owner(owner) {}
			uint32 Run() override {
				owner->run();
				return 0;
			}
		private:
			Search* owner;
		};

		FRunnableThread* thread = nullptr;
		SearchRunnable* searchRunnable = nullptr;
		Semaphore wakeupSignal;
		Semaphore runSignal;
		Semaphore stopSignal;
		FCriticalSection sync;
		Protocol& protocol;
		bool running = false;
		bool shutdown = false;
		Position position;
		std::array<MoveGenerator, depth::MAX_PLY> moveGenerators;

		// Depth search
		int searchDepth;
		// Nodes search
		uint64_t searchNodes;
		// Time & Clock & Ponder search
		uint64_t searchTime;
		Timer timer;
		bool timerStopped;
		bool runTimer;
		bool doTimeManagement;

		// Search parameters
		MoveList<RootEntry> rootMoves;
		bool abort;
		uint64_t totalNodes;
		const int initialDepth = 1;
		int currentDepth;
		int currentMaxDepth;
		int currentMove;
		int currentMoveNumber;
		std::array<MoveVariation, depth::MAX_PLY + 1> pv;

		void checkStopConditions();
		void updateSearch(int ply);
		void searchRoot(int depth, int alpha, int beta);
		int search(int depth, int alpha, int beta, int ply);
		int quiescent(int depth, int alpha, int beta, int ply);
		static void savePV(int move, MoveVariation& src, MoveVariation& dest);
	};
}