#define _CRT_SECURE_NO_WARNINGS

#include <iterator>
#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <bitset>

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

typedef struct MapTable {
	unsigned char value;
	string code;
};

class Node {
	friend class HuffmanTree;
public:
	double prob;
	unsigned char value;
	Node* left;
	Node* right;

	Node(const Node& node)
	{
		this->prob = node.prob;
		this->value = node.value;
		this->left = node.left;
		this->right = node.right;
	}

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
	vector<Node> tree;
	vector<HuffmanCode> huffman;

	void insert(Node* node)
	{
		tree.push_back(*node);
		huffman.push_back({ node->value, new vector<bool>() });
	}

	void makeTree()
	{
		while (tree.size() > 1)
		{
			sort( tree.begin(), tree.end(), [](const Node& n1, const Node& n2) {
				return n1.prob > n2.prob;
			});
			Node* n1 = new Node(tree[tree.size() - 1]);	// smallest node
			tree.pop_back();
			Node* n2 = new Node(tree[tree.size() - 1]);	// second smallest node
			tree.pop_back();
			Node* parent = new Node(n1->prob + n2->prob, 0, n1, n2);
			tree.push_back(*parent);
		}
	}

	void makeHuffmanCode(Node* current, vector<bool> code, bool firstTime)
	{
		if (firstTime)
			code.pop_back();

		unsigned char value = current->value;
		if (current->left == NULL && current->right == NULL)
		{
			auto it = find_if(huffman.begin(), huffman.end(),
				[value](const HuffmanCode& h){return h.value == value;});
			*((*it).code) = code;
		}
		else
		{
			// search left tree
			code.push_back(false);
			makeHuffmanCode(current->left, code, false);
			code.pop_back();
			// search right tree
			code.push_back(true);
			makeHuffmanCode(current->right, code, false);
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
	unsigned char** outImg = MemAlloc2D(HEIGHT, WIDTH);
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
	huf.makeTree();

	// make huffman code
	huf.makeHuffmanCode(&huf.tree[0], { true }, true);

	double sum = 0;
	for (int i = 0; i < huf.huffman.size(); i++)
	{
		int size = 0;
		for (int n = 0; n < 256; n++)
		{
			if (histogram[n].value == huf.huffman[i].value)
			{
				size = histogram[n].size;
				break;
			}
		}
		sum += (size / (double)(HEIGHT * WIDTH)) * (double)(huf.huffman[i].code->size());
	}
	cout << sum << endl;

	// write mapping table.
	ofstream outFile("mapping_table.txt");
	for (int i = 0; i < huf.huffman.size(); i++)
	{
		outFile << (int)(huf.huffman[i].value) << endl;
		for (int n = 0; n < huf.huffman[i].code->size(); n++)
		{
			if ((*(huf.huffman[i].code))[n])
				outFile << "1";
			else
				outFile << "0";
		}
		outFile << endl;
	}
	outFile.close();

	// save compressed image
	string saveData = "";
	for (int y = 0; y < HEIGHT; y++)
	{
		for (int x = 0; x < WIDTH; x++)
		{
			unsigned char value = oriImg[y][x];

			auto it = find_if(huf.huffman.begin(), huf.huffman.end(),
				[value](const HuffmanCode& h) {return h.value == value; });
			for (int i = 0; i < ((*it).code)->size(); i++)
			{
				if ((*((*it).code))[i])
					saveData += "1";
				else
					saveData += "0";
			}
			saveData += "";
		}
	}

	ofstream saveFile("compressedImage.hf", ios::binary);
	unsigned char c = 0;

	for (int i = 0; i < saveData.length(); i++) // saveData.length()
	{
		c <<= 1;
		c += ((int)(saveData[i]) - 48);
		//std::bitset<8> x(c);
		//std::cout << x << endl;
		if (i % 8 == 7)
		{
			saveFile << c;
			c = 0;
		}
		if (i == saveData.length()-1)
		{
			c <<= 8 - (saveData.length() % 8);
			saveFile << c;
		}
	}
	saveFile.close();

	// Read Map table
	vector<MapTable> mapTable;
	char inputString[100];
	ifstream inFile("mapping_table.txt");
	while (!inFile.eof())
	{
		MapTable map;
		inFile.getline(inputString, 100);
		map.value = atoi(inputString);
		inFile.getline(inputString, 100);
		map.code = inputString;
		mapTable.push_back(map);
	}

	// decode
	unsigned char* readData;
	FILE *compressedImage;
	compressedImage = fopen("compressedImage.hf", "rb");
	fseek(compressedImage, 0, SEEK_END);
	int fileSize = ftell(compressedImage);
	fseek(compressedImage, 0, SEEK_SET);
	readData = new unsigned char[fileSize];
	fread(readData, sizeof(unsigned char), fileSize, compressedImage);
	fclose(compressedImage);

	string readDataString = "";
	for (int i = 0; i < fileSize; i++)
	{
		for (int n = 0; n < 8; n++)
		{
			if (readData[i] & (int)(pow(2, 7 - n)))	// if 1, result is true
				readDataString += "1";
			else
				readDataString += "0";
		}
	}

	string temp = "";
	int count = 0;
	for (int i = 0; i < readDataString.length(); i++)
	{
		temp += readDataString[i];
		auto it = find_if(mapTable.begin(), mapTable.end(),
			[temp](const MapTable& m) {return m.code == temp; });
		if (it != mapTable.end() )
		{
			outImg[count / HEIGHT][count % WIDTH] = (*it).value;
			temp = "";
			count++;
		}
	}

	FileWrite("result.raw", outImg, HEIGHT, WIDTH);

	return;
}