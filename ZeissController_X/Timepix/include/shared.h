#ifndef SHARED_H
#define SHARED_H
#include <exception>
#include <fstream>
#include <cctype>

std::string getParamArgument(int argc, char** argv, std::string param, std::string defVal);

// trim from start
static inline std::string &ltrim(std::string &s)
{
    //s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
		return !std::isspace(ch);
		}));
    return s;
}

// trim from end
static inline std::string &rtrim(std::string &s)
{
   // s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
	s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
		return !std::isspace(ch);
		}).base(), s.end());
	return s;
}

// trim from both ends
static inline std::string &trim(std::string &s)
{
    return ltrim(rtrim(s));
}



std::string getParamArgument(int argc, char** argv, std::string param, std::string defVal)
{
	std::string ret = defVal;
	for (int i = 1; i < argc; i++)
	{
		if ((param.compare(argv[i]) == 0) && (argc > (i + 1)))
		{
			ret = argv[i + 1];
			break;
		}
	}
	return ret;
}

void writeRawData(i16* data, u32 nshorts, std::string fileName)
{
	std::ofstream _rawDataFile(fileName, std::ios::out | std::ios::binary);
	if (_rawDataFile.is_open())
	{
		_rawDataFile.write((const char*)data, sizeof(i16) * nshorts);
		_rawDataFile.close();
	}
}

void setHwInfo(MpxModule& module, std::string& hwFileName) {
	std::ifstream f(hwFileName);
	std::string line, key, value;
	while (std::getline(f, line)) {
		size_t col = line.find(':');
		if (col != std::string::npos) {
			std::string left = line.substr(0, col);
			key = trim(left);
			if (key.length() > 0) {
				std::string right = line.substr(col + 1, line.length());
				value = trim(right);
				module.setHwInfo(key, value);
			}
		}
	}
}

char const* dacs[] = {
	"IKrum","Disc","Preamp","BuffAnalogA","BuffAnalogB",
	"Hist","THL","THLCoarse","Vcas","FBK",
	"GND","THS","BiasLVDS","RefLVDS" };

int nDacs = 14;

static int dacIndex(const std::string& key) {
	int index = 0;
	while (index < nDacs && key.compare(dacs[index]) != 0)
		index++;
	return (index == nDacs ? -1 : index);
}

static void setDacs(MpxModule& module, const std::string& dacsFileName) {
	std::ifstream f(dacsFileName);
	std::string line;

	// Get a working-copy of the current DAC settings
	int ndevs = module.chipCount();
	int dacs[4][20], ndacs;
	for (int i = 0; i < ndevs; ++i)
		if (module.readFsr(i, dacs[i], &ndacs))
			std::cout << "### readFsr() chip " << i << " failed" << std::endl;

	int chipIx = -1;
	while (std::getline(f, line)) {
		if (line.substr(0, 5).compare("[Chip") == 0) {
			if (chipIx != -1)
				module.setFsr(chipIx, dacs[chipIx]);
			chipIx = line.at(5) - '0';
		}
		else {
			int col = line.find(':');
			if (col != std::string::npos) {
				std::string left = line.substr(0, col);
				std::string key = trim(left);
				int dacIx = dacIndex(key);
				if (dacIx >= 0) {
					std::string right = line.substr(col + 1, line.length());
					std::string value = trim(right);
					dacs[chipIx][dacIx] = std::stoi(value);
				}
			}
		}
	}
	if (chipIx != -1)
		module.setFsr(chipIx, dacs[chipIx]);
}



#endif
