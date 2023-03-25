#include <algorithm>
#include <iostream>
#include <memory>
#include <stack>
#include <string>
#include <utility>

using std::cin;
using std::cout;
using std::endl;

using std::make_unique;
using std::shared_ptr;
using std::stack;
using std::string;

/**
 * @brief NFA 状态结构体
 *
 */
struct NFAState {
  char symbol;
  shared_ptr<NFAState> out1;
  shared_ptr<NFAState> out2;
};

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

public:
  void output_detail() const {
    cout << "symbols: " << symbols << endl;
    cout << "regex: " << regex << endl;
  }

  explicit NFAAlgorithm(string symbols, string &regex)
      : symbols(std::move(symbols)), regex(regex) {
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

  bool is_in_symbols(char cur_char) {
    return symbols.find(cur_char) != std::string::npos;
  }

  /**
   * @brief 添加 . 连接符
   *
   */
  void add_connect_op() {
    string new_regex;
    for (int i = 0; i < regex.size() - 1; i++) {
      auto flag1 = is_in_symbols(regex[i]);
      auto flag2 = is_in_symbols(regex[i + 1]);
      if ((flag1 && regex[i + 1] == '(') || (flag1 && flag2) ||
          (regex[i] == ')' && flag2) || (regex[i] == '*' && flag2)) {
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
        while (!op_stack.empty() &&
               get_op(op_stack.top()) >= get_op(cur_char)) {
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
  void build_nfa() {}
};

int main() {
  string symbols;
  string regex;
  cin >> symbols >> regex;

  auto solver = make_unique<NFAAlgorithm>(symbols, regex);
  solver->output_detail();
}
