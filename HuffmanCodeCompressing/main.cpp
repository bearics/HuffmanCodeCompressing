#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <vector>
#include <algorithm>

#define HEIGHT 256
#define WIDTH 256

using namespace std;

class HuffmanTree;

typedef struct Histogram {
	unsigned char value;
	int size;
};

typedef struct HuffmanCode {
	unsigned char value;
	vector<bool>* code;	// 0 is false, 1 is true
};

class Node {
	friend class HuffmanTree;
public:
	double prob;
	unsigned char value;
	Node* left;
	Node* right;

	Node(double prob = 0, int value = 0, Node* left = NULL, Node* right = NULL)
	{
		this->prob = prob;
		this->value= value;
		this->left = left;
		this->right = right;
	}

};

class HuffmanTree {
public:
	Node* root = new Node(0, 0);
	vector<HuffmanCode> huffman;

	void insert(Node* node)
	{
		Node* right = root->right;
		Node* left = root->left;

		huffman.push_back({ node->value, new vector<bool>() });

		if (root->right == NULL)
		{	// first
			root->right = node;
		}	// second
		else if(root->left == NULL)
		{
			root->left = node;
		}
		else
		{	// insert third node.
			Node* parent = new Node(right->prob + left->prob, 0, left, right);

			if (parent->prob >= node->prob)
			{
				root->left = parent;
				root->right = node;
			}
			else
			{
				root->left = node;
				root->right = parent;
			}
		}
	}

	void makeHuffmanCode(Node* current, vector<bool> code)
	{
		unsigned char value = current->value;
		if (current->left == NULL)
		{
			auto it = find_if(huffman.begin(), huffman.end(),
				[value](const HuffmanCode& h){return h.value == value;});
			*((*it).code) = code;
		}
		else
		{
			code.push_back(true);
			makeHuffmanCode(current->right, code);
		}
		

		if (current->right == NULL)
		{
			auto it = find_if(huffman.begin(), huffman.end(),
				[value](const HuffmanCode& h) {return h.value == value; });
			*((*it).code) = code;
		}
		else
		{
			code.push_back(true);
			makeHuffmanCode(current->left, code);

		}
	}
};

unsigned char** MemAlloc2D(int nHeight, int nWidth)
{
	unsigned char** rtn = new unsigned char*[nHeight];
	for (int h = 0; h < nHeight; h++)
	{
		rtn[h] = new unsigned char[nWidth];
		for (int w = 0; w < nWidth; w++)
		{
			rtn[h][w] = 0;
		}
	}
	return rtn;
}

template<typename T> void MemFree2D(T** arr2D, int nHeight)
{
	for (int h = 0; h < nHeight; h++)
	{
		delete[] arr2D[h];
	}
	delete[] arr2D;
}

void FileRead(const char* strFilename, unsigned char** arr2D, int nHeight, int nWidth)
{
	FILE* fp_in = fopen(strFilename, "rb");
	for (int h = 0; h < nHeight; h++)
	{
		fread(arr2D[h], sizeof(unsigned char), nWidth, fp_in);
	}

	fclose(fp_in);
}

void FileWrite(const char* strFilename, unsigned char** arr2D, int nHeight, int nWidth)
{
	FILE* fp_out = fopen(strFilename, "wb");
	for (int h = 0; h < nHeight; h++)
	{
		fwrite(arr2D[h], sizeof(unsigned char), nWidth, fp_out);
	}

	fclose(fp_out);
}

Histogram* GetHistogram(unsigned char** value)
{	// Make Histogram
	Histogram* hist = new Histogram[256];

	for (int i = 0; i < 256; i++)
	{
		hist[i].value = (unsigned char)i;
		hist[i].size = 0;
	}

	for (int h = 0; h < HEIGHT; h++)
	{
		for (int w = 0; w < WIDTH; w++)
		{
			hist[value[h][w]].size++;
		}
	}	

	return hist;
}

int* GetHuffmanCode(unsigned char** value)
{	// Make Histogram
	int hist[256];

	for (int i = 0; i < 256; i++)
	{
		hist[i] = 0;
	}

	for (int h = 0; h < HEIGHT; h++)
	{
		for (int w = 0; w < WIDTH; w++)
		{
			hist[value[h][w]]++;
		}
	}

	return hist;
}


void main()
{

	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	// File Open & Memory Allocation

	unsigned char** oriImg = MemAlloc2D(HEIGHT, WIDTH);
	unsigned char** whiteArr2D = MemAlloc2D(HEIGHT, WIDTH);
	Histogram* histogram;

	HuffmanTree huf;

	// read lena256.raw and sort histogram.
	FileRead("lena256.raw", oriImg, HEIGHT, WIDTH);
	histogram = GetHistogram(oriImg);
	sort(histogram, histogram + 256, [](const Histogram& h1, const Histogram& h2) {
		if (h1.size != h2.size)
			return h1.size < h2.size;
		return h1.value < h2.value;
	});

	// make huffman tree
	for (int i = 0; i < 256; i++)
	{
		if (histogram[i].size != 0)
		{
			Node* node = new Node((double)histogram[i].size / (double)(HEIGHT * WIDTH), histogram[i].value);
			huf.insert(node);
		}
	}



	FileWrite("white.raw", whiteArr2D, HEIGHT, WIDTH);

	return;
}