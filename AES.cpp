#include <iostream>
#include <fstream>
#include <string>

using namespace std;

typedef unsigned char byte;
const int blockSize = 16;
// block size in 32-bit words.  Always 4 for AES.  (128 bits).
#define Nb 4

byte Sbox[16][16] = {  // populate the Sbox matrix
		    /* 0     1     2     3     4     5     6     7     8     9     a     b     c     d     e     f */
	/*0*/  {0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76},
	/*1*/  {0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0},
	/*2*/  {0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15},
	/*3*/  {0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75},
	/*4*/  {0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84},
	/*5*/  {0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf},
	/*6*/  {0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8},
	/*7*/  {0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2},
	/*8*/  {0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73},
	/*9*/  {0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb},
	/*a*/  {0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79},
	/*b*/  {0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08},
	/*c*/  {0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a},
	/*d*/  {0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e},
	/*e*/  {0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf},
	/*f*/  {0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16} };
byte iSbox[16][16]= {  // populate the iSbox matrix
			/* 0     1     2     3     4     5     6     7     8     9     a     b     c     d     e     f */
	/*0*/  {0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb},
	/*1*/  {0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb},
	/*2*/  {0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e},
	/*3*/  {0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25},
	/*4*/  {0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92},
	/*5*/  {0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84},
	/*6*/  {0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06},
	/*7*/  {0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b},
	/*8*/  {0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73},
	/*9*/  {0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e},
	/*a*/  {0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b},
	/*b*/  {0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4},
	/*c*/  {0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f},
	/*d*/  {0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef},
	/*e*/  {0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61},
	/*f*/  {0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d} };
	
byte Rcon[11][4] ={ {0x00, 0x00, 0x00, 0x00},
		{0x01, 0x00, 0x00, 0x00},
		{0x02, 0x00, 0x00, 0x00},
		{0x04, 0x00, 0x00, 0x00},
		{0x08, 0x00, 0x00, 0x00},
		{0x10, 0x00, 0x00, 0x00},
		{0x20, 0x00, 0x00, 0x00},
		{0x40, 0x00, 0x00, 0x00},
		{0x80, 0x00, 0x00, 0x00},
		{0x1b, 0x00, 0x00, 0x00},
		{0x36, 0x00, 0x00, 0x00} };

struct UsfulStuff
{
	int Nk;         // key size in 32-bit words.  4, 6, 8.  (128, 192, 256 bits).
	int Nr;         // number of rounds. 10, 12, 14.
	byte **w;		// key schedule array. 
};

void RotWord(byte *word) {
	byte word0 = word[0];
	word[0] = word[1];
	word[1] = word[2];
	word[2] = word[3];
	word[3] = word0;
}

void SubWord(byte *word)
{
	word[0] = Sbox[word[0] >> 4][word[0] & 0x0f];
	word[1] = Sbox[word[1] >> 4][word[1] & 0x0f];
	word[2] = Sbox[word[2] >> 4][word[2] & 0x0f];
	word[3] = Sbox[word[3] >> 4][word[3] & 0x0f];
}

UsfulStuff* KeyExpansion(byte *key, int keySize) {
	UsfulStuff *stuff = new UsfulStuff();
	switch(keySize){
	case 128:
		stuff->Nk = 4;
		stuff->Nr = 10;
		break;
	case 192:
		stuff->Nk = 6;
		stuff->Nr = 12;
		break;
	case 256:
		stuff->Nk = 8;
		stuff->Nr = 14;
		break;
	default:
		delete stuff;
		return NULL;
	}

	stuff->w = new byte*[Nb * (stuff->Nr + 1)];  // 4 columns of bytes corresponds to a word
	for (int i = 0; i < Nb * (stuff->Nr + 1); i++) {
		stuff->w[i] = new byte[4];
	}

	for (int row = 0; row < stuff->Nk; ++row)
	{
		stuff->w[row][0] = key[4 * row];
		stuff->w[row][1] = key[4 * row + 1];
		stuff->w[row][2] = key[4 * row + 2];
		stuff->w[row][3] = key[4 * row + 3];
	}

	byte temp[4];

	for (int row = stuff->Nk; row < Nb * (stuff->Nr + 1); ++row)
	{
		temp[0] = stuff->w[row - 1][0]; temp[1] = stuff->w[row - 1][1];
		temp[2] = stuff->w[row - 1][2]; temp[3] = stuff->w[row - 1][3];

		if (row % stuff->Nk == 0)
		{
			RotWord(temp);
			SubWord(temp);

			temp[0] = (byte)((int)temp[0] ^ (int)Rcon[row / stuff->Nk][0]);
			temp[1] = (byte)((int)temp[1] ^ (int)Rcon[row / stuff->Nk][1]);
			temp[2] = (byte)((int)temp[2] ^ (int)Rcon[row / stuff->Nk][2]);
			temp[3] = (byte)((int)temp[3] ^ (int)Rcon[row / stuff->Nk][3]);
		}
		else if (stuff->Nk > 6 && (row % stuff->Nk == 4))
		{
			SubWord(temp);
		}

		// w[row] = w[row-Nk] xor temp
		stuff->w[row][0] = (byte)((int)stuff->w[row - stuff->Nk][0] ^ (int)temp[0]);
		stuff->w[row][1] = (byte)((int)stuff->w[row - stuff->Nk][1] ^ (int)temp[1]);
		stuff->w[row][2] = (byte)((int)stuff->w[row - stuff->Nk][2] ^ (int)temp[2]);
		stuff->w[row][3] = (byte)((int)stuff->w[row - stuff->Nk][3] ^ (int)temp[3]);

	}  // for loop
	return stuff;
}

void ReleaseUsfulStuff(UsfulStuff *stuff) {
	for (int i = 0; i < Nb * (stuff->Nr + 1); i++) {
		delete stuff->w[i];
	}
	delete stuff->w;
	delete stuff;
}

void AddRoundKey(byte state[4][Nb], UsfulStuff &stuff, int round)
{

	for (int r = 0; r < 4; ++r)
	{
		for (int c = 0; c < 4; ++c)
		{
			state[r][c] = (byte)((int)state[r][c] ^ (int)stuff.w[(round * 4) + c][r]);
		}
	}
}  // AddRoundKey()

void SubBytes(byte state[4][Nb])
{
	for (int r = 0; r < 4; ++r)
	{
		for (int c = 0; c < 4; ++c)
		{
			state[r][c] = Sbox[(state[r][c] >> 4)][(state[r][c] & 0x0f)];
		}
	}
}  // SubBytes

void ShiftRows(byte state[4][Nb])
{
	byte temp[4][4];

	for (int r = 0; r < 4; ++r)  // copy State into temp[]
	{
		for (int c = 0; c < 4; ++c)
		{
			temp[r][c] = state[r][c];
		}
	}

	for (int r = 1; r < 4; ++r)  // shift temp into State
	{
		for (int c = 0; c < 4; ++c)
		{
			state[r][c] = temp[r][(c + r) % Nb];
		}
	}
}  // ShiftRows()

byte gfmultby01(byte b)
{
	return b;
}

byte gfmultby02(byte b)
{
	if (b < 0x80)
		return (byte)(int)(b << 1);
	else
		return (byte)((int)(b << 1) ^ (int)(0x1b));
}

byte gfmultby03(byte b)
{
	return (byte)((int)gfmultby02(b) ^ (int)b);
}

byte gfmultby09(byte b)
{
	return (byte)((int)gfmultby02(gfmultby02(gfmultby02(b))) ^
		(int)b);
}

byte gfmultby0b(byte b)
{
	return (byte)((int)gfmultby02(gfmultby02(gfmultby02(b))) ^
		(int)gfmultby02(b) ^
		(int)b);
}

byte gfmultby0d(byte b)
{
	return (byte)((int)gfmultby02(gfmultby02(gfmultby02(b))) ^
		(int)gfmultby02(gfmultby02(b)) ^
		(int)(b));
}

byte gfmultby0e(byte b)
{
	return (byte)((int)gfmultby02(gfmultby02(gfmultby02(b))) ^
		(int)gfmultby02(gfmultby02(b)) ^
		(int)gfmultby02(b));
}

void MixColumns(byte state[4][Nb])
{
	byte temp[4][4];

	for (int r = 0; r < 4; ++r)  // copy State into temp[]
	{
		for (int c = 0; c < 4; ++c)
		{
			temp[r][c] = state[r][c];
		}
	}

	for (int c = 0; c < 4; ++c)
	{
		state[0][c] = (byte)((int)gfmultby02(temp[0][c]) ^ (int)gfmultby03(temp[1][c]) ^
			(int)gfmultby01(temp[2][c]) ^ (int)gfmultby01(temp[3][c]));
		state[1][c] = (byte)((int)gfmultby01(temp[0][c]) ^ (int)gfmultby02(temp[1][c]) ^
			(int)gfmultby03(temp[2][c]) ^ (int)gfmultby01(temp[3][c]));
		state[2][c] = (byte)((int)gfmultby01(temp[0][c]) ^ (int)gfmultby01(temp[1][c]) ^
			(int)gfmultby02(temp[2][c]) ^ (int)gfmultby03(temp[3][c]));
		state[3][c] = (byte)((int)gfmultby03(temp[0][c]) ^ (int)gfmultby01(temp[1][c]) ^
			(int)gfmultby01(temp[2][c]) ^ (int)gfmultby02(temp[3][c]));
	}
}  // MixColumns



void Cipher(byte *input, byte *output, UsfulStuff &stuff)  // encipher 16-bit input
{

	// state = input
	byte state[4][Nb];  // always [4,4]
	for (int i = 0; i < (4 * Nb); ++i)
	{
		state[i % 4][i / 4] = input[i];
	}

	AddRoundKey(state, stuff, 0);

	for (int round = 1; round <= (stuff.Nr - 1); ++round)  // main round loop
	{
		SubBytes(state);
		ShiftRows(state);
		MixColumns(state);
		AddRoundKey(state, stuff, round);
	}  // main round loop

	SubBytes(state);
	ShiftRows(state);
	AddRoundKey(state, stuff, stuff.Nr);

	// output = state
	for (int i = 0; i < (4 * Nb); ++i)
	{
		output[i] = state[i % 4][i / 4];
	}

}  // Cipher()

void InvShiftRows(byte state[4][Nb])
{
	byte temp[4][Nb];

	for (int r = 0; r < 4; ++r)  // copy State into temp[]
	{
		for (int c = 0; c < 4; ++c)
		{
			temp[r][c] = state[r][c];
		}
	}


	for (int r = 1; r < 4; ++r)  // shift temp into State
	{
		for (int c = 0; c < 4; ++c)
		{
			state[r][(c + r) % Nb] = temp[r][c];
		}
	}
}  // InvShiftRows()

void InvSubBytes(byte state[4][Nb])
{
	for (int r = 0; r < 4; ++r)
	{
		for (int c = 0; c < 4; ++c)
		{
			state[r][c] = iSbox[(state[r][c] >> 4)][(state[r][c] & 0x0f)];
		}
	}
}  // InvSubBytes

void InvMixColumns(byte state[4][Nb])
{
	byte temp[4][4];

	for (int r = 0; r < 4; ++r)  // copy State into temp[]
	{
		for (int c = 0; c < 4; ++c)
		{
			temp[r][c] = state[r][c];
		}
	}


	for (int c = 0; c < 4; ++c)
	{
		state[0][c] = (byte)((int)gfmultby0e(temp[0][c]) ^ (int)gfmultby0b(temp[1][c]) ^
			(int)gfmultby0d(temp[2][c]) ^ (int)gfmultby09(temp[3][c]));
		state[1][c] = (byte)((int)gfmultby09(temp[0][c]) ^ (int)gfmultby0e(temp[1][c]) ^
			(int)gfmultby0b(temp[2][c]) ^ (int)gfmultby0d(temp[3][c]));
		state[2][c] = (byte)((int)gfmultby0d(temp[0][c]) ^ (int)gfmultby09(temp[1][c]) ^
			(int)gfmultby0e(temp[2][c]) ^ (int)gfmultby0b(temp[3][c]));
		state[3][c] = (byte)((int)gfmultby0b(temp[0][c]) ^ (int)gfmultby0d(temp[1][c]) ^
			(int)gfmultby09(temp[2][c]) ^ (int)gfmultby0e(temp[3][c]));
	}
}  // InvMixColumns

void InvCipher(byte *input, byte *output, UsfulStuff &stuff)  // decipher 16-bit input
{
	// state = input
	byte state[4][Nb];  // always [4,4]
	for (int i = 0; i < (4 * Nb); ++i)
	{
		state[i % 4][i / 4] = input[i];
	}

	AddRoundKey(state, stuff, stuff.Nr);

	for (int round = stuff.Nr - 1; round >= 1; --round)  // main round loop
	{
		InvShiftRows(state);
		InvSubBytes(state);
		AddRoundKey(state, stuff, round);
		InvMixColumns(state);
	}  // end main round loop for InvCipher

	InvShiftRows(state);
	InvSubBytes(state);
	AddRoundKey(state, stuff, 0);

	// output = state
	for (int i = 0; i < (4 * Nb); ++i)
	{
		output[i] = state[i % 4][i / 4];
	}

}  // InvCipher()

int main()
{
	setlocale(LC_CTYPE, "rus");
	string toDo, keyFileName, materialFileName;
	cout << "�������� ��������:\n1 - ���������\n0 - ������������\n�� ��������� - ��������� ����������" << endl;
	getline(cin, toDo);
	if (toDo == "1" || toDo == "0") {
		cout << "������� ��� ����� � ������" << endl;
		getline(cin, keyFileName);
		int lengthKey = 0;
		ifstream inFileKey(keyFileName, ios::in);
		inFileKey.seekg(0, ios::end);
		lengthKey = (int)(inFileKey.tellg());
		inFileKey.seekg(0, ios::beg);
		if (lengthKey == 16 || lengthKey == 24 || lengthKey == 32) {
			byte *key = new byte[lengthKey];
			inFileKey.read((char *)key, lengthKey);
			inFileKey.close();
			UsfulStuff *stuff = KeyExpansion(key, lengthKey * 8);
			cout << "������� ��� ����� ��� ���������" << endl;
			getline(cin, materialFileName);
			byte inBuffer[16];
			byte outBuffer[16];
			int length = 0;
			if (toDo == "1") {	
				ifstream inFile(materialFileName, ios::binary);

				

				// 1. Encode the file.

				int length = 0;
				inFile.seekg(0, ios::end);
				length = (int)(inFile.tellg());
				inFile.seekg(0, ios::beg);

				// Add length to the first block
				for (int i = 0; i < 4; i++)
				{
					inBuffer[i] = (byte)(length >> (8 * i));
				}

				inFile.read((char *)inBuffer + 4, length < blockSize - 4 ? length : blockSize - 4);

				Cipher(inBuffer, outBuffer, *stuff);

				ofstream outFile("Encoded "+materialFileName, ios::out | ios::binary | ios::trunc);

				outFile.write((char *)outBuffer, blockSize);

				for (int i = blockSize - 4; i < length; i += blockSize)
				{
					inFile.read((char *)inBuffer, length - i >= blockSize ? blockSize : length - i);

					Cipher(inBuffer, outBuffer, *stuff);
					outFile.write((char *)outBuffer, blockSize);
				}

				inFile.close();
				outFile.close();
			}
			else {
				// 2. Decode the file
				ifstream inFile1(materialFileName, ios::binary);
				inFile1.seekg(0, ios::end);
				length = (int)(inFile1.tellg());
				inFile1.seekg(0, ios::beg);

				inFile1.read((char *)inBuffer, blockSize);
				InvCipher(inBuffer, outBuffer, *stuff);

				int lengthOut = outBuffer[0] + (outBuffer[1] << 8) + (outBuffer[2] << 16) + (outBuffer[3] << 24);

				ofstream outFile1("Decoded "+materialFileName, ios::out | ios::binary | ios::trunc);

				outFile1.write((char *)outBuffer + 4, lengthOut < blockSize - 4 ? lengthOut : blockSize - 4);

				for (int i = blockSize; i < length; i += blockSize)
				{
					inFile1.read((char *)inBuffer, blockSize);
					InvCipher(inBuffer, outBuffer, *stuff);
					outFile1.write((char *)outBuffer, lengthOut - (i - 4) >= blockSize ? blockSize : lengthOut - (i - 4));
				}
				inFile1.close();
				outFile1.close();

				ReleaseUsfulStuff(stuff);
			}
		}
		else {
			cout << "������ ����� �� ������������� ��������� (16, 24, 32) �����" << endl;
			system("pause");
		}
	}
	return 0;
}