#include <algorithm>
#include <fstream>
#include <iostream>
#include <memory>
#include <queue>
#include <stack>
#include <string>
#include <utility>
#include <vector>

using std::endl;
using std::ifstream;
using std::ofstream;

using std::queue;
using std::shared_ptr;
using std::stack;
using std::string;
using std::vector;

/**
 * @brief NFA 类
 *
 */
class NFAAlgorithm {
private:
  string symbols;
  string regex;

  enum class OP {
    BRACKET,
    OR,
    CONNECT,
    STAR,
  };

  struct Edge;
  static constexpr char gDefaultID = 'a';
  char gID = gDefaultID;
  struct Node {
    char id; // Node 编号
    vector<shared_ptr<Edge>> out_edges;
  };

  static constexpr char EPSILON = 'E';
  struct Edge {
    shared_ptr<Node> from;
    shared_ptr<Node> to;
    char symbol;
  };

  struct NFA {
    shared_ptr<Node> start;
    shared_ptr<Node> end;

    NFA(shared_ptr<Node> start, shared_ptr<Node> end) : start(std::move(start)), end(std::move(end)) {}
  };
  shared_ptr<NFA> nfa;

public:
  explicit NFAAlgorithm(string symbols, string &regex) : symbols(std::move(symbols)), regex(regex) {
    add_connect_op();
    re2post();
    build_nfa();
  };

  static OP get_op(char cur_char) {
    switch (cur_char) {
    case '|':
      return OP::OR;
    case '.':
      return OP::CONNECT;
    case '*':
      return OP::STAR;
    case '(':
    case ')':
      return OP::BRACKET;
    default:
      return OP::CONNECT;
    }
  }

  bool is_in_symbols(char cur_char) { return symbols.find(cur_char) != std::string::npos; }

  /** NFA构建用的函数 */

  /**
   * @brief 添加 . 连接符
   *
   */
  void add_connect_op() {
    string new_regex;
    for (int i = 0; i < regex.size() - 1; i++) {
      auto flag1 = is_in_symbols(regex[i]);
      auto flag2 = is_in_symbols(regex[i + 1]);
      if ((flag1 && regex[i + 1] == '(') || (flag1 && flag2) || (regex[i] == ')' && flag2) || (regex[i] == '*' && flag2)) {
        new_regex += regex[i];
        new_regex += '.';
      } else {
        new_regex += regex[i];
      }
    }

    new_regex += regex.back();

    regex = new_regex;
  }

  /**
   * @brief 将正规式转换成后缀式
   *
   */
  void re2post() {
    string post_regex;
    stack<char> op_stack;

    for (auto &cur_char : regex) {
      if (is_in_symbols(cur_char)) {
        post_regex += cur_char;
      } else if (cur_char == '(') {
        op_stack.push(cur_char);
      } else if (cur_char == ')') {
        while (op_stack.top() != '(') {
          post_regex += op_stack.top();
          op_stack.pop();
        }
        op_stack.pop();
      } else {
        while (!op_stack.empty() && get_op(op_stack.top()) >= get_op(cur_char)) {
          post_regex += op_stack.top();
          op_stack.pop();
        }
        op_stack.push(cur_char);
      }
    }
    while (!op_stack.empty()) {
      post_regex += op_stack.top();
      op_stack.pop();
    }

    regex = post_regex;
  }

  /**
   * @brief 构建 NFA
   *
   */
  void build_nfa() {
    stack<shared_ptr<NFA>> nfa_stack;

    for (auto &cur_char : regex) {
      if (cur_char == '.') {
        auto nfa2 = nfa_stack.top();
        nfa_stack.pop();
        auto nfa1 = nfa_stack.top();
        nfa_stack.pop();

        auto edge = std::make_shared<Edge>(Edge{nfa1->end, nfa2->start, EPSILON});

        nfa1->end->out_edges.push_back(edge);

        auto nfa = std::make_shared<NFA>(nfa1->start, nfa2->end);
        nfa_stack.push(nfa);
      } else if (cur_char == '|') {
        auto nfa2 = nfa_stack.top();
        nfa_stack.pop();
        auto nfa1 = nfa_stack.top();
        nfa_stack.pop();

        auto start_node = std::make_shared<Node>();
        auto end_node = std::make_shared<Node>();
        start_node->id = gID++;
        end_node->id = gID++;

        auto edge1 = std::make_shared<Edge>(Edge{start_node, nfa1->start, EPSILON});
        auto edge2 = std::make_shared<Edge>(Edge{start_node, nfa2->start, EPSILON});
        auto edge3 = std::make_shared<Edge>(Edge{nfa1->end, end_node, EPSILON});
        auto edge4 = std::make_shared<Edge>(Edge{nfa2->end, end_node, EPSILON});

        start_node->out_edges.push_back(edge1);
        start_node->out_edges.push_back(edge2);
        nfa1->end->out_edges.push_back(edge3);
        nfa2->end->out_edges.push_back(edge4);

        auto nfa = std::make_shared<NFA>(start_node, end_node);
        nfa_stack.push(nfa);
      } else if (cur_char == '*') {
        auto nfa = nfa_stack.top();
        nfa_stack.pop();

        auto start_node = std::make_shared<Node>();
        auto end_node = std::make_shared<Node>();
        start_node->id = gID++;
        end_node->id = gID++;

        auto edge1 = std::make_shared<Edge>(Edge{start_node, nfa->start, EPSILON});
        auto edge2 = std::make_shared<Edge>(Edge{start_node, end_node, EPSILON});
        auto edge3 = std::make_shared<Edge>(Edge{nfa->end, end_node, EPSILON});
        auto edge4 = std::make_shared<Edge>(Edge{nfa->end, nfa->start, EPSILON});

        start_node->out_edges.push_back(edge1);
        start_node->out_edges.push_back(edge2);
        nfa->end->out_edges.push_back(edge3);
        nfa->end->out_edges.push_back(edge4);

        auto new_nfa = std::make_shared<NFA>(start_node, end_node);
        nfa_stack.push(new_nfa);
      } else {
        auto start_node = std::make_shared<Node>();
        auto end_node = std::make_shared<Node>();
        start_node->id = gID++;
        end_node->id = gID++;

        auto edge = std::make_shared<Edge>(Edge{start_node, end_node, cur_char});

        start_node->out_edges.push_back(edge);

        auto nfa = std::make_shared<NFA>(start_node, end_node);
        nfa_stack.push(nfa);
      }
    }

    nfa = nfa_stack.top();
  }

  void output_nfa() {
    auto output_file = ofstream("output.txt"); // 打开文件

    // 输出状态集
    for (auto state = gDefaultID; state < gID; state++) {
      output_file << state;
    }
    output_file << endl;

    // 输出符号集
    output_file << symbols << endl;

    // 输出初态集
    output_file << nfa->start->id << endl;

    // 输出终态集
    output_file << nfa->end->id << endl;

    // 输出转移函数
    queue<shared_ptr<Node>> node_queue;
    vector<bool> visited(gID, false);

    node_queue.push(nfa->start);
    visited[nfa->start->id] = true;

    while (!node_queue.empty()) {
      auto cur_node = node_queue.front();
      visited[cur_node->id] = true;
      node_queue.pop();

      for (auto &edge : cur_node->out_edges) {
        output_file << "(" << edge->from->id << "," << edge->symbol << "," << edge->to->id << ")" << endl;
        if (!visited[edge->to->id]) {
          visited[edge->to->id] = true;
          node_queue.push(edge->to);
        }
      }
    }
  }
};

int main() {
  auto input_file = ifstream("input.txt");

  string symbols;
  string regex;
  input_file >> symbols >> regex;

  auto solver = std::make_unique<NFAAlgorithm>(symbols, regex);
  solver->output_nfa();
}
