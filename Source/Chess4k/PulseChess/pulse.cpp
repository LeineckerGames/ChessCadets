// Copyright 2013-2023 Phokham Nonava
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#include "pulse.h"
#include <cctype>
#include <string>

namespace pulse {

    // Helper to split a string by spaces
    static std::vector<std::string> splitTokens(const std::string& s) {
        std::vector<std::string> tokens;
        std::string remaining = s;
        size_t pos;
        while ((pos = remaining.find(' ')) != std::string::npos) {
            std::string token = remaining.substr(0, pos);
            if (!token.empty()) tokens.push_back(token);
            remaining = remaining.substr(pos + 1);
        }
        if (!remaining.empty()) tokens.push_back(remaining);
        return tokens;
    }

    void Pulse::run() {
        // In UE5 commands come directly via receiveCommand()
    }

    void Pulse::receiveCommand(const std::string& line) {
        std::vector<std::string> tokens = splitTokens(line);
        if (tokens.empty()) return;

        std::string command = tokens[0];
        std::string args = line.size() > command.size() + 1 ? line.substr(command.size() + 1) : "";

        if (command == "uci")           receiveInitialize();
        else if (command == "isready")  receiveReady();
        else if (command == "ucinewgame") receiveNewGame();
        else if (command == "debug")    receiveDebug(args);
        else if (command == "position") receivePosition(args);
        else if (command == "go")       receiveGo(args);
        else if (command == "stop")     receiveStop();
        else if (command == "ponderhit") receivePonderHit();
        else if (command == "quit")     receiveQuit();
    }

    void Pulse::receiveQuit() {
        search->quit();
    }

    void Pulse::receiveInitialize() {
        search->stop();
    }

    void Pulse::receiveDebug(const std::string& args) {
        if (args == "on") debug = true;
        else if (args == "off") debug = false;
        else if (args.empty()) debug = !debug;
    }

    void Pulse::receiveReady() {
        // readyok - handled by UE5
    }

    void Pulse::receiveNewGame() {
        search->stop();
        *currentPosition = notation::toPosition(notation::STANDARDPOSITION);
    }

    void Pulse::receivePosition(const std::string& args) {
        search->stop();

        std::vector<std::string> tokens = splitTokens(args);
        if (tokens.empty()) return;

        int i = 0;
        if (tokens[i] == "startpos") {
            *currentPosition = notation::toPosition(notation::STANDARDPOSITION);
            i++;
        }
        else if (tokens[i] == "fen") {
            i++;
            std::string fen;
            while (i < (int)tokens.size() && tokens[i] != "moves") {
                fen += tokens[i++] + " ";
            }
            *currentPosition = notation::toPosition(fen);
        }
        else {
            throw std::exception();
        }

        // Skip "moves" keyword
        if (i < (int)tokens.size() && tokens[i] == "moves") i++;

        MoveGenerator moveGenerator;
        while (i < (int)tokens.size()) {
            std::string moveToken = tokens[i++];
            MoveList<MoveEntry>& moves = moveGenerator.getLegalMoves(*currentPosition, 1, currentPosition->isCheck());
            bool found = false;
            for (int j = 0; j < moves.size; j++) {
                int move = moves.entries[j]->move;
                if (fromMove(move) == moveToken) {
                    currentPosition->makeMove(move);
                    found = true;
                    break;
                }
            }
            if (!found) throw std::exception();
        }
    }

    void Pulse::receiveGo(const std::string& args) {
        search->stop();

        std::vector<std::string> tokens = splitTokens(args);
        if (tokens.empty()) {
            search->newInfiniteSearch(*currentPosition);
            search->start();
            return;
        }

        int i = 0;
        if (tokens[i] == "depth") {
            int searchDepth = std::stoi(tokens[++i]);
            search->newDepthSearch(*currentPosition, searchDepth);
        }
        else if (tokens[i] == "nodes") {
            uint64_t searchNodes = std::stoull(tokens[++i]);
            search->newNodesSearch(*currentPosition, searchNodes);
        }
        else if (tokens[i] == "movetime") {
            uint64_t searchTime = std::stoull(tokens[++i]);
            search->newTimeSearch(*currentPosition, searchTime);
        }
        else if (tokens[i] == "infinite") {
            search->newInfiniteSearch(*currentPosition);
        }
        else {
            uint64_t whiteTimeLeft = 1, whiteTimeIncrement = 0;
            uint64_t blackTimeLeft = 1, blackTimeIncrement = 0;
            int searchMovesToGo = 40;
            bool ponder = false;

            while (i < (int)tokens.size()) {
                std::string token = tokens[i++];
                if (token == "wtime")       whiteTimeLeft = std::stoull(tokens[i++]);
                else if (token == "winc")   whiteTimeIncrement = std::stoull(tokens[i++]);
                else if (token == "btime")  blackTimeLeft = std::stoull(tokens[i++]);
                else if (token == "binc")   blackTimeIncrement = std::stoull(tokens[i++]);
                else if (token == "movestogo") searchMovesToGo = std::stoi(tokens[i++]);
                else if (token == "ponder") ponder = true;
            }

            if (ponder) {
                search->newPonderSearch(*currentPosition, whiteTimeLeft, whiteTimeIncrement,
                    blackTimeLeft, blackTimeIncrement, searchMovesToGo);
            }
            else {
                search->newClockSearch(*currentPosition, whiteTimeLeft, whiteTimeIncrement,
                    blackTimeLeft, blackTimeIncrement, searchMovesToGo);
            }
        }

        search->start();
        //startTime = std::chrono::system_clock::now();
        //statusStartTime = startTime;
    }

    void Pulse::receivePonderHit() {
        search->ponderhit();
    }

    void Pulse::receiveStop() {
        search->stop();
    }

    void Pulse::sendBestMove(int bestMove, int ponderMove) {
        lastBestMove = bestMove;
        lastPonderMove = ponderMove;
    }

    void Pulse::sendStatus(int currentDepth, int currentMaxDepth, uint64_t totalNodes, int currentMove, int currentMoveNumber) {
        // Handled by UE5
    }

    void Pulse::sendStatus(bool force, int currentDepth, int currentMaxDepth, uint64_t totalNodes, int currentMove, int currentMoveNumber) {
        // Handled by UE5
    }

    void Pulse::sendMove(RootEntry entry, int currentDepth, int currentMaxDepth, uint64_t totalNodes) {
        // Handled by UE5
    }

    void Pulse::sendInfo(const std::string& message) {
        // Handled by UE5
    }

    void Pulse::sendDebug(const std::string& message) {
        // Handled by UE5
    }

    std::string Pulse::fromMove(int move) {
        std::string n;
        n += notation::fromSquare(move::getOriginSquare(move));
        n += notation::fromSquare(move::getTargetSquare(move));
        int promotion = move::getPromotion(move);
        if (promotion != piecetype::NOPIECETYPE) {
            n += (char)::tolower((unsigned char)notation::fromPieceType(promotion));
        }
        return n;
    }

} // namespace pulse