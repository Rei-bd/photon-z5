#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>
#include <chrono>
#include <math.h>

using namespace std;

#pragma pack(1)
struct bmpFH {
	uint16_t bfType;
	uint32_t bfSize;
	uint16_t bfReversed1;
	uint16_t bfReversed2;
	uint32_t bfOffBits;
};

#pragma pack(1)
struct bmpIH {
	uint32_t biSize;
	uint32_t biWidth;
	uint32_t biHeight;
	uint16_t biPlanes;
	uint16_t biBitCount;
	uint32_t biCompression;
	uint32_t biSizeImage;
	uint32_t biXPelsPerMeter;
	uint32_t biYPelsPerMeter;
	uint32_t biClrUsed;
	uint32_t biClrImportant;
};

#pragma pack(1)
struct RGBQuad {
	uint32_t Red = 255;
	uint32_t Green = 255;
	uint32_t Blue = 255;
	uint32_t Alpha = 255;
};

double MO(const vector<vector<RGBQuad>>& pixels, int wi, int he, int channel) {
	double average = 0;
	switch (channel)
	{
	case 1:
		for (int x = 0; x != wi; ++x) {
			for (int y = 0; y != he; ++y) {
				average += pixels[y][x].Red;
			}
		}
		break;
	case 2:
		for (int x = 0; x != wi; ++x) {
			for (int y = 0; y != he; ++y) {
				average += pixels[y][x].Green;
			}
		}
		break;
	case 3:
		for (int x = 0; x != wi; ++x) {
			for (int y = 0; y != he; ++y) {
				average += pixels[y][x].Blue;
			}
		}
		break;
	case 4:
		for (int x = 0; x != wi; ++x) {
			for (int y = 0; y != he; ++y) {
				average += pixels[y][x].Alpha;
			}
		}
		break;
	default:
		break;
	}
	average /= wi * he;
	return average;
}

double CKO(const vector<vector<RGBQuad>>& pixels, int wi, int he, int channel, const vector<double>& E) {
	double S = 0;
	switch (channel)
	{
	case 1:
		for (int x = 0; x != wi; ++x) {
			for (int y = 0; y != he; ++y) {
				S += pow(pixels[y][x].Red - E[0], 2);
			}
		}
		break;
	case 2:
		for (int x = 0; x != wi; ++x) {
			for (int y = 0; y != he; ++y) {
				S += pow(pixels[y][x].Green - E[1], 2);
			}
		}
		break;
	case 3:
		for (int x = 0; x != wi; ++x) {
			for (int y = 0; y != he; ++y) {
				S += pow(pixels[y][x].Blue - E[2], 2);
			}
		}
		break;
	case 4:
		for (int x = 0; x != wi; ++x) {
			for (int y = 0; y != he; ++y) {
				S += pow(pixels[y][x].Alpha - E[3], 2);
			}
		}
		break;
	default:
		break;
	}
	S = sqrt(S / (wi * he));
	return S;
}

int main() {
	string fname;
	getline(cin, fname);
	ifstream bmp(fname + ".bmp", ios::binary);
	bmpFH fileHeader;
	bmpIH infoHeader;

	if (!bmp.is_open()) {
		cout << "Can't open file" << endl;
		return 0;
	}

	bmp.read((char*)&fileHeader, sizeof(fileHeader));
	bmp.read((char*)&infoHeader, sizeof(infoHeader));

	vector<vector<RGBQuad>> pixels(infoHeader.biHeight);
	for (int i = 0; i != infoHeader.biHeight; ++i) {
		vector<RGBQuad> row(infoHeader.biWidth);
		pixels[i] = row;
	}

	bmp.seekg(fileHeader.bfOffBits, ios::beg);
	double offset = infoHeader.biBitCount == 4 ? 0 : ceil(infoHeader.biWidth / 4.0f) * 4 - infoHeader.biWidth;

	for (int y = infoHeader.biHeight - 1; y != -1; --y) {
		for (int x = 0; x != infoHeader.biWidth; ++x) {
			switch (infoHeader.biBitCount / 8)
			{
			case 1:
				bmp.read((char*)&pixels[y][x], 1);
				pixels[y][x].Green = pixels[y][x].Red;
				pixels[y][x].Blue = pixels[y][x].Red;
				pixels[y][x].Alpha = pixels[y][x].Red;
				break;
			case 3:
				bmp.read((char*)&pixels[y][x].Blue, 1);
				bmp.read((char*)&pixels[y][x].Green, 1);
				bmp.read((char*)&pixels[y][x].Red, 1);
				break;
			case 4:
				bmp.read((char*)&pixels[y][x].Blue, 1);
				bmp.read((char*)&pixels[y][x].Green, 1);
				bmp.read((char*)&pixels[y][x].Red, 1);
				bmp.read((char*)&pixels[y][x].Alpha, 1);
				break;
			default:
				break; 
			}
		}
		bmp.seekg(offset, ios::cur);
	}
	vector<double> E;

	cout << endl << "MO: " << endl;
	for (int c = 1; c != infoHeader.biBitCount / 8 + 1; ++c) {
		auto start = chrono::high_resolution_clock::now();
		double M = MO(pixels, infoHeader.biWidth, infoHeader.biHeight, c);
		auto end = chrono::high_resolution_clock::now();
		chrono::duration<double> dur = end - start;
		cout << fixed << setprecision(6) << "Channel " << c << ": " << M << "\tTime: " << dur.count() << endl;
		E.push_back(M);
	}

	cout << endl << "CKO: " << endl;
	for (int c = 1; c != infoHeader.biBitCount / 8 + 1; ++c) {
		auto start = chrono::high_resolution_clock::now();
		double S = CKO(pixels, infoHeader.biWidth, infoHeader.biHeight, c, E);
		auto end = chrono::high_resolution_clock::now();
		chrono::duration<double> dur = end - start;
		cout << fixed << setprecision(6) << "Channel " << c << ": " << S << "\tTime: " << dur.count() << endl;
	}
}