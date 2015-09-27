#include <set>
#include <stack>
#include <sstream>
#include <algorithm>

#include "macros_base.h"

void DependencyGraph::clear() {
    this->pos_words.clear();
    this->predicates_index.clear();
    memset(this->graph, 0, sizeof this->graph);
    this->num_words = this->num_arcs = 0;
}

int encodeLinkDistanceOrDirection(const int &hi, const int &di, bool dir) {
    int diff = hi - di;
    if (dir) {
        return diff > 0 ? 7 : -7;
    }
    if (diff < 0) {
        diff = -diff;
    }
    if (diff > 10) {
        diff = 6;
    }
    else if (diff > 5) {
        diff = 5;
    }
    if (hi < di) {
        diff = -diff;
    }
    return diff;
}

// read from SemEval SDP format
std::istream &operator>>(std::istream &input, DependencyGraph &graph) {
    graph.clear();
    std::string line;
    do {
        // skip empty lines
        std::getline(input, line);
    } while (line.length() == 0 && !input.eof());

    if (input.eof()) {
        return input;
    }

    if (line[0] != '#') {
        std::cerr << "Illegal format: " << line << "\nExpecting a comment line(SemEval SDP format)" << std::endl;
        return input;
    }

    graph.pos_words.push_back(POSTaggedWord(ROOT_WORD, ROOT_POSTAG));
    graph.predicates_index.push_back(0);

    while ("words to be read") {
        std::getline(input, line);
        if (line.length() == 0) {
            break;
        }
        graph.num_words++;
        std::istringstream iss(line);
        ttoken word;
        ttoken pos;
        ttoken tmp;
        iss >> tmp >> word >> tmp >> pos;
        graph.pos_words.push_back(POSTaggedWord(word, pos));

        // pointed by root?
        iss >> tmp;
        if (tmp == "+") {
            graph.graph[0][graph.num_words] = 1;
            graph.num_arcs += 1;
        }

        // is a predicate?
        iss >> tmp;
        if (tmp == "+") {
            graph.predicates_index.push_back(graph.num_words);
        }

        int idx = 0;
        while (iss >> tmp) {
            idx++;
            if (tmp != "_") {
                // tmp is the label, but we just ignore it
                graph.graph[idx][graph.num_words] = 1;
                graph.num_arcs += 1;
            }
        }
    }

    // transform the graph in-place. is it cool?
    for (int i = 1; i <= graph.num_words; i++) {
        for (int j = graph.num_words; j > 0; j--) {
            // predicates[j] >= j, so we loop j decreasingly.
            if (graph.graph[j][i]) {
                graph.graph[j][i] = 0;
                graph.graph[graph.predicates_index[j]][i] = 1;
            }
        }
    }

    return input;
}

std::istream &operator>>(std::istream &input, Sentence &sentence) {
    sentence.clear();
    ttoken line, token;
    std::getline(input, line);
    std::istringstream iss(line);
    while (iss >> token) {
        int i = token.rfind(SENT_SPTOKEN);
        sentence.push_back(POSTaggedWord(token.substr(0, i), token.substr(i + 1)));
    }
    return input;
}


std::istream &operator>>(std::istream &input, DependencyTree &tree) {
    tree.clear();
    ttoken line, token;
    while (true) {
        std::getline(input, line);
        if (line.empty()) {
            break;
        }
        DependencyTreeNode node;
        std::istringstream iss(line);
        iss >> token;
        std::get<0>(std::get<0>(node)) = token;
        iss >> token;
        std::get<1>(std::get<0>(node)) = token;
        iss >> std::get<1>(node) >> std::get<2>(node);
        tree.push_back(node);
    }
    return input;
}

std::ostream &operator<<(std::ostream &output, const Sentence &sentence) {
    auto itr = sentence.begin();
    while (true) {
        output << std::get<0>(*itr) << SENT_SPTOKEN << std::get<1>(*itr);
        if (++itr == sentence.end()) {
            break;
        }
        else {
            output << " ";
        }
    }
    output << std::endl;
    return output;
}

std::ostream &operator<<(std::ostream &output, const DependencyTree &tree) {
    for (auto itr = tree.begin(); itr != tree.end(); ++itr) {
        output << std::get<0>(std::get<0>(*itr)) << "\t" << std::get<1>(std::get<0>(*itr)) << "\t" <<
        std::get<1>(*itr) << "\t" << std::get<2>(*itr) << std::endl;
    }
    output << std::endl;
    return output;
}
