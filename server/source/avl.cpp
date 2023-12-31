#include "../include/avl.hpp"

int AVLTree::height(Node *node)
{
	if (node == NULL)
	{
		return 0;
	}
	return node->height;
}

int AVLTree::balance(Node *node)
{
	if (node == NULL)
	{
		return 0;
	}
	return height(node->left) - height(node->right);
}

Node *AVLTree::rotateRight(Node *node)
{
	Node *left = node->left;
	Node *leftRight = left->right;

	left->right = node;
	node->left = leftRight;

	node->height = std::max(height(node->left), height(node->right)) + 1;
	left->height = std::max(height(left->left), height(left->right)) + 1;

	if (node == root)
	{
		root = left; // Update the root if it changed
	}

	return left;
}

Node *AVLTree::rotateLeft(Node *node)
{
	Node *right = node->right;
	Node *rightLeft = right->left;

	right->left = node;
	node->right = rightLeft;

	node->height = std::max(height(node->left), height(node->right)) + 1;
	right->height = std::max(height(right->left), height(right->right)) + 1;

	if (node == root)
	{
		root = right; // Update the root if it changed
	}

	return right;
}

Node *AVLTree::insert(Node *node, int key, std::string value)
{
	if (root == NULL)
	{
		Node *node = new Node(key, value);
		this->root = node;
		size++;
		return node;
	}
	if (node == NULL)
	{
		size++;
		return new Node(key, value);
	}
	if (key < node->key)
	{
		node->left = insert(node->left, key, value);
	}
	else if (key > node->key)
	{
		node->right = insert(node->right, key, value);
	}
	else
	{
		size++;
		return node;
	}
	node->height = std::max(height(node->left), height(node->right)) + 1;
	int balanceFactor = balance(node);
	if (balanceFactor > 1 && key < node->left->key)
	{
		return rotateRight(node);
	}
	if (balanceFactor < -1 && key > node->right->key)
	{
		return rotateLeft(node);
	}
	if (balanceFactor > 1 && key > node->left->key)
	{
		node->left = rotateLeft(node->left);
		return rotateRight(node);
	}
	if (balanceFactor < -1 && key < node->right->key)
	{
		node->right = rotateRight(node->right);
		return rotateLeft(node);
	}
	return node;
}

Node *AVLTree::remove(Node *node, int key)
{
	if (node == NULL)
	{
		size--;
		return node;
	}
	if (key < node->key)
	{
		node->left = remove(node->left, key);
	}
	else if (key > node->key)
	{
		node->right = remove(node->right, key);
	}
	else
	{
		size--;
		if (node->left == NULL || node->right == NULL)
		{
			Node *temp = node->left ? node->left : node->right;
			if (temp == NULL)
			{
				temp = node;
				node = NULL;
			}
			else
			{
				*node = *temp;
			}
			delete temp;
		}
		else
		{
			Node *temp = min(node->right);
			node->key = temp->key;
			node->right = remove(node->right, temp->key);
		}
	}
	if (node == NULL)
	{
		return node;
	}
	node->height = std::max(height(node->left), height(node->right)) + 1;
	int balanceFactor = balance(node);
	if (balanceFactor > 1 && balance(node->left) >= 0)
	{
		return rotateRight(node);
	}
	if (balanceFactor > 1 && balance(node->left) < 0)
	{
		node->left = rotateLeft(node->left);
		return rotateRight(node);
	}
	if (balanceFactor < -1 && balance(node->right) <= 0)
	{
		return rotateLeft(node);
	}
	if (balanceFactor < -1 && balance(node->right) > 0)
	{
		node->right = rotateRight(node->right);
		return rotateLeft(node);
	}
	
	return node;
}

Node *AVLTree::min(Node *node)
{
	Node *current = node;
	while (current->left != NULL)
	{
		current = current->left;
	}
	return current;
}

Node *AVLTree::max(Node *node)
{
	Node *current = node;
	while (current->right != NULL)
	{
		current = current->right;
	}
	return current;
}

Node *AVLTree::search(Node *node, int key)
{
	if (node == NULL || node->key == key)
	{
		return node;
	}
	if (key < node->key)
	{
		return search(node->left, key);
	}
	return search(node->right, key);
}

void printTreeStructure(Node *node, int level)
{
	// NOTE: PRINTS TREE HORIZOBNTALLY

	if (node == NULL)
	{
		return;
	}
	printTreeStructure(node->right, level + 1);
	for (int i = 0; i < level; i++)
	{
		std::cout << "    ";
	}
	std::cout << node->key << std::endl;
	printTreeStructure(node->left, level + 1);
}

bool AVLTree::contains(Node *node, int key)
{
	if (search(node, key) == NULL)
	{
		return false;
	}
	else
		return true;
}

void AVLTree::deleteTree(Node *node)
{
	if (node == NULL)
	{
		return;
	}
	deleteTree(node->left);
	deleteTree(node->right);
	delete node;
	
}

// int main()
// {
// 	AVLTree *tree = new AVLTree();
// 	tree->root = tree->insert(tree->root, 13, "amr");
// 	printTreeStructure(tree->root, 0);
// 	std::cout << std::endl;
// 	std::cout << std::endl;
// 	tree->root = tree->insert(tree->root, 9, "omar");
// 	printTreeStructure(tree->root, 0);
// 	std::cout << std::endl;
// 	std::cout << std::endl;
// 	std::cout << tree->contains(tree->root, -10);
// 	tree->root = tree->insert(tree->root, 11, "zoz");
// 	printTreeStructure(tree->root, 0);
//  std::cout << std::endl;
//  std::cout << std::endl;
// tree->root = tree->insert(tree->root, 40);
//  printTreeStructure(tree->root, 0);
//  std::cout << std::endl;
//  std::cout << std::endl;
// tree->root = tree->insert(tree->root, 50);
//  printTreeStructure(tree->root, 0);
//  std::cout << std::endl;
//  std::cout << std::endl;
// tree->root = tree->insert(tree->root, 25);

// printTreeStructure(tree->root, 0);
//  std::cout << std::endl;
//  std::cout << std::endl;

// tree->root = tree->remove(tree->root, 30);
// printTreeStructure(tree->root, 0);
//  std::cout << std::endl;
//  std::cout << std::endl;
// tree->root = tree->remove(tree->root, 40);
// tree->root = tree->remove(tree->root, 50);
// tree->root = tree->remove(tree->root, 25);
// tree->root = tree->remove(tree->root, 20);
// tree->root = tree->remove(tree->root, 10);

// 	return 0;
// }
