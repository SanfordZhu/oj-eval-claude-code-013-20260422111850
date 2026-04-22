/**
* implement a container like std::map
*/
#ifndef SJTU_MAP_HPP
#define SJTU_MAP_HPP

// only for std::less<T>
#include <functional>
#include <cstddef>
#include "utility.hpp"
#include "exceptions.hpp"

namespace sjtu {

template<
   class Key,
   class T,
   class Compare = std::less <Key>
   > class map {
  public:
   /**
  * the internal type of data.
  * it should have a default constructor, a copy constructor.
  * You can use sjtu::map as value_type by typedef.
    */
   typedef pair<const Key, T> value_type;

  private:
   struct Node {
       value_type *data;
       Node *left, *right, *parent;
       int height;

       Node(const value_type &val, Node *p = nullptr)
           : data(new value_type(val)), left(nullptr), right(nullptr), parent(p), height(1) {}

       ~Node() { delete data; }
   };

   Node *root;
   size_t tree_size;
   Compare comp;

   int getHeight(Node *node) const {
       return node ? node->height : 0;
   }

   void updateHeight(Node *node) {
       if (node) {
           node->height = std::max(getHeight(node->left), getHeight(node->right)) + 1;
       }
   }

   int getBalance(Node *node) const {
       return node ? getHeight(node->left) - getHeight(node->right) : 0;
   }

   Node *rotateRight(Node *y) {
       Node *x = y->left;
       Node *T2 = x->right;

       x->right = y;
       y->left = T2;

       x->parent = y->parent;
       y->parent = x;
       if (T2) T2->parent = y;

       updateHeight(y);
       updateHeight(x);

       return x;
   }

   Node *rotateLeft(Node *x) {
       Node *y = x->right;
       Node *T2 = y->left;

       y->left = x;
       x->right = T2;

       y->parent = x->parent;
       x->parent = y;
       if (T2) T2->parent = x;

       updateHeight(x);
       updateHeight(y);

       return y;
   }

   Node *balance(Node *node) {
       updateHeight(node);
       int balance = getBalance(node);

       if (balance > 1) {
           if (getBalance(node->left) < 0) {
               node->left = rotateLeft(node->left);
           }
           return rotateRight(node);
       }

       if (balance < -1) {
           if (getBalance(node->right) > 0) {
               node->right = rotateRight(node->right);
           }
           return rotateLeft(node);
       }

       return node;
   }

   Node *insert(Node *node, Node *parent, const value_type &val) {
       if (!node) {
           tree_size++;
           return new Node(val, parent);
       }

       if (comp(val.first, node->data->first)) {
           node->left = insert(node->left, node, val);
       } else if (comp(node->data->first, val.first)) {
           node->right = insert(node->right, node, val);
       } else {
           return node;
       }

       return balance(node);
   }

   Node *findMin(Node *node) const {
       while (node && node->left) {
           node = node->left;
       }
       return node;
   }

   Node *findMax(Node *node) const {
       while (node && node->right) {
           node = node->right;
       }
       return node;
   }

   Node *eraseNode(Node *node) {
       tree_size--;

       if (!node->left || !node->right) {
           Node *temp = node->left ? node->left : node->right;
           if (!temp) {
               delete node;
               return nullptr;
           } else {
               // Store the children before copying
               Node *leftChild = temp->left;
               Node *rightChild = temp->right;

               // Update parent pointers for children
               if (leftChild) leftChild->parent = node;
               if (rightChild) rightChild->parent = node;

               // Copy the data and child pointers
               node->data = temp->data;
               node->left = leftChild;
               node->right = rightChild;
               node->height = temp->height;

               // Prevent double deletion
               temp->data = nullptr;
               temp->left = nullptr;
               temp->right = nullptr;
               delete temp;
               return node;
           }
       } else {
           Node *temp = findMin(node->right);
           value_type *oldData = node->data;
           node->data = new value_type(*temp->data);
           delete oldData;
           // Use a helper that doesn't decrement tree_size
           node->right = eraseWithoutDecrement(node->right, temp->data->first);
           return node;
       }
   }

   Node *eraseWithoutDecrement(Node *node, const Key &key) {
       if (!node) return nullptr;

       if (comp(key, node->data->first)) {
           node->left = eraseWithoutDecrement(node->left, key);
       } else if (comp(node->data->first, key)) {
           node->right = eraseWithoutDecrement(node->right, key);
       } else {
           // Found the node to delete - don't decrement here
           if (!node->left || !node->right) {
               Node *temp = node->left ? node->left : node->right;
               if (!temp) {
                   delete node;
                   return nullptr;
               } else {
                   // Store the children before copying
                   Node *leftChild = temp->left;
                   Node *rightChild = temp->right;

                   // Update parent pointers for children
                   if (leftChild) leftChild->parent = node;
                   if (rightChild) rightChild->parent = node;

                   // Copy the data and child pointers
                   node->data = temp->data;
                   node->left = leftChild;
                   node->right = rightChild;
                   node->height = temp->height;

                   // Prevent double deletion
                   temp->data = nullptr;
                   temp->left = nullptr;
                   temp->right = nullptr;
                   delete temp;
                   return balance(node);
               }
           } else {
               Node *temp = findMin(node->right);
               value_type *oldData = node->data;
               node->data = new value_type(*temp->data);
               delete oldData;
               node->right = eraseWithoutDecrement(node->right, temp->data->first);
               return balance(node);
           }
       }

       return balance(node);
   }

   Node *erase(Node *node, const Key &key) {
       if (!node) return nullptr;

       if (comp(key, node->data->first)) {
           node->left = erase(node->left, key);
       } else if (comp(node->data->first, key)) {
           node->right = erase(node->right, key);
       } else {
           // Found the node to delete
           return eraseNode(node);
       }

       return balance(node);
   }

   void clear(Node *node) {
       if (node) {
           clear(node->left);
           clear(node->right);
           delete node;
       }
   }

   Node *copy(Node *node, Node *parent = nullptr) {
       if (!node) return nullptr;
       Node *newNode = new Node(*node->data, parent);
       newNode->left = copy(node->left, newNode);
       newNode->right = copy(node->right, newNode);
       newNode->height = node->height;
       return newNode;
   }

  public:
   /**
  * see BidirectionalIterator at CppReference for help.
  *
  * if there is anything wrong throw invalid_iterator.
  *     like it = map.begin(); --it;
  *       or it = map.end(); ++end();
    */
   class const_iterator;
   class iterator {
      private:
       Node *node;
       const map *container;

       friend class map;

      public:
       iterator() : node(nullptr), container(nullptr) {}

       iterator(Node *n, const map *c) : node(n), container(c) {}

       iterator(const iterator &other) : node(other.node), container(other.container) {}

       /**
    * TODO iter++
        */
       iterator operator++(int) {
           iterator temp = *this;
           ++(*this);
           return temp;
       }

       /**
    * TODO ++iter
        */
       iterator &operator++() {
           if (!node) throw invalid_iterator();

           if (node->right) {
               node = container->findMin(node->right);
           } else {
               Node *parent = node->parent;
               while (parent && node == parent->right) {
                   node = parent;
                   parent = parent->parent;
               }
               node = parent;
           }
           return *this;
       }

       /**
    * TODO iter--
        */
       iterator operator--(int) {
           iterator temp = *this;
           --(*this);
           return temp;
       }

       /**
    * TODO --iter
        */
       iterator &operator--() {
           if (!node) {
               node = container->findMax(container->root);
               return *this;
           }

           if (node->left) {
               node = container->findMax(node->left);
           } else {
               Node *parent = node->parent;
               while (parent && node == parent->left) {
                   node = parent;
                   parent = parent->parent;
               }
               node = parent;
           }
           return *this;
       }

       /**
    * a operator to check whether two iterators are same (pointing to the same memory).
        */
       value_type &operator*() const {
           if (!node) throw invalid_iterator();
           return *node->data;
       }

       bool operator==(const iterator &rhs) const {
           return node == rhs.node;
       }

       bool operator==(const const_iterator &rhs) const;

       /**
    * some other operator for iterator.
        */
       bool operator!=(const iterator &rhs) const {
           return !(*this == rhs);
       }

       bool operator!=(const const_iterator &rhs) const;

       /**
    * for the support of it->first.
    * See <http://kelvinh.github.io/blog/2013/11/20/overloading-of-member-access-operator-dash-greater-than-symbol-in-cpp/> for help.
        */
       value_type *operator->() const
           noexcept {
           return &(*node->data);
       }
   };

   class const_iterator {
       Node *node;
       const map *container;

       friend class map;

   public:
       const_iterator() : node(nullptr), container(nullptr) {}

       const_iterator(Node *n, const map *c) : node(n), container(c) {}

       const_iterator(const const_iterator &other) : node(other.node), container(other.container) {}

       const_iterator(const iterator &other) : node(other.node), container(other.container) {}

       const_iterator operator++(int) {
           const_iterator temp = *this;
           ++(*this);
           return temp;
       }

       const_iterator &operator++() {
           if (!node) throw invalid_iterator();

           if (node->right) {
               node = container->findMin(node->right);
           } else {
               Node *parent = node->parent;
               while (parent && node == parent->right) {
                   node = parent;
                   parent = parent->parent;
               }
               node = parent;
           }
           return *this;
       }

       const_iterator operator--(int) {
           const_iterator temp = *this;
           --(*this);
           return temp;
       }

       const_iterator &operator--() {
           if (!node) {
               node = container->findMax(container->root);
               return *this;
           }

           if (node->left) {
               node = container->findMax(node->left);
           } else {
               Node *parent = node->parent;
               while (parent && node == parent->left) {
                   node = parent;
                   parent = parent->parent;
               }
               node = parent;
           }
           return *this;
       }

       const value_type &operator*() const {
           if (!node) throw invalid_iterator();
           return *node->data;
       }

       bool operator==(const const_iterator &rhs) const {
           return node == rhs.node;
       }

       bool operator!=(const const_iterator &rhs) const {
           return !(*this == rhs);
       }

       const value_type *operator->() const noexcept {
           return &(*node->data);
       }
   };

   /**
  * TODO two constructors
    */
   map() : root(nullptr), tree_size(0) {}

   map(const map &other) : root(copy(other.root)), tree_size(other.tree_size), comp(other.comp) {}

   /**
  * TODO assignment operator
    */
   map &operator=(const map &other) {
       if (this != &other) {
           clear();
           root = copy(other.root);
           tree_size = other.tree_size;
           comp = other.comp;
       }
       return *this;
   }

   /**
  * TODO Destructors
    */
   ~map() {
       clear(root);
   }

   /**
  * TODO
  * access specified element with bounds checking
  * Returns a reference to the mapped value of the element with key equivalent to key.
  * If no such element exists, an exception of type `index_out_of_bound'
    */
   T &at(const Key &key) {
       Node *node = root;
       while (node) {
           if (comp(key, node->data->first)) {
               node = node->left;
           } else if (comp(node->data->first, key)) {
               node = node->right;
           } else {
               return node->data->second;
           }
       }
       throw index_out_of_bound();
   }

   const T &at(const Key &key) const {
       Node *node = root;
       while (node) {
           if (comp(key, node->data->first)) {
               node = node->left;
           } else if (comp(node->data->first, key)) {
               node = node->right;
           } else {
               return node->data->second;
           }
       }
       throw index_out_of_bound();
   }

   /**
  * TODO
  * access specified element
  * Returns a reference to the value that is mapped to a key equivalent to key,
  *   performing an insertion if such key does not already exist.
    */
   T &operator[](const Key &key) {
       Node *node = root;

       while (node) {
           if (comp(key, node->data->first)) {
               node = node->left;
           } else if (comp(node->data->first, key)) {
               node = node->right;
           } else {
               return node->data->second;
           }
       }

       value_type val(key, T());
       root = insert(root, nullptr, val);

       node = root;
       while (node) {
           if (comp(key, node->data->first)) {
               node = node->left;
           } else if (comp(node->data->first, key)) {
               node = node->right;
           } else {
               return node->data->second;
           }
       }

       throw runtime_error();
   }

   /**
  * behave like at() throw index_out_of_bound if such key does not exist.
    */
   const T &operator[](const Key &key) const {
       return at(key);
   }

   /**
  * return a iterator to the beginning
    */
   iterator begin() {
       return iterator(findMin(root), this);
   }

   const_iterator cbegin() const {
       return const_iterator(findMin(root), this);
   }

   /**
  * return a iterator to the end
  * in fact, it returns past-the-end.
    */
   iterator end() {
       return iterator(nullptr, this);
   }

   const_iterator cend() const {
       return const_iterator(nullptr, this);
   }

   /**
  * checks whether the container is empty
  * return true if empty, otherwise false.
    */
   bool empty() const {
       return tree_size == 0;
   }

   /**
  * returns the number of elements.
    */
   size_t size() const {
       return tree_size;
   }

   /**
  * clears the contents
    */
   void clear() {
       clear(root);
       root = nullptr;
       tree_size = 0;
   }

   /**
  * insert an element.
  * return a pair, the first of the pair is
  *   the iterator to the new element (or the element that prevented the insertion),
  *   the second one is true if insert successfully, or false.
    */
   pair<iterator, bool> insert(const value_type &value) {
       Node *node = root;

       while (node) {
           if (comp(value.first, node->data->first)) {
               node = node->left;
           } else if (comp(node->data->first, value.first)) {
               node = node->right;
           } else {
               return pair<iterator, bool>(iterator(node, this), false);
           }
       }

       size_t old_size = tree_size;
       root = insert(root, nullptr, value);

       node = root;
       while (node) {
           if (comp(value.first, node->data->first)) {
               node = node->left;
           } else if (comp(node->data->first, value.first)) {
               node = node->right;
           } else {
               return pair<iterator, bool>(iterator(node, this), old_size != tree_size);
           }
       }

       throw runtime_error();
   }

   /**
  * erase the element at pos.
  *
  * throw if pos pointed to a bad element (pos == this->end() || pos points an element out of this)
    */
   void erase(iterator pos) {
       if (pos.node == nullptr || pos.container != this) {
           throw invalid_iterator();
       }
       root = erase(root, pos.node->data->first);
   }

   /**
  * Returns the number of elements with key
  *   that compares equivalent to the specified argument,
  *   which is either 1 or 0
  *     since this container does not allow duplicates.
  * The default method of check the equivalence is !(a < b || b > a)
    */
   size_t count(const Key &key) const {
       Node *node = root;
       while (node) {
           if (comp(key, node->data->first)) {
               node = node->left;
           } else if (comp(node->data->first, key)) {
               node = node->right;
           } else {
               return 1;
           }
       }
       return 0;
   }

   /**
  * Finds an element with key equivalent to key.
  * key value of the element to search for.
  * Iterator to an element with key equivalent to key.
  *   If no such element is found, past-the-end (see end()) iterator is returned.
    */
   iterator find(const Key &key) {
       Node *node = root;
       while (node) {
           if (comp(key, node->data->first)) {
               node = node->left;
           } else if (comp(node->data->first, key)) {
               node = node->right;
           } else {
               return iterator(node, this);
           }
       }
       return end();
   }

   const_iterator find(const Key &key) const {
       Node *node = root;
       while (node) {
           if (comp(key, node->data->first)) {
               node = node->left;
           } else if (comp(node->data->first, key)) {
               node = node->right;
           } else {
               return const_iterator(node, this);
           }
       }
       return cend();
   }
};

template<class Key, class T, class Compare>
bool map<Key, T, Compare>::iterator::operator==(const const_iterator &rhs) const {
   return node == rhs.node;
}

template<class Key, class T, class Compare>
bool map<Key, T, Compare>::iterator::operator!=(const const_iterator &rhs) const {
   return !(*this == rhs);
}

}

#endif