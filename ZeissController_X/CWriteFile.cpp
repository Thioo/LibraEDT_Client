#include "pch.h"

CWriteFile* CWriteFile::m_pWriteFile = nullptr;

bool CWriteFile::create_directory(std::string& _directory_to_create)
{
	bool bReturn = false;
	if (std::filesystem::create_directories(_directory_to_create))
	{
		printf("error creating directory %s\n", _directory_to_create.c_str());
		return bReturn;
	}

	return true;
}

bool CWriteFile::create_directory(const char* _directory_to_create_cstr)
{
	bool bReturn = false;
    std::string _directory_to_create(_directory_to_create_cstr);
	if (std::filesystem::create_directories(_directory_to_create))
	{
		printf("error creating directory %s\n", _directory_to_create.c_str());
		return bReturn;
	}

	return true;
}

CWriteFile::CWriteFile()
{

    CWriteFile::create_directory("C:\\Users\\TEM\\Documents\\Moussa_SoftwareImages\\Logs");
    CWriteFile::create_directory("C:\\Users\\TEM\\Documents\\Moussa_SoftwareImages\\ImgBasedRecord");
    CWriteFile::create_directory("C:\\Users\\TEM\\Documents\\Moussa_SoftwareImages\\EucentricHeight");
    CWriteFile::create_directory("C:\\Users\\TEM\\Documents\\Moussa_SoftwareImages\\LiveStreaming");

    _param_file_name = "Params.ini";
    is_config_loaded = false;
}


CWriteFile::~CWriteFile()
{
    printf("CWriteFile::Destructor");
    //SAFE_RELEASE(m_pWriteFile);
}


void CWriteFile::restore_param(std::string _key, float& _var, std::string _customPrefix, std::map<std::string, std::string>* configMap)
{
	if (configMap == nullptr)
		configMap = &m_configMap;

	if (_customPrefix != "")
		_key = _customPrefix + _key;

    if (configMap->empty() || configMap->contains(_key) == false)
        return;
    
    _var = std::stof(configMap->at(_key));
}

void CWriteFile::restore_param(std::string _key, int& _var, std::string _customPrefix, std::map<std::string, std::string>* configMap)
{
	if (configMap == nullptr)
		configMap = &m_configMap;

	if (_customPrefix != "")
		_key = _customPrefix + _key;

	if (configMap->empty() || configMap->contains(_key) == false)
		return;

	_var = std::stoi(configMap->at(_key));
}

void CWriteFile::restore_param(std::string _key, bool& _var, std::string _customPrefix, std::map<std::string, std::string>* configMap)
{
	if (configMap == nullptr)
		configMap = &m_configMap;

	if (_customPrefix != "")
		_key = _customPrefix + _key;

	if (configMap->empty() || configMap->contains(_key) == false)
		return;

	_var = static_cast<bool>(std::stoi(configMap->at(_key)));
}

void CWriteFile::restore_param(std::string _key, std::string& _var, std::string _customPrefix, std::map<std::string, std::string>* configMap)
{
	if (configMap == nullptr)
		configMap = &m_configMap;

    if (_customPrefix != "")
        _key = _customPrefix + _key;

	if (configMap->empty() || configMap->contains(_key) == false)
		return;

	_var = configMap->at(_key);
}

void CWriteFile::restore_param(std::string _key, double& _var, std::string _customPrefix, std::map<std::string, std::string>* configMap)
{
	if (configMap == nullptr)
		configMap = &m_configMap;

	if (_customPrefix != "")
		_key = _customPrefix + _key;

	if (configMap->empty() || configMap->contains(_key) == false)
		return;

	_var = std::stod(configMap->at(_key));
}

void CWriteFile::restore_param(std::string _key, unsigned int& _var, std::string _customPrefix /*= ""*/, std::map<std::string, std::string>* configMap /*= nullptr*/)
{
	if (configMap == nullptr)
		configMap = &m_configMap;

	if (_customPrefix != "")
		_key = _customPrefix + _key;

	if (configMap->empty() || configMap->contains(_key) == false)
		return;

	_var = std::stod(configMap->at(_key));
}

bool CWriteFile::exists(std::string& _name)
{
    if (FILE* file = fopen(_name.c_str(), "r")) {
        fclose(file);
        return true;
    }
    else {
        return false;
    }
}

bool CWriteFile::WriteToFile(std::string _filename, std::map<unsigned int, CVec>& _map, bool _bEverything)
{
    std::ofstream _file;
    _file.open(_filename, std::ofstream::app);

    if (_bEverything)
    {
        _file << "Key(º|t),\tPosX,\tPosY,\tPosZ,\tTilt,\tTime(ms),\tSpeed(Deg/s)" << std::endl;
        
        if (_map.begin()->second.iTime <= _map.end()->second.iTime)
        {
            for (const auto& val : _map)
            {
               // double dKeyToAngle = static_cast<double>(val.first) / _ANG_TO_KEY_MULTIPLIER_;
                
                double dKey = val.first;
                _file << dKey << ",\t" << val.second.fX << ",\t" << val.second.fY << ",\t";
                _file << val.second.fZ << ",\t" << val.second.fT << ",\t" << val.second.iTime << ",\t" << val.second.fRotSpeed << std::endl;
            }
        }
        else
        {
            for (auto it = _map.rbegin(); it != _map.rend(); it++)
            {
                //double dKeyToAngle = static_cast<double>(it->first) / _ANG_TO_KEY_MULTIPLIER_;
                double dKey = it->first;
                _file << dKey << ",\t" << it->second.fX << ",\t" << it->second.fY << ",\t";
				_file << it->second.fZ << ",\t" << it->second.fT << ",\t" << it->second.iTime << ",\t" << it->second.fRotSpeed << std::endl;
            }
        }
    }
    else
    {
        _file << "Key(º|t),\tRotation Speed(Deg/s),\tTime (ms),\tTime(s) " << std::endl;

        if (_map.begin()->second.iTime <= _map.end()->second.iTime)
        {
            for (const auto& val : _map) // if 
            {
              //  double dKeyToAngle = static_cast<double>(val.first) / _ANG_TO_KEY_MULTIPLIER_;
                double dKey = val.first;
                float fTime = val.second.iTime / 1000.0f;
                _file << dKey << ",\t" << val.second.fRotSpeed << ",\t" << val.second.iTime << ",\t" << fTime << std::endl;
            }
        }
        else
        {
            for (auto it = _map.rbegin(); it != _map.rend(); it++)
            {
                //double dKeyToAngle = static_cast<double>(it->first) / _ANG_TO_KEY_MULTIPLIER_;
                double dKey = it->first;
                float fTime = it->second.iTime / 1000.0f;
                _file << dKey << ",\t" << it->second.fRotSpeed << ",\t" << it->second.iTime << ",\t" << fTime << std::endl;
            }
        }
    }
    _file.close();
    return false;
}
void CWriteFile::write_params(std::vector<my_params>& params, std::ofstream& file)
{
	for (auto& v : params)
		file << v.param_name << "=" << v.param_val << std::endl;
}

void CWriteFile::write_params_file()
{
	std::ofstream config_file(_param_file_name);

    // From CDataCollection
    auto params_vec = CDataCollection::GetInstance()->save_parameters();
    write_params(params_vec, config_file);

    config_file.close();
}

void CWriteFile::write_tracking_data(std::string& fileName)
{
	std::ofstream config_file(fileName);

	// From CDataCollection
	auto params_vec = CDataCollection::GetInstance()->save_tracking_data();
	write_params(params_vec, config_file);

	config_file.close();
}

void CWriteFile::read_params_file(std::string sFileName, std::map<std::string, std::string>* configMap)
{
    if (configMap == nullptr)
        configMap = &m_configMap;
    if (sFileName == "")
        sFileName = _param_file_name;

	if (exists(sFileName))
	{
		std::ifstream file(sFileName);
		std::string line;
		while (std::getline(file, line)) {
			size_t pos = line.find('=');
			if (pos != std::string::npos) {
				std::string parameter = line.substr(0, pos);
				std::string value = line.substr(pos + 1);
                (*configMap)[parameter] = value;
			}
		}
		file.close();
	}
}

std::string CWriteFile::GenerateNewFileName(float _From, float _To, unsigned int _index)
{

    std::string _fileName;
    _fileName.reserve(128);

    _fileName.append("C:/Users/TEM/Documents/Moussa_SoftwareImages/Logs/From_");
    _fileName.append(std::to_string(_From));
    _fileName.append("To_");
    _fileName.append(std::to_string(_To));
    _fileName.append("_Speed_");
    _fileName.append(std::to_string(_index));
    _fileName.append("%_0.txt");


    int i = 1;
    while (exists(_fileName))
    {
        std::string sNewName = _fileName.substr(0, _fileName.find_last_of("_") + 1);
        sNewName.append(std::to_string(i));
        _fileName = sNewName;
        _fileName.append(".txt");
        i++;
    }

    return _fileName;
}

CWriteFile* CWriteFile::GetInstance()
{
	PRINTD("\t\t\t\CWriteFile::GetInstance\n");
	if (m_pWriteFile == nullptr)
        m_pWriteFile = new CWriteFile();

	return m_pWriteFile;
    //return m_pWriteFile ? m_pWriteFile : new CWriteFile();
}