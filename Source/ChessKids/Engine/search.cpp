// Copyright 2013-2023 Phokham Nonava
//
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#include "search.h"
#include "HAL/PlatformProcess.h"

namespace pulse {

    // ---- Timer ----

    Search::Timer::Timer(bool& timerStopped, bool& doTimeManagement, int& currentDepth, const int& initialDepth, bool& abort)
        : timerStopped(timerStopped), doTimeManagement(doTimeManagement),
        currentDepth(currentDepth), initialDepth(initialDepth), abort(abort) {
        condition = FPlatformProcess::GetSynchEventFromPool(false);
    }

    void Search::Timer::run(uint64_t _searchTime) {
        bool signaled = condition->Wait((uint32)_searchTime);
        if (!signaled) {
            // Timer timed out
            timerStopped = true;
            if (!doTimeManagement || currentDepth > initialDepth) {
                abort = true;
            }
        }
    }

    void Search::Timer::start(uint64_t _searchTime) {
        TimerRunnable* runnable = new TimerRunnable(this, _searchTime);
        thread = FRunnableThread::Create(runnable, TEXT("PulseTimer"));
    }

    void Search::Timer::stop() {
        timerStopped = true;
        condition->Trigger();
        if (thread) {
            thread->WaitForCompletion();
            delete thread;
            thread = nullptr;
        }
    }

    // ---- Semaphore ----

    Search::Semaphore::Semaphore(int permits)
        : permits(permits) {
        condition = FPlatformProcess::GetSynchEventFromPool(false);
    }

    Search::Semaphore::~Semaphore() {
        FPlatformProcess::ReturnSynchEventToPool(condition);
    }

    void Search::Semaphore::acquire() {
        while (true) {
            {
                FScopeLock lock(&mutex);
                if (permits > 0) {
                    permits--;
                    return;
                }
            }
            condition->Wait();
        }
    }

    void Search::Semaphore::release() {
        FScopeLock lock(&mutex);
        permits++;
        condition->Trigger();
    }

    void Search::Semaphore::drainPermits() {
        FScopeLock lock(&mutex);
        permits = 0;
    }

    // ---- Search ----

    Search::Search(Protocol& protocol)
        : protocol(protocol),
        timer(timerStopped, doTimeManagement, currentDepth, initialDepth, abort),
        wakeupSignal(0), runSignal(0), stopSignal(0) {

        reset();

        searchRunnable = new SearchRunnable(this);
        thread = FRunnableThread::Create(searchRunnable, TEXT("PulseSearch"));
    }

    void Search::reset() {
        searchDepth = depth::MAX_DEPTH;
        searchNodes = std::numeric_limits<uint64_t>::max();
        searchTime = 0;
        runTimer = false;
        timerStopped = false;
        doTimeManagement = false;
        rootMoves.size = 0;
        abort = false;
        totalNodes = 0;
        currentDepth = initialDepth;
        currentMaxDepth = 0;
        currentMove = move::NOMOVE;
        currentMoveNumber = 0;
    }
    void Search::newDepthSearch(Position& _position, int _searchDepth) {
        if (_searchDepth < 1 || _searchDepth > depth::MAX_DEPTH) throw std::exception();
        if (running) throw std::exception();
        reset();
        position = _position;
        searchDepth = _searchDepth;
    }

    void Search::newNodesSearch(Position& _position, uint64_t _searchNodes) {
        if (_searchNodes < 1) throw std::exception();
        if (running) throw std::exception();
        reset();
        position = _position;
        searchNodes = _searchNodes;
    }

    void Search::newTimeSearch(Position& _position, uint64_t _searchTime) {
        if (_searchTime < 1) throw std::exception();
        if (running) throw std::exception();
        reset();
        position = _position;
        searchTime = _searchTime;
        runTimer = true;
    }

    void Search::newInfiniteSearch(Position& _position) {
        if (running) throw std::exception();
        reset();
        position = _position;
    }

    void Search::newClockSearch(Position& _position,
        uint64_t whiteTimeLeft, uint64_t whiteTimeIncrement, uint64_t blackTimeLeft,
        uint64_t blackTimeIncrement, int movesToGo) {
        newPonderSearch(_position, whiteTimeLeft, whiteTimeIncrement, blackTimeLeft, blackTimeIncrement, movesToGo);
        this->runTimer = true;
    }

    void Search::newPonderSearch(Position& _position,
        uint64_t whiteTimeLeft, uint64_t whiteTimeIncrement, uint64_t blackTimeLeft,
        uint64_t blackTimeIncrement, int movesToGo) {
        if (whiteTimeLeft < 1 && _position.activeColor == color::WHITE) throw std::exception();
        if (blackTimeLeft < 1 && _position.activeColor == color::BLACK) throw std::exception();
        if (movesToGo < 0) throw std::exception();
        if (running) throw std::exception();
        reset();
        position = _position;

        uint64_t timeLeft;
        uint64_t timeIncrement;
        if (_position.activeColor == color::WHITE) {
            timeLeft = whiteTimeLeft;
            timeIncrement = whiteTimeIncrement;
        }
        else {
            timeLeft = blackTimeLeft;
            timeIncrement = blackTimeIncrement;
        }

        uint64_t maxSearchTime = (uint64_t)(timeLeft * 0.95) - 1000L;
        if (maxSearchTime < 1) maxSearchTime = 1;

        this->searchTime = (maxSearchTime + (movesToGo - 1) * timeIncrement) / movesToGo;
        if (this->searchTime > maxSearchTime) this->searchTime = maxSearchTime;

        this->doTimeManagement = true;
    }

    void Search::start() {
        FScopeLock lock(&sync);
        if (!running) {
            wakeupSignal.release();
            runSignal.acquire();
        }
    }

    void Search::stop() {
        FScopeLock lock(&sync);
        if (running) {
            abort = true;
            stopSignal.acquire();
        }
    }

    void Search::ponderhit() {
        FScopeLock lock(&sync);
        if (running) {
            runTimer = true;
            timer.start(searchTime);
            if (currentDepth > initialDepth) {
                checkStopConditions();
            }
        }
    }

    void Search::quit() {
        FScopeLock lock(&sync);
        stop();
        shutdown = true;
        wakeupSignal.release();
        if (thread) {
            thread->WaitForCompletion();
            delete thread;
            thread = nullptr;
        }
    }

    void Search::run() {
        while (true) {
            wakeupSignal.acquire();

            if (shutdown) break;

            if (runTimer) {
                timer.start(searchTime);
            }

            MoveList<MoveEntry>& moves = moveGenerators[0].getLegalMoves(position, 1, position.isCheck());
            for (int i = 0; i < moves.size; i++) {
                int move = moves.entries[i]->move;
                rootMoves.entries[rootMoves.size]->move = move;
                rootMoves.entries[rootMoves.size]->pv.moves[0] = move;
                rootMoves.entries[rootMoves.size]->pv.size = 1;
                rootMoves.size++;
            }

            stopSignal.drainPermits();
            running = true;
            runSignal.release();

            for (int depth = initialDepth; depth <= searchDepth; depth++) {
                currentDepth = depth;
                currentMaxDepth = 0;
                protocol.sendStatus(false, currentDepth, currentMaxDepth, totalNodes, currentMove, currentMoveNumber);

                searchRoot(currentDepth, -value::INFINITE, value::INFINITE);
                rootMoves.sort();
                checkStopConditions();

                if (abort) break;
            }

            if (runTimer) {
                timer.stop();
            }

            protocol.sendStatus(true, currentDepth, currentMaxDepth, totalNodes, currentMove, currentMoveNumber);

            int bestMove = move::NOMOVE;
            int ponderMove = move::NOMOVE;
            if (rootMoves.size > 0) {
                bestMove = rootMoves.entries[0]->move;
                if (rootMoves.entries[0]->pv.size >= 2) {
                    ponderMove = rootMoves.entries[0]->pv.moves[1];
                }
            }

            protocol.sendBestMove(bestMove, ponderMove);

            running = false;
            stopSignal.release();
        }
    }

    void Search::checkStopConditions() {
        if (runTimer && doTimeManagement) {
            if (timerStopped) {
                abort = true;
            }
            else {
                if (rootMoves.size == 1) {
                    abort = true;
                }
                else if (value::isCheckmate(rootMoves.entries[0]->value)
                    && currentDepth >= (value::CHECKMATE - std::abs(rootMoves.entries[0]->value))) {
                    abort = true;
                }
            }
        }
    }

    void Search::updateSearch(int ply) {
        totalNodes++;
        if (ply > currentMaxDepth) currentMaxDepth = ply;
        if (searchNodes <= totalNodes) abort = true;
        pv[ply].size = 0;
        protocol.sendStatus(currentDepth, currentMaxDepth, totalNodes, currentMove, currentMoveNumber);
    }

    void Search::searchRoot(int depth, int alpha, int beta) {
        int ply = 0;
        updateSearch(ply);
        if (abort) return;

        for (int i = 0; i < rootMoves.size; i++) {
            rootMoves.entries[i]->value = -value::INFINITE;
        }

        for (int i = 0; i < rootMoves.size; i++) {
            int move = rootMoves.entries[i]->move;
            currentMove = move;
            currentMoveNumber = i + 1;
            protocol.sendStatus(false, currentDepth, currentMaxDepth, totalNodes, currentMove, currentMoveNumber);

            position.makeMove(move);
            int value = -search(depth - 1, -beta, -alpha, ply + 1);
            position.undoMove(move);

            if (abort) return;

            if (value > alpha) {
                alpha = value;
                rootMoves.entries[i]->value = value;
                savePV(move, pv[ply + 1], rootMoves.entries[i]->pv);
                protocol.sendMove(*rootMoves.entries[i], currentDepth, currentMaxDepth, totalNodes);
            }
        }

        if (rootMoves.size == 0) abort = true;
    }

    int Search::search(int depth, int alpha, int beta, int ply) {
        if (depth <= 0) return quiescent(0, alpha, beta, ply);

        updateSearch(ply);

        if (abort || ply == depth::MAX_PLY) return evaluation::evaluate(position);
        if (position.isRepetition() || position.hasInsufficientMaterial() || position.halfmoveClock >= 100) return value::DRAW;

        int bestValue = -value::INFINITE;
        int searchedMoves = 0;
        bool isCheck = position.isCheck();

        MoveList<MoveEntry>& moves = moveGenerators[ply].getMoves(position, depth, isCheck);
        for (int i = 0; i < moves.size; i++) {
            int move = moves.entries[i]->move;
            int value = bestValue;

            position.makeMove(move);
            if (!position.isCheck(color::opposite(position.activeColor))) {
                searchedMoves++;
                value = -search(depth - 1, -beta, -alpha, ply + 1);
            }
            position.undoMove(move);

            if (abort) return bestValue;

            if (value > bestValue) {
                bestValue = value;
                if (value > alpha) {
                    alpha = value;
                    savePV(move, pv[ply + 1], pv[ply]);
                    if (value >= beta) break;
                }
            }
        }

        if (searchedMoves == 0) {
            return isCheck ? -value::CHECKMATE + ply : value::DRAW;
        }

        return bestValue;
    }

    int Search::quiescent(int depth, int alpha, int beta, int ply) {
        updateSearch(ply);

        if (abort || ply == depth::MAX_PLY) return evaluation::evaluate(position);
        if (position.isRepetition() || position.hasInsufficientMaterial() || position.halfmoveClock >= 100) return value::DRAW;

        int bestValue = -value::INFINITE;
        int searchedMoves = 0;
        bool isCheck = position.isCheck();

        if (!isCheck) {
            bestValue = evaluation::evaluate(position);
            if (bestValue > alpha) {
                alpha = bestValue;
                if (bestValue >= beta) return bestValue;
            }
        }

        MoveList<MoveEntry>& moves = moveGenerators[ply].getMoves(position, depth, isCheck);
        for (int i = 0; i < moves.size; i++) {
            int move = moves.entries[i]->move;
            int value = bestValue;

            position.makeMove(move);
            if (!position.isCheck(color::opposite(position.activeColor))) {
                searchedMoves++;
                value = -quiescent(depth - 1, -beta, -alpha, ply + 1);
            }
            position.undoMove(move);

            if (abort) return bestValue;

            if (value > bestValue) {
                bestValue = value;
                if (value > alpha) {
                    alpha = value;
                    savePV(move, pv[ply + 1], pv[ply]);
                    if (value >= beta) break;
                }
            }
        }

        if (searchedMoves == 0 && isCheck) return -value::CHECKMATE + ply;

        return bestValue;
    }

    void Search::savePV(int move, MoveVariation& src, MoveVariation& dest) {
        dest.moves[0] = move;
        for (int i = 0; i < src.size; i++) {
            dest.moves[i + 1] = src.moves[i];
        }
        dest.size = src.size + 1;
    }

} // namespace pulse
