#include <algorithm>
#include <iostream>
#include <limits>
#include <memory>
#include <numeric>
#include <optional>
#include <queue>
#include <random>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <utility>

// Часть 1. Бинарное дерево и способы его обхода.
class BinaryTree {
protected:
    struct Node {
        int key;
        std::unique_ptr<Node> left, right;
        explicit Node(int value) : key(value) {}
    };
    std::unique_ptr<Node> root;

    static void pre(const Node* node, std::vector<int>& result) {
        if (!node) return;
        result.push_back(node->key);
        pre(node->left.get(), result);
        pre(node->right.get(), result);
    }
    static void in(const Node* node, std::vector<int>& result) {
        if (!node) return;
        in(node->left.get(), result);
        result.push_back(node->key);
        in(node->right.get(), result);
    }
    static void post(const Node* node, std::vector<int>& result) {
        if (!node) return;
        post(node->left.get(), result);
        post(node->right.get(), result);
        result.push_back(node->key);
    }
    static std::size_t leaves(const Node* node) {
        if (!node) return 0;
        if (!node->left && !node->right) return 1;
        return leaves(node->left.get()) + leaves(node->right.get());
    }
    static std::size_t nodes(const Node* node) {
        if (!node) return 0;
        return 1 + nodes(node->left.get()) + nodes(node->right.get());
    }
    // Высота измеряется количеством узлов на самом длинном пути.
    static std::size_t depth(const Node* node) {
        if (!node) return 0;
        return 1 + std::max(depth(node->left.get()), depth(node->right.get()));
    }
    static void draw(const Node* node, std::ostream& out,
                     std::size_t level, const char* label) {
        if (!node) return;
        draw(node->right.get(), out, level + 1, "R: ");
        out << std::string(level * 4, ' ') << label << node->key << '\n';
        draw(node->left.get(), out, level + 1, "L: ");
    }

public:
    BinaryTree() = default;
    // unique_ptr автоматически освобождает все дочерние узлы.
    virtual ~BinaryTree() = default;
    BinaryTree(const BinaryTree&) = delete;
    BinaryTree& operator=(const BinaryTree&) = delete;

    // Обычное бинарное дерево можно построить вручную.
    static std::unique_ptr<BinaryTree> sample() {
        auto tree = std::make_unique<BinaryTree>();
        tree->root = std::make_unique<Node>(10);
        auto& r = tree->root;
        r->left = std::make_unique<Node>(5);
        r->right = std::make_unique<Node>(15);
        r->left->left = std::make_unique<Node>(3);
        r->left->right = std::make_unique<Node>(7);
        r->left->right->left = std::make_unique<Node>(6);
        r->right->right = std::make_unique<Node>(20);
        r->right->right->left = std::make_unique<Node>(18);
        return tree;
    }
    std::vector<int> preOrder() const {
        std::vector<int> result;
        pre(root.get(), result);
        return result;
    }
    std::vector<int> inOrder() const {
        std::vector<int> result;
        in(root.get(), result);
        return result;
    }
    std::vector<int> postOrder() const {
        std::vector<int> result;
        post(root.get(), result);
        return result;
    }
    // Обход в ширину с использованием очереди.
    std::vector<int> levelOrder() const {
        std::vector<int> result;
        if (!root) return result;
        std::queue<const Node*> pending;
        pending.push(root.get());
        while (!pending.empty()) {
            const Node* node = pending.front();
            pending.pop();
            result.push_back(node->key);
            if (node->left) pending.push(node->left.get());
            if (node->right) pending.push(node->right.get());
        }
        return result;
    }
    std::size_t countLeaves() const { return leaves(root.get()); }
    std::size_t countNodes() const { return nodes(root.get()); }
    std::size_t height() const { return depth(root.get()); }
    bool empty() const { return !root; }
    void print(std::ostream& out = std::cout) const {
        if (!root) out << "(empty tree)\n";
        else draw(root.get(), out, 0, "Root: ");
    }
};

// Часть 2. Бинарное дерево поиска без повторяющихся ключей.
class BST : public BinaryTree {
    static bool add(std::unique_ptr<Node>& node, int key) {
        if (!node) {
            node = std::make_unique<Node>(key);
            return true;
        }
        if (key < node->key) return add(node->left, key);
        if (key > node->key) return add(node->right, key);
        return false;
    }
    static bool contains(const Node* node, int key) {
        if (!node) return false;
        if (node->key == key) return true;
        return key < node->key ? contains(node->left.get(), key)
                               : contains(node->right.get(), key);
    }
    static const Node* minimum(const Node* node) {
        if (!node) return nullptr;
        while (node->left) node = node->left.get();
        return node;
    }
    static bool erase(std::unique_ptr<Node>& node, int key) {
        if (!node) return false;
        if (key < node->key) return erase(node->left, key);
        if (key > node->key) return erase(node->right, key);
        // Лист или узел с одним потомком: заменяем его потомком.
        if (!node->left || !node->right) {
            auto old = std::move(node);
            node = old->left ? std::move(old->left) : std::move(old->right);
            return true;
        }
        // Два потомка: используем минимальный ключ правого поддерева.
        const int successor = minimum(node->right.get())->key;
        node->key = successor;
        return erase(node->right, successor);
    }
    // Раннее завершение симметричного обхода при нахождении k-го ключа.
    static const Node* kth(const Node* node, std::size_t& k) {
        if (!node) return nullptr;
        if (const Node* found = kth(node->left.get(), k)) return found;
        if (--k == 0) return node;
        return kth(node->right.get(), k);
    }
public:
    bool insert(int key) { return add(root, key); }
    bool search(int key) const { return contains(root.get(), key); }
    bool remove(int key) { return erase(root, key); }
    std::optional<int> findMin() const {
        const Node* node = minimum(root.get());
        if (!node) return std::nullopt;
        return node->key;
    }
    std::optional<int> findMax() const {
        const Node* node = root.get();
        if (!node) return std::nullopt;
        while (node->right) node = node->right.get();
        return node->key;
    }
    std::optional<int> kthSmallest(std::size_t k) const {
        if (k == 0) return std::nullopt;
        const Node* node = kth(root.get(), k);
        if (!node) return std::nullopt;
        return node->key;
    }
};

void printValues(const std::string& label, const std::vector<int>& values) {
    std::cout << label;
    for (int value : values) std::cout << ' ' << value;
    std::cout << '\n';
}

int passed = 0;
void check(const std::string& name, bool result) {
    std::cout << name << ": " << (result ? "PASS" : "FAIL") << '\n';
    if (!result) throw std::runtime_error("Test failed: " + name);
    ++passed;
}

void testBinaryTree() {
    std::cout << "\n=== PART 1: BINARY TREE AND TRAVERSALS ===\n";
    BinaryTree empty;
    check("Empty traversals", empty.preOrder().empty() && empty.inOrder().empty()
          && empty.postOrder().empty() && empty.levelOrder().empty());
    check("Empty characteristics", empty.countLeaves() == 0
          && empty.countNodes() == 0 && empty.height() == 0);
    auto tree = BinaryTree::sample();
    std::cout << "Sample tree (right above, left below):\n";
    tree->print();
    printValues("Pre-order:", tree->preOrder());
    printValues("In-order:", tree->inOrder());
    printValues("Post-order:", tree->postOrder());
    printValues("Level-order:", tree->levelOrder());
    check("Pre-order", tree->preOrder() == std::vector<int>{10,5,3,7,6,15,20,18});
    check("In-order", tree->inOrder() == std::vector<int>{3,5,6,7,10,15,18,20});
    check("Post-order", tree->postOrder() == std::vector<int>{3,6,7,5,18,20,15,10});
    check("Level-order", tree->levelOrder() == std::vector<int>{10,5,15,3,7,20,6,18});
    std::cout << "Leaves: " << tree->countLeaves() << "; nodes: "
              << tree->countNodes() << "; height: " << tree->height() << '\n';
    check("Leaf count", tree->countLeaves() == 3);
    check("Node count", tree->countNodes() == 8);
    check("Height", tree->height() == 4);
    BST single;
    single.insert(9);
    check("Singleton characteristics", single.countLeaves() == 1
          && single.countNodes() == 1 && single.height() == 1);
}

void testBST() {
    std::cout << "\n=== PART 2: BST AND KTH SMALLEST ===\n";
    BST tree;
    check("Empty search and removal", !tree.search(5) && !tree.remove(5));
    check("Empty min/max/kth", !tree.findMin() && !tree.findMax()
          && !tree.kthSmallest(1));
    bool inserted = true;
    for (int value : {50,30,70,20,40,60,80})
        inserted = tree.insert(value) && inserted;
    check("Insertion and size", inserted && tree.countNodes() == 7);
    check("Duplicate rejected", !tree.insert(40) && tree.countNodes() == 7);
    printValues("BST in-order:", tree.inOrder());
    check("BST sorted order", tree.inOrder() == std::vector<int>{20,30,40,50,60,70,80});
    check("Existing and missing keys", tree.search(40) && !tree.search(100));
    check("Min and max", tree.findMin() == 20 && tree.findMax() == 80);
    std::cout << "K = 3: " << tree.kthSmallest(3).value() << '\n';
    check("Kth smallest", tree.kthSmallest(3) == 40);
    check("Kth boundaries", tree.kthSmallest(1) == 20 && tree.kthSmallest(7) == 80);
    check("Invalid K", !tree.kthSmallest(0) && !tree.kthSmallest(8));
    check("Delete leaf", tree.remove(20) && !tree.search(20)
          && tree.inOrder() == std::vector<int>{30,40,50,60,70,80});
    check("Delete one-child node", tree.remove(30) && !tree.search(30)
          && tree.inOrder() == std::vector<int>{40,50,60,70,80});
    check("Delete two-child root", tree.remove(50) && !tree.search(50)
          && tree.inOrder() == std::vector<int>{40,60,70,80}
          && tree.preOrder().front() == 60);
    check("Kth after deletion", tree.kthSmallest(2) == 60);
    check("Missing removal", !tree.remove(999) && tree.countNodes() == 4);
    printValues("After deletions:", tree.inOrder());
    BST one;
    one.insert(1); one.insert(2);
    check("Delete root with one child", one.remove(1) && one.search(2)
          && one.countNodes() == 1);
    check("Delete last root", one.remove(2) && one.empty());
    BST left;
    left.insert(2); left.insert(1);
    check("Delete root with left child", left.remove(2) && left.search(1)
          && left.countNodes() == 1);
    BST limits;
    limits.insert(0);
    limits.insert(std::numeric_limits<int>::min());
    limits.insert(std::numeric_limits<int>::max());
    check("Integer limits", limits.findMin() == std::numeric_limits<int>::min()
          && limits.findMax() == std::numeric_limits<int>::max());
    BST skew;
    for (int i = 1; i <= 10; ++i) skew.insert(i);
    check("Degenerate tree", skew.height() == 10 && skew.countLeaves() == 1
          && skew.kthSmallest(10) == 10);

    // Сравнение случайных операций с эталонным множеством.
    BST randomTree;
    std::set<int> reference;
    std::mt19937 rng(2026);
    std::uniform_int_distribution<int> values(-100, 100);
    bool valid = true;
    for (int i = 0; i < 1000; ++i) {
        int key = values(rng);
        if (i % 3 == 0) valid = (randomTree.remove(key) == (reference.erase(key) != 0)) && valid;
        else if (i % 3 == 1) valid = (randomTree.insert(key) == reference.insert(key).second) && valid;
        else valid = (randomTree.search(key) == (reference.count(key) != 0)) && valid;
        valid = (randomTree.inOrder() == std::vector<int>(reference.begin(), reference.end())) && valid;
    }
    check("Random operations vs reference", valid);
}

// Часть 3. Демонстрация на 20 случайных уникальных ключах.
bool demo() {
    std::cout << "\n=== PART 3: BST DEMONSTRATION (N = 20) ===\n";
    std::vector<int> pool(99);
    std::iota(pool.begin(), pool.end(), 1);
    // Фиксированное начальное значение для повторяемости эксперимента.
    std::mt19937 rng(42);
    std::shuffle(pool.begin(), pool.end(), rng);
    pool.resize(20);
    BST tree;
    for (int key : pool) tree.insert(key);
    printValues("Insertion order:", pool);
    const auto before = tree.inOrder();
    printValues("Sorted values:", before);
    std::cout << "Min: " << tree.findMin().value() << "; max: "
              << tree.findMax().value() << "; height: " << tree.height() << '\n';
    std::cout << "Nodes: " << tree.countNodes() << "; leaves: " << tree.countLeaves() << '\n';
    check("Demo initial tree", before.size() == 20 && std::is_sorted(before.begin(), before.end())
          && std::adjacent_find(before.begin(), before.end()) == before.end());
    std::cout << "Tree before deletion (right above, left below):\n";
    tree.print();
    int key = 0;
    std::string line;
    while (true) {
        std::cout << "Enter a key to delete from the tree: " << std::flush;
        if (!std::getline(std::cin, line)) {
            std::cout << "\nNo input. Interactive demonstration not completed.\n";
            return false;
        }
        std::istringstream input(line);
        char extra;
        if (!(input >> key) || (input >> extra)) {
            std::cout << "Invalid integer. Try again.\n";
            continue;
        }
        if (!tree.search(key)) {
            std::cout << "Key not found. Choose a value from the tree.\n";
            continue;
        }
        break;
    }
    bool removed = tree.remove(key);
    auto expected = before;
    expected.erase(std::find(expected.begin(), expected.end(), key));
    const auto after = tree.inOrder();
    std::cout << "Deleted key: " << key << '\n';
    printValues("Sorted values after deletion:", after);
    std::cout << "Tree after deletion:\n";
    tree.print();
    std::cout << "Nodes: " << tree.countNodes() << "; leaves: " << tree.countLeaves()
              << "; height: " << tree.height() << '\n';
    check("Demo deletion verified", removed && !tree.search(key)
          && after == expected && tree.countNodes() == 19);
    return true;
}

int main() {
    try {
        std::cout << "LAB 9 - YEAR 2 - VARIANT 2\n";
        testBinaryTree();
        testBST();
        if (!demo()) return 1;
        std::cout << "\n=== COMPLEXITY SUMMARY ===\n"
                  << "BST insert/search/remove: O(h), worst O(n)\n"
                  << "Randomly shaped BST: expected O(log n) per operation\n"
                  << "Min/max: O(h); kth smallest: O(h+k), at most O(n)\n"
                  << "Traversals/count/height: O(n); recursive stack O(h)\n"
                  << "Level-order queue: O(w); traversal results: O(n)\n"
                  << "Tree storage: O(n); h = height, w = maximum width\n";
        std::cout << "\nTests passed: " << passed << '\n';
        std::cout << "Program completed successfully.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
