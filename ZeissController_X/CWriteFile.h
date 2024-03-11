#ifndef _ZEISS_WRITEFILE_H
#define _ZEISS_WRITEFILE_H
struct CVec;
struct my_params;

class CWriteFile
{

private:
	static CWriteFile* m_pWriteFile;
	std::map<int, CVec> _map;
	std::map<std::string, std::string> m_configMap;

public:
	std::string _param_file_name;
	bool is_config_loaded;

public:

	static bool create_directory(std::string& _directory_to_create);
	static bool create_directory(const char* _directory_to_create_cstr);
	bool exists(std::string& _name);
	bool WriteToFile(std::string _filename, std::map<unsigned int, CVec>& _map, bool _bEverything = false);
	void write_params_file();
	void write_tracking_data(std::string& fileName);
	void read_params_file(std::string sFileName = "", std::map<std::string, std::string>* configMap = nullptr);
	std::string GenerateNewFileName(float _From, float _To, unsigned int _index);
	static CWriteFile* GetInstance();
	~CWriteFile();

	void restore_param(std::string _key, float& _var, std::string _customPrefix = "", std::map<std::string, std::string>* configMap = nullptr);
	void restore_param(std::string _key, double& _var, std::string _customPrefix = "", std::map<std::string, std::string>* configMap = nullptr);
	void restore_param(std::string _key, int& _var, std::string _customPrefix = "", std::map<std::string, std::string>* configMap = nullptr);
	void restore_param(std::string _key, unsigned int& _var, std::string _customPrefix = "", std::map<std::string, std::string>* configMap = nullptr);
	void restore_param(std::string _key, bool& _var, std::string _customPrefix = "", std::map<std::string, std::string>* configMap = nullptr);
	void restore_param(std::string _key, std::string& _var, std::string _customPrefix = "", std::map<std::string, std::string>* configMap = nullptr);

	void empty_map() { m_configMap.clear(); }
	std::map<std::string, std::string>& get_configMap() { return m_configMap; };
private:
	CWriteFile();
	void write_params(std::vector<my_params>& params, std::ofstream& file);
};

struct my_params
{
	std::string param_name;
	std::string param_val;

	my_params(std::string _name, std::string _val) { param_name = _name; param_val = _val; }
	my_params(std::string _name, float _val) { param_name = _name; param_val = std::format("{:.10f}", _val); }
	my_params(std::string _name, double _val) { param_name = _name; param_val = std::format("{:.10f}", _val); }
	my_params(std::string _name, int _val) { param_name = _name; param_val = std::format("{}", _val); }
	my_params(std::string _name, unsigned int _val) { param_name = _name; param_val = std::format("{}", _val); }
};

#endif