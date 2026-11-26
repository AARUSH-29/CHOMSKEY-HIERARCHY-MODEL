#include <algorithm>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <queue>
#include <set>
#include <string>
#include <utility>
#include <vector>

// ============================================================================
// MODULE 1: DFA / NFA SIMULATOR
// ============================================================================

struct DFA {
    std::string name;
    std::set<std::string> states;
    std::set<char> alphabet;
    std::string startState;
    std::set<std::string> acceptStates;
    std::map<std::pair<std::string, char>, std::string> transitions;

    bool validate(std::string& errorMsg) const {
        if (states.empty()) {
            errorMsg = "State set cannot be empty.";
            return false;
        }
        if (states.find(startState) == states.end()) {
            errorMsg = "Start state '" + startState + "' is not in states.";
            return false;
        }
        for (const auto& acc : acceptStates) {
            if (states.find(acc) == states.end()) {
                errorMsg = "Accept state '" + acc + "' is not in states.";
                return false;
            }
        }
        for (const auto& kv : transitions) {
            if (states.find(kv.first.first) == states.end() || states.find(kv.second) == states.end()) {
                errorMsg = "Transition references an undefined state.";
                return false;
            }
        }
        return true;
    }

    bool simulate(const std::string& input, bool verbose = true) const {
        std::string current = startState;
        if (verbose) {
            std::cout << "\n--- DFA Execution Trace (" << name << ") ---\n";
            std::cout << "Input string : \"" << input << "\"\n";
            std::cout << "Start state  : " << current << "\n\n";
        }

        for (size_t i = 0; i < input.size(); ++i) {
            char sym = input[i];
            if (!alphabet.empty() && alphabet.find(sym) == alphabet.end()) {
                if (verbose) {
                    std::cout << "Step " << (i + 1) << ": Symbol '" << sym
                              << "' not in alphabet -> REJECT\n";
                }
                return false;
            }
            auto it = transitions.find({current, sym});
            if (it == transitions.end()) {
                if (verbose) {
                    std::cout << "Step " << (i + 1) << ": No transition from (" << current
                              << ", '" << sym << "') -> REJECT\n";
                }
                return false;
            }
            std::string next = it->second;
            if (verbose) {
                std::cout << "Step " << std::setw(2) << (i + 1) << ": "
                          << std::setw(7) << current << " --('" << sym << "')--> "
                          << std::setw(7) << next
                          << " | Remaining: \"" << input.substr(i + 1) << "\"\n";
            }
            current = next;
        }

        bool accepted = (acceptStates.find(current) != acceptStates.end());
        if (verbose) {
            std::cout << "\nFinal state  : " << current
                      << (accepted ? " (Accepting)" : " (Non-accepting)") << "\n";
            std::cout << "Result       : " << (accepted ? "ACCEPTED" : "REJECTED") << "\n";
        }
        return accepted;
    }
};

struct NFA {
    std::string name;
    std::set<std::string> states;
    std::set<char> alphabet;
    std::string startState;
    std::set<std::string> acceptStates;
    // '#' represents epsilon
    std::map<std::pair<std::string, char>, std::set<std::string>> transitions;

    std::set<std::string> epsilonClosure(const std::set<std::string>& currentStates) const {
        std::set<std::string> closure = currentStates;
        std::queue<std::string> q;
        for (const auto& s : currentStates) q.push(s);

        while (!q.empty()) {
            std::string curr = q.front();
            q.pop();
            auto it = transitions.find({curr, '#'});
            if (it != transitions.end()) {
                for (const auto& nxt : it->second) {
                    if (closure.insert(nxt).second) {
                        q.push(nxt);
                    }
                }
            }
        }
        return closure;
    }

    std::set<std::string> move(const std::set<std::string>& currentStates, char symbol) const {
        std::set<std::string> nextStates;
        for (const auto& s : currentStates) {
            auto it = transitions.find({s, symbol});
            if (it != transitions.end()) {
                nextStates.insert(it->second.begin(), it->second.end());
            }
        }
        return nextStates;
    }

    bool validate(std::string& errorMsg) const {
        if (states.empty()) {
            errorMsg = "NFA state set cannot be empty.";
            return false;
        }
        if (states.find(startState) == states.end()) {
            errorMsg = "Start state '" + startState + "' not found.";
            return false;
        }
        return true;
    }

    static std::string formatStateSet(const std::set<std::string>& s) {
        if (s.empty()) return "{}";
        std::string res = "{";
        bool first = true;
        for (const auto& item : s) {
            if (!first) res += ", ";
            res += item;
            first = false;
        }
        res += "}";
        return res;
    }

    bool simulate(const std::string& input, bool verbose = true) const {
        std::set<std::string> current = epsilonClosure({startState});
        if (verbose) {
            std::cout << "\n--- NFA Execution Trace (" << name << ") ---\n";
            std::cout << "Input string        : \"" << input << "\"\n";
            std::cout << "Initial eps-closure : " << formatStateSet(current) << "\n\n";
        }

        for (size_t i = 0; i < input.size(); ++i) {
            char sym = input[i];
            std::set<std::string> moved = move(current, sym);
            std::set<std::string> next = epsilonClosure(moved);
            if (verbose) {
                std::cout << "Step " << std::setw(2) << (i + 1) << " ['" << sym << "']: "
                          << formatStateSet(current) << " -> " << formatStateSet(next)
                          << " | Remaining: \"" << input.substr(i + 1) << "\"\n";
            }
            current = next;
            if (current.empty()) {
                if (verbose) std::cout << "Active state set became empty -> REJECT\n";
                return false;
            }
        }

        bool accepted = false;
        for (const auto& s : current) {
            if (acceptStates.find(s) != acceptStates.end()) {
                accepted = true;
                break;
            }
        }
        if (verbose) {
            std::cout << "\nFinal active states : " << formatStateSet(current) << "\n";
            std::cout << "Result              : " << (accepted ? "ACCEPTED" : "REJECTED") << "\n";
        }
        return accepted;
    }
};

DFA presetDFAEnding01() {
    DFA d;
    d.name = "DFA: Strings ending in '01'";
    d.states = {"q0", "q1", "q2"};
    d.alphabet = {'0', '1'};
    d.startState = "q0";
    d.acceptStates = {"q2"};
    d.transitions = {
        {{"q0", '0'}, "q1"}, {{"q0", '1'}, "q0"},
        {{"q1", '0'}, "q1"}, {{"q1", '1'}, "q2"},
        {{"q2", '0'}, "q1"}, {{"q2", '1'}, "q0"}
    };
    return d;
}

DFA presetDFAEvenZeros() {
    DFA d;
    d.name = "DFA: Even number of '0's";
    d.states = {"q_even", "q_odd"};
    d.alphabet = {'0', '1'};
    d.startState = "q_even";
    d.acceptStates = {"q_even"};
    d.transitions = {
        {{"q_even", '0'}, "q_odd"}, {{"q_even", '1'}, "q_even"},
        {{"q_odd", '0'}, "q_even"}, {{"q_odd", '1'}, "q_odd"}
    };
    return d;
}

NFA presetNFAEnding01() {
    NFA n;
    n.name = "NFA: Strings ending in '01'";
    n.states = {"q0", "q1", "q2"};
    n.alphabet = {'0', '1'};
    n.startState = "q0";
    n.acceptStates = {"q2"};
    n.transitions = {
        {{"q0", '0'}, {"q0", "q1"}},
        {{"q0", '1'}, {"q0"}},
        {{"q1", '1'}, {"q2"}}
    };
    return n;
}

static DFA buildCustomDFA() {
    DFA d;
    d.name = "Custom DFA";
    int nStates;
    std::cout << "Number of states: ";
    std::cin >> nStates;
    std::cout << "Enter state names: ";
    for (int i = 0; i < nStates; ++i) {
        std::string s;
        std::cin >> s;
        d.states.insert(s);
    }
    int nAlpha;
    std::cout << "Alphabet size: ";
    std::cin >> nAlpha;
    std::cout << "Enter alphabet symbols: ";
    for (int i = 0; i < nAlpha; ++i) {
        char c;
        std::cin >> c;
        d.alphabet.insert(c);
    }
    std::cout << "Start state: ";
    std::cin >> d.startState;
    int nAcc;
    std::cout << "Number of accept states: ";
    std::cin >> nAcc;
    std::cout << "Enter accept states: ";
    for (int i = 0; i < nAcc; ++i) {
        std::string s;
        std::cin >> s;
        d.acceptStates.insert(s);
    }
    int nTrans;
    std::cout << "Number of transitions: ";
    std::cin >> nTrans;
    std::cout << "Enter transitions as: <from_state> <symbol> <to_state>\n";
    for (int i = 0; i < nTrans; ++i) {
        std::string from, to;
        char sym;
        std::cin >> from >> sym >> to;
        d.transitions[{from, sym}] = to;
    }
    return d;
}

static NFA buildCustomNFA() {
    NFA n;
    n.name = "Custom NFA";
    int nStates;
    std::cout << "Number of states: ";
    std::cin >> nStates;
    std::cout << "Enter state names: ";
    for (int i = 0; i < nStates; ++i) {
        std::string s;
        std::cin >> s;
        n.states.insert(s);
    }
    std::cout << "Start state: ";
    std::cin >> n.startState;
    int nAcc;
    std::cout << "Number of accept states: ";
    std::cin >> nAcc;
    std::cout << "Enter accept states: ";
    for (int i = 0; i < nAcc; ++i) {
        std::string s;
        std::cin >> s;
        n.acceptStates.insert(s);
    }
    int nTrans;
    std::cout << "Number of transitions: ";
    std::cin >> nTrans;
    std::cout << "Enter transitions as: <from_state> <symbol_or_#_for_eps> <to_state>\n";
    for (int i = 0; i < nTrans; ++i) {
        std::string from, to;
        char sym;
        std::cin >> from >> sym >> to;
        n.transitions[{from, sym}].insert(to);
        if (sym != '#') n.alphabet.insert(sym);
    }
    return n;
}

void runFAModule() {
    while (true) {
        std::cout << "\n========================================\n";
        std::cout << "       DFA / NFA SIMULATOR              \n";
        std::cout << "========================================\n";
        std::cout << "1. Preset DFA: Strings ending in '01'\n";
        std::cout << "2. Preset DFA: Even number of '0's\n";
        std::cout << "3. Preset NFA: Strings ending in '01'\n";
        std::cout << "4. Custom DFA\n";
        std::cout << "5. Custom NFA\n";
        std::cout << "0. Back\n";
        std::cout << "Choose option: ";

        int choice;
        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            continue;
        }
        if (choice == 0) break;

        bool isNFA = (choice == 3 || choice == 5);
        DFA dfa;
        NFA nfa;
        std::string err;

        if (choice == 1) dfa = presetDFAEnding01();
        else if (choice == 2) dfa = presetDFAEvenZeros();
        else if (choice == 3) nfa = presetNFAEnding01();
        else if (choice == 4) dfa = buildCustomDFA();
        else if (choice == 5) nfa = buildCustomNFA();
        else {
            std::cout << "Invalid choice.\n";
            continue;
        }

        if (!isNFA && !dfa.validate(err)) {
            std::cout << "Validation error: " << err << "\n";
            continue;
        }
        if (isNFA && !nfa.validate(err)) {
            std::cout << "Validation error: " << err << "\n";
            continue;
        }

        while (true) {
            std::cout << "\nEnter test string ('back' to switch machine, 'eps' for empty): ";
            std::string input;
            std::cin >> input;
            if (input == "back") break;
            if (input == "eps") input = "";
            if (isNFA) nfa.simulate(input, true);
            else dfa.simulate(input, true);
        }
    }
}

// ============================================================================
// MODULE 2: CFG PARSER
// ============================================================================

struct ParseNode {
    std::string symbol;
    std::vector<std::shared_ptr<ParseNode>> children;
};

struct BackPtr {
    bool isTerminal = false;
    char termChar = 0;
    int splitK = -1;
    std::string leftNT;
    std::string rightNT;
};

struct CFG {
    std::string name;
    std::string startSymbol = "S";
    std::set<std::string> nonTerminals;
    std::set<char> terminals;
    std::map<std::string, std::vector<std::pair<std::string, std::string>>> binaryRules;
    std::map<std::string, std::set<char>> terminalRules;
    bool derivesEpsilon = false;

    void printRules() const {
        std::cout << "\nProduction Rules (" << name << "):\n";
        if (derivesEpsilon) {
            std::cout << "  " << startSymbol << " -> eps\n";
        }
        for (const auto& kv : binaryRules) {
            for (const auto& rhs : kv.second) {
                std::cout << "  " << kv.first << " -> " << rhs.first << " " << rhs.second << "\n";
            }
        }
        for (const auto& kv : terminalRules) {
            for (char ch : kv.second) {
                std::cout << "  " << kv.first << " -> '" << ch << "'\n";
            }
        }
    }

    static std::shared_ptr<ParseNode> buildTree(
        const std::string& sym, int i, int j,
        const std::vector<std::vector<std::map<std::string, BackPtr>>>& bp
    ) {
        auto node = std::make_shared<ParseNode>();
        node->symbol = sym;
        auto cellIt = bp[i][j].find(sym);
        if (cellIt == bp[i][j].end()) return node;
        const BackPtr& ptr = cellIt->second;
        if (ptr.isTerminal) {
            auto leaf = std::make_shared<ParseNode>();
            leaf->symbol = std::string(1, ptr.termChar);
            node->children.push_back(leaf);
        } else {
            node->children.push_back(buildTree(ptr.leftNT, i, ptr.splitK, bp));
            node->children.push_back(buildTree(ptr.rightNT, ptr.splitK + 1, j, bp));
        }
        return node;
    }

    static void printTree(const std::shared_ptr<ParseNode>& node, const std::string& prefix = "", bool isLast = true) {
        if (!node) return;
        std::cout << prefix << (isLast ? "`-- " : "|-- ") << node->symbol << "\n";
        std::string next = prefix + (isLast ? "    " : "|   ");
        for (size_t i = 0; i < node->children.size(); ++i) {
            printTree(node->children[i], next, i + 1 == node->children.size());
        }
    }

    bool parseCYK(const std::string& input, bool verbose = true) const {
        if (verbose) {
            std::cout << "\n--- CFG Parser (" << name << ") ---\n";
            std::cout << "Input: \"" << input << "\"\n";
        }
        if (input.empty()) {
            if (verbose) {
                std::cout << "Result: " << (derivesEpsilon ? "ACCEPTED (eps)" : "REJECTED") << "\n";
            }
            return derivesEpsilon;
        }

        int n = static_cast<int>(input.size());
        std::vector<std::vector<std::set<std::string>>> table(n, std::vector<std::set<std::string>>(n));
        std::vector<std::vector<std::map<std::string, BackPtr>>> bp(n, std::vector<std::map<std::string, BackPtr>>(n));

        for (int i = 0; i < n; ++i) {
            char c = input[i];
            for (const auto& kv : terminalRules) {
                if (kv.second.count(c)) {
                    table[i][i].insert(kv.first);
                    bp[i][i][kv.first] = {true, c, -1, "", ""};
                }
            }
        }

        for (int len = 2; len <= n; ++len) {
            for (int i = 0; i <= n - len; ++i) {
                int j = i + len - 1;
                for (int k = i; k < j; ++k) {
                    for (const auto& kv : binaryRules) {
                        for (const auto& rhs : kv.second) {
                            if (table[i][k].count(rhs.first) && table[k + 1][j].count(rhs.second)) {
                                if (table[i][j].insert(kv.first).second) {
                                    bp[i][j][kv.first] = {false, 0, k, rhs.first, rhs.second};
                                }
                            }
                        }
                    }
                }
            }
        }

        bool accepted = (table[0][n - 1].count(startSymbol) > 0);
        if (verbose) {
            std::cout << "\nDP Table:\n";
            for (int len = n; len >= 1; --len) {
                std::cout << "Len " << len << ": ";
                for (int i = 0; i <= n - len; ++i) {
                    int j = i + len - 1;
                    std::cout << "[";
                    if (table[i][j].empty()) {
                        std::cout << "-";
                    } else {
                        bool first = true;
                        for (const auto& nt : table[i][j]) {
                            if (!first) std::cout << ",";
                            std::cout << nt;
                            first = false;
                        }
                    }
                    std::cout << "] ";
                }
                std::cout << "\n";
            }
            if (accepted) {
                std::cout << "\nParse Tree:\n";
                printTree(buildTree(startSymbol, 0, n - 1, bp));
            }
            std::cout << "\nResult: " << (accepted ? "ACCEPTED" : "REJECTED") << "\n";
        }
        return accepted;
    }
};

CFG presetCFGAnBn() {
    CFG g;
    g.name = "CFG: a^n b^n (n >= 0)";
    g.startSymbol = "S";
    g.nonTerminals = {"S", "X", "A", "B"};
    g.terminals = {'a', 'b'};
    g.derivesEpsilon = true;
    g.binaryRules["S"] = {{"A", "B"}, {"A", "X"}};
    g.binaryRules["X"] = {{"S", "B"}};
    g.terminalRules["A"] = {'a'};
    g.terminalRules["B"] = {'b'};
    return g;
}

CFG presetCFGBalancedParens() {
    CFG g;
    g.name = "CFG: Balanced Parentheses";
    g.startSymbol = "S";
    g.nonTerminals = {"S", "X", "L", "R"};
    g.terminals = {'(', ')'};
    g.derivesEpsilon = true;
    g.binaryRules["S"] = {{"L", "R"}, {"S", "S"}, {"L", "X"}};
    g.binaryRules["X"] = {{"S", "R"}};
    g.terminalRules["L"] = {'('};
    g.terminalRules["R"] = {')'};
    return g;
}

void runCFGModule() {
    while (true) {
        std::cout << "\n========================================\n";
        std::cout << "            CFG PARSER                  \n";
        std::cout << "========================================\n";
        std::cout << "1. Preset CFG: a^n b^n (n >= 0)\n";
        std::cout << "2. Preset CFG: Balanced Parentheses\n";
        std::cout << "0. Back\n";
        std::cout << "Choose option: ";

        int choice;
        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            continue;
        }
        if (choice == 0) break;

        CFG g;
        if (choice == 1) g = presetCFGAnBn();
        else if (choice == 2) g = presetCFGBalancedParens();
        else {
            std::cout << "Invalid choice.\n";
            continue;
        }

        g.printRules();

        while (true) {
            std::cout << "\nEnter string to parse ('back' to switch grammar, 'eps' for empty): ";
            std::string input;
            std::cin >> input;
            if (input == "back") break;
            if (input == "eps") input = "";
            g.parseCYK(input, true);
        }
    }
}

// ============================================================================
// MODULE 3: TURING MACHINE SIMULATOR
// ============================================================================

struct TMTransition {
    std::string nextState;
    char writeSymbol;
    char direction; // 'L', 'R', 'S'
};

struct TuringMachine {
    std::string name;
    std::string startState;
    std::string acceptState;
    std::string rejectState;
    char blankSymbol = '_';
    std::map<std::pair<std::string, char>, TMTransition> transitions;

    static void printTape(const std::map<long long, char>& tape, long long head, char blank, int step, const std::string& state) {
        long long minIdx = head, maxIdx = head;
        for (const auto& kv : tape) {
            minIdx = std::min(minIdx, kv.first);
            maxIdx = std::max(maxIdx, kv.first);
        }
        minIdx -= 1;
        maxIdx += 1;

        std::cout << "Step " << std::setw(3) << step << " | State: " << std::left << std::setw(8) << state
                  << std::right << " | ";
        int headOffset = 0, pos = 0;
        for (long long i = minIdx; i <= maxIdx; ++i) {
            char ch = blank;
            auto it = tape.find(i);
            if (it != tape.end()) ch = it->second;
            if (i == head) {
                std::cout << "[" << ch << "]";
                headOffset = pos + 1;
            } else {
                std::cout << " " << ch << " ";
            }
            pos += 3;
        }
        std::cout << "\n" << std::string(26 + headOffset, ' ') << "^\n";
    }

    bool simulate(const std::string& input, bool verbose = true, int maxSteps = 500) const {
        std::map<long long, char> tape;
        for (size_t i = 0; i < input.size(); ++i) {
            tape[static_cast<long long>(i)] = input[i];
        }
        long long head = 0;
        std::string state = startState;
        int step = 0;

        if (verbose) {
            std::cout << "\n--- Turing Machine Trace (" << name << ") ---\n";
            printTape(tape, head, blankSymbol, step, state);
        }

        while (step < maxSteps) {
            if (state == acceptState) {
                if (verbose) std::cout << "Halted in ACCEPT state (" << acceptState << ") in " << step << " steps.\n";
                return true;
            }
            if (state == rejectState) {
                if (verbose) std::cout << "Halted in REJECT state (" << rejectState << ") in " << step << " steps.\n";
                return false;
            }

            char readSym = blankSymbol;
            auto cellIt = tape.find(head);
            if (cellIt != tape.end()) readSym = cellIt->second;

            auto trIt = transitions.find({state, readSym});
            if (trIt == transitions.end()) {
                if (verbose) std::cout << "No transition for (" << state << ", '" << readSym << "') -> REJECT.\n";
                return false;
            }

            const TMTransition& tr = trIt->second;
            tape[head] = tr.writeSymbol;
            state = tr.nextState;
            if (tr.direction == 'L') head--;
            else if (tr.direction == 'R') head++;

            step++;
            if (verbose) printTape(tape, head, blankSymbol, step, state);
        }

        if (verbose) std::cout << "Max step limit (" << maxSteps << ") reached.\n";
        return false;
    }
};

TuringMachine presetTMAnBnCn() {
    TuringMachine tm;
    tm.name = "TM: a^n b^n c^n (n >= 1)";
    tm.startState = "q0";
    tm.acceptState = "q_acc";
    tm.rejectState = "q_rej";
    tm.transitions = {
        {{"q0", 'a'}, {"q1", 'X', 'R'}},
        {{"q0", 'Y'}, {"q3", 'Y', 'R'}},
        {{"q1", 'a'}, {"q1", 'a', 'R'}},
        {{"q1", 'Y'}, {"q1", 'Y', 'R'}},
        {{"q1", 'b'}, {"q2", 'Y', 'R'}},
        {{"q2", 'b'}, {"q2", 'b', 'R'}},
        {{"q2", 'Z'}, {"q2", 'Z', 'R'}},
        {{"q2", 'c'}, {"q4", 'Z', 'L'}},
        {{"q4", 'a'}, {"q4", 'a', 'L'}},
        {{"q4", 'b'}, {"q4", 'b', 'L'}},
        {{"q4", 'Y'}, {"q4", 'Y', 'L'}},
        {{"q4", 'Z'}, {"q4", 'Z', 'L'}},
        {{"q4", 'X'}, {"q0", 'X', 'R'}},
        {{"q3", 'Y'}, {"q3", 'Y', 'R'}},
        {{"q3", 'Z'}, {"q3", 'Z', 'R'}},
        {{"q3", '_'}, {"q_acc", '_', 'S'}}
    };
    return tm;
}

static TuringMachine presetTMAnBn() {
    TuringMachine tm;
    tm.name = "TM: a^n b^n (n >= 0)";
    tm.startState = "q0";
    tm.acceptState = "q_acc";
    tm.rejectState = "q_rej";
    tm.transitions = {
        {{"q0", '_'}, {"q_acc", '_', 'S'}},
        {{"q0", 'a'}, {"q1", 'X', 'R'}},
        {{"q0", 'Y'}, {"q3", 'Y', 'R'}},
        {{"q1", 'a'}, {"q1", 'a', 'R'}},
        {{"q1", 'Y'}, {"q1", 'Y', 'R'}},
        {{"q1", 'b'}, {"q2", 'Y', 'L'}},
        {{"q2", 'a'}, {"q2", 'a', 'L'}},
        {{"q2", 'Y'}, {"q2", 'Y', 'L'}},
        {{"q2", 'X'}, {"q0", 'X', 'R'}},
        {{"q3", 'Y'}, {"q3", 'Y', 'R'}},
        {{"q3", '_'}, {"q_acc", '_', 'S'}}
    };
    return tm;
}

void runTMModule() {
    while (true) {
        std::cout << "\n========================================\n";
        std::cout << "   TURING MACHINE SIMULATOR             \n";
        std::cout << "========================================\n";
        std::cout << "1. Preset TM: a^n b^n c^n (n >= 1)\n";
        std::cout << "0. Back\n";
        std::cout << "Choose option: ";

        int choice;
        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            continue;
        }
        if (choice == 0) break;

        TuringMachine tm;
        if (choice == 1) tm = presetTMAnBnCn();
        else {
            std::cout << "Invalid choice.\n";
            continue;
        }

        while (true) {
            std::cout << "\nEnter initial tape string ('back' to switch TM, 'eps' for blank): ";
            std::string input;
            std::cin >> input;
            if (input == "back") break;
            if (input == "eps") input = "";
            tm.simulate(input, true);
        }
    }
}

// ============================================================================
// MODULE 4: COMPARISON PAGE
// ============================================================================

void runComparisonModule() {
    std::cout << "\n========================================================================\n";
    std::cout << "              CHOMSKY HIERARCHY COMPARISON PAGE                         \n";
    std::cout << "========================================================================\n";

    std::cout << "\n--- Chomsky Hierarchy Summary ---\n";
    std::cout << std::left
              << std::setw(10) << "Type"
              << std::setw(24) << "Language Class"
              << std::setw(26) << "Model"
              << "Example\n";
    std::cout << std::string(76, '-') << "\n";
    std::cout << std::setw(10) << "Type-3" << std::setw(24) << "Regular"
              << std::setw(26) << "DFA / NFA" << "Strings ending in '01'\n";
    std::cout << std::setw(10) << "Type-2" << std::setw(24) << "Context-Free"
              << std::setw(26) << "CFG / PDA" << "a^n b^n\n";
    std::cout << std::setw(10) << "Type-1" << std::setw(24) << "Context-Sensitive"
              << std::setw(26) << "Linear Bounded Automaton" << "a^n b^n c^n\n";
    std::cout << std::setw(10) << "Type-0" << std::setw(24) << "Recursively Enumerable"
              << std::setw(26) << "Turing Machine" << "Computable languages\n";

    std::cout << "\n--- Comparing Models on L = { a^n b^n | n >= 0 } ---\n";
    DFA approxDFA;
    approxDFA.name = "Approx DFA (a*b*)";
    approxDFA.states = {"q0", "q1"};
    approxDFA.alphabet = {'a', 'b'};
    approxDFA.startState = "q0";
    approxDFA.acceptStates = {"q0", "q1"};
    approxDFA.transitions = {
        {{"q0", 'a'}, "q0"}, {{"q0", 'b'}, "q1"}, {{"q1", 'b'}, "q1"}
    };

    CFG cfg = presetCFGAnBn();
    TuringMachine tm = presetTMAnBn();

    std::vector<std::string> tests = {"", "ab", "aabb", "aaabbb", "aab", "abb"};
    std::cout << std::left
              << std::setw(14) << "Input"
              << std::setw(14) << "Expected"
              << std::setw(16) << "DFA (a*b*)"
              << std::setw(16) << "CFG (CYK)"
              << std::setw(16) << "TM" << "\n";
    std::cout << std::string(76, '-') << "\n";

    for (const auto& s : tests) {
        bool exp = (s == "" || s == "ab" || s == "aabb" || s == "aaabbb");
        bool dRes = approxDFA.simulate(s, false);
        bool cRes = cfg.parseCYK(s, false);
        bool tRes = tm.simulate(s, false);

        std::cout << std::left
                  << std::setw(14) << (s.empty() ? "eps" : s)
                  << std::setw(14) << (exp ? "ACCEPT" : "REJECT")
                  << std::setw(16) << (dRes ? "ACCEPT" : "REJECT")
                  << std::setw(16) << (cRes ? "ACCEPT" : "REJECT")
                  << std::setw(16) << (tRes ? "ACCEPT" : "REJECT") << "\n";
    }
}

// ============================================================================
// MAIN ENTRY POINT
// ============================================================================

int main() {
    while (true) {
        std::cout << "\n============================================================\n";
        std::cout << "     CHOMSKY HIERARCHY LANGUAGE RECOGNIZER & SIMULATOR      \n";
        std::cout << "============================================================\n";
        std::cout << "1. DFA / NFA Simulator\n";
        std::cout << "2. CFG Parser\n";
        std::cout << "3. Turing Machine Simulator\n";
        std::cout << "4. Comparison Page\n";
        std::cout << "0. Exit\n";
        std::cout << "Select a module: ";

        int choice;
        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            continue;
        }

        switch (choice) {
            case 1: runFAModule(); break;
            case 2: runCFGModule(); break;
            case 3: runTMModule(); break;
            case 4: runComparisonModule(); break;
            case 0:
                std::cout << "Goodbye!\n";
                return 0;
            default:
                std::cout << "Invalid choice.\n";
                break;
        }
    }
}
