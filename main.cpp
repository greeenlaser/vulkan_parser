#include <iostream>
#include <filesystem>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>

using std::cout;
using std::cin;
using std::filesystem::current_path;
using std::filesystem::path;
using std::filesystem::exists;
using std::filesystem::copy_file;
using std::filesystem::copy_options;
using std::filesystem::directory_iterator;
using std::string;
using std::to_string;
using std::ifstream;
using std::ofstream;
using std::getline;
using std::vector;
using std::exception;

enum class MessageType
{
	TYPE_MESSAGE,
	TYPE_SUCCESS,
	TYPE_ERROR
};

static void PrintMessage(
	MessageType type,
	const string& message,
	int indentCount = 0);
static bool ParseExtensions();
static bool ParseLayers();

static path parser_in{};
static path parser_out{};
static path result_log_path{};
static ofstream result_log{};

int main ()
{
	parser_in = path(current_path().parent_path().parent_path().parent_path() / "parser_in");
	if (!exists(parser_in))
	{
		PrintMessage(
			MessageType::TYPE_ERROR, 
			"Failed to find parser_in folder from '" + parser_in.string() + "'!");
		
		cin.get();
		return 0;
	}
	
	parser_out = path(current_path().parent_path().parent_path().parent_path() / "parser_out");
	if (!exists(parser_out))
	{
		PrintMessage(
			MessageType::TYPE_ERROR, 
			"Failed to find parser_out folder from '" + parser_out.string() + "'!");
			
		cin.get();
		return 0;
	}
	
	result_log_path = path(parser_out / "result_log.txt");
	result_log.open(result_log_path);
	if (!result_log.is_open())
	{
		PrintMessage(
			MessageType::TYPE_ERROR, 
			"Failed to open '" + result_log_path.string() + "' for writing!");
		
		cin.get();
		return 0;
	}
	
	PrintMessage(
		MessageType::TYPE_MESSAGE, 
		"Starting to parse extensions...");
	
	if (ParseExtensions()) 
	{
		PrintMessage(
			MessageType::TYPE_SUCCESS,
			"Parsed all extensions!");
	}
	else 
	{
		PrintMessage(
			MessageType::TYPE_ERROR, 
			"Failed to parse extensions!");
			
		result_log.close();
		cin.get();
		return 0;
	}
	
	PrintMessage(
		MessageType::TYPE_MESSAGE, 
		"Starting to parse layers...");
	
	if (ParseLayers()) 
	{
		PrintMessage(
			MessageType::TYPE_SUCCESS,
			"Parsed all layers!");
	}
	else 
	{
		PrintMessage(
			MessageType::TYPE_ERROR, 
			"Failed to parse layers!");
			
		result_log.close();
		cin.get();
		return 0;
	}
	
	result_log.close();
	cin.get();
	return 0;
}

void PrintMessage(
	MessageType type,
	const string& message,
	int indentCount)
{
	string typeStr{};
	switch (type)
	{
	case MessageType::TYPE_MESSAGE:
	{
		typeStr = "";
		break;
	}
	case MessageType::TYPE_SUCCESS:
	{
		typeStr = "[SUCCESS] ";
		break;
	}
	case MessageType::TYPE_ERROR:
	{
		typeStr = "[ERROR] ";
		break;
	}
	}
	
	string indentStr{};
	for (int i = 0; i < indentCount; ++i)
	{
		indentStr += " ";
	}
	
	string fullMessage = indentStr + typeStr + message + "\n";
	
	cout << fullMessage;
	result_log << fullMessage;
}

bool ParseExtensions()
{
	path filePath = path(parser_in / "vk.xml");
	if (!exists(filePath))
	{
		PrintMessage(
			MessageType::TYPE_ERROR, 
			"Failed to find file '" + filePath.string() + "'!");
		return false;
	}
	
	ifstream file(filePath);
	if (!file.is_open())
	{
		PrintMessage(
			MessageType::TYPE_ERROR, 
			"Failed to open file '" + filePath.string() + "' for reading!");
		return false;
	}
	
	vector<string> instanceExtensions{};
	vector<string> deviceExtensions{};
	string line{};
	
	while (getline(file, line))
	{
		if (line.find("<extension") == string::npos) continue;
		
		//extract attributes
		auto ExtractAttribute = [](const string& line, const string& key) -> string
		{
			string pattern = key + "=\"";
			size_t start = line.find(pattern);
			if (start == string::npos) return "";
			
			start += pattern.length();
			size_t end = line.find('"', start);
			if (end == string::npos) return "";
			
			return line.substr(start, end - start);
		};
		
		const string name = ExtractAttribute(line, "name");
		const string supported = ExtractAttribute(line, "supported");
		const string promoted = ExtractAttribute(line, "promotedto");
		const string type = ExtractAttribute(line, "type");
		
		//skip extensions promoted to Vulkan core
		if (promoted == "vulkan12"
			|| promoted == "vulkan13"
			|| promoted == "vulkan14")
		{
			continue;
		}

		//skip disabled, ratified-only, or undefined extensions
		if (supported != "vulkan") continue;
		if (name.empty()) continue;
		if (name.find("VK_VERSION_") == 0) continue;
		if (name.find("_extension_") != string::npos) continue;

		//skip vendor- and OS-specific junk (non-Windows, non-X11)
		if (name.starts_with("VK_AMD_")
			|| name.starts_with("VK_AMDX_")
			|| name.starts_with("VK_NV_")
			|| name.starts_with("VK_NVX_")
			|| name.starts_with("VK_INTEL_")
			|| name.starts_with("VK_QCOM_")
			|| name.starts_with("VK_ARM_")
			|| name.starts_with("VK_IMG_")
			|| name.starts_with("VK_HUAWEI_")
			|| name.starts_with("VK_MSFT_")
			|| name.starts_with("VK_FUCHSIA_")
			|| name.starts_with("VK_VALVE_")
			|| name.starts_with("VK_GOOGLE_")
			|| name.starts_with("VK_LUNARG_")
			|| name.starts_with("VK_SEC_")
			|| name.starts_with("VK_ANDROID_")
			|| name.starts_with("VK_NOKIA_")
			|| name.starts_with("VK_BRCM_")
			|| name.starts_with("VK_VIV_")
			|| name.starts_with("VK_RENESAS_")
			|| name.starts_with("VK_TI_")
			|| name.starts_with("VK_MVK_")
			|| name.starts_with("VK_QNX_")
			|| name.starts_with("VK_IOS_")
			|| name.starts_with("VK_MACOS_")
			|| name.starts_with("VK_GGP_")
			|| name.starts_with("VK_NN_")
			|| name.starts_with("VK_MESA_")
			|| name.starts_with("VK_MTK_")
			|| name.starts_with("VK_FB_")
			|| name.starts_with("VK_COREAVI_")
			|| name.starts_with("VK_OHOS_")
			|| name.find("wayland") != string::npos
			|| name.find("directfb") != string::npos
			|| name.find("metal") != string::npos
			|| name.find("drm") != string::npos)
		{
			continue;
		}
		
		if (!name.empty())
		{
				if (type == "device") deviceExtensions.push_back(name);
				else if (type == "instance") instanceExtensions.push_back(name);
		}
		
		PrintMessage(
			MessageType::TYPE_SUCCESS, 
			"Found '" + type + "' extension '" + name + "'!",
			2);
	}
	
	file.close();
	
	size_t deviceExtSize = deviceExtensions.size();
	size_t instanceExtSize = instanceExtensions.size();
	if (deviceExtSize == 0)
	{
		PrintMessage(
			MessageType::TYPE_ERROR, 
			"Failed to find any opt-in vulkan 1.2 device extensions from file '" + filePath.string() + "'!");
		return false;
	}
	if (instanceExtSize == 0)
	{
		PrintMessage(
			MessageType::TYPE_ERROR, 
			"Failed to find any opt-in vulkan 1.2 instance extensions from file '" + filePath.string() + "'!");
		return false;
	}
	
	//
	// CREATE INSTANCE EXTENSIONS LIST
	//
	
	//write output
	path deviceOutputPath = path(parser_out / "vulkan_device_extensions.txt");
	
	ofstream deviceOut(deviceOutputPath);
	if (!deviceOut.is_open())
	{
		PrintMessage(
			MessageType::TYPE_ERROR, 
			"Failed to open file '" + deviceOutputPath.string() + "' for writing!");
		return false;
	}
	
	for (const auto& ext : deviceExtensions)
	{
		deviceOut << ext << "\n";
	}
	deviceOut.close();
	
	PrintMessage(
		MessageType::TYPE_SUCCESS, 
		"Found and saved '" + to_string(deviceExtSize) + 
		"' device extensions to '" + deviceOutputPath.string() + "'!");
	
	//
	// CREATE DEVICE EXTENSIONS LIST
	//
	
	//write output
	path instanceOutputPath = path(parser_out / "vulkan_instance_extensions.txt");
	
	ofstream instanceOut(instanceOutputPath);
	if (!instanceOut.is_open())
	{
		PrintMessage(
			MessageType::TYPE_ERROR, 
			"Failed to open file '" + instanceOutputPath.string() + "' for writing!");
		return false;
	}
	
	for (const auto& ext : instanceExtensions)
	{
		instanceOut << ext << "\n";
	}
	instanceOut.close();
	
	PrintMessage(
		MessageType::TYPE_SUCCESS, 
		"Found and saved '" + to_string(instanceExtSize) + 
		"' instance extensions to '" + instanceOutputPath.string() + "'!");

	return true;
}

bool ParseLayers()
{
	vector<string> instanceLayers{};
	
	for (const auto& file : directory_iterator(parser_in))
	{
		path filePath = path(file);
		
		if (filePath.extension() != ".json") continue;
		
		ifstream file(filePath);
		if (!file.is_open())
		{
			PrintMessage(
				MessageType::TYPE_ERROR, 
				"Failed to open file '" + filePath.string() + "' for reading!");
			return false;
		}
		
		string line{};
		string layerName{};
		
		while (getline(file, line))
		{
			auto pos = line.find("\"name\"");
			if (pos != string::npos)
			{
				size_t start = line.find("\"", pos + 6);
				size_t end = line.find("\"", start + 1);
				if (start != string::npos
					&& end != string::npos)
				{
					layerName = line.substr(start + 1, end - start - 1);
					break;
				}
			}
		}
		file.close();
		
		if (layerName.empty()) continue;
			
		if (layerName != "VK_LAYER_KHRONOS_validation" 
			&& !layerName.starts_with("VK_LAYER_KHRONOS_")
			&& !layerName.starts_with("VK_LAYER_LUNARG_"))
		{
			if (layerName.starts_with("VK_LAYER_RENDERDOC_")
				|| layerName.starts_with("VK_LAYER_NV_")
				|| layerName.starts_with("VK_LAYER_MESA_")
				|| layerName.starts_with("VK_LAYER_INTEL_")
				|| layerName.starts_with("VK_LAYER_GOOGLE_")
				|| layerName.starts_with("VK_LAYER_OBSOLETE_"))
			{
				continue;
			}
		}
			
		instanceLayers.push_back(layerName);
			
		PrintMessage(
			MessageType::TYPE_SUCCESS, 
			"Found instance layer '" + layerName + "'!",
			2);
	}
	
	size_t instanceLayersSize = instanceLayers.size();
	if (instanceLayersSize == 0)
	{
		PrintMessage(
			MessageType::TYPE_ERROR, 
			"Failed to find any vulkan 1.2 instance layers!");
		return false;
	}
		
	//write output
	path instanceLayerOutputPath = path(parser_out / "vulkan_instance_layers.txt");
		
	ofstream instanceLayerOut(instanceLayerOutputPath);
	if (!instanceLayerOut.is_open())
	{
		PrintMessage(
			MessageType::TYPE_ERROR, 
			"Failed to open file '" + instanceLayerOutputPath.string() + "' for writing!");
		return false;
	}
		
	for (const auto& ext : instanceLayers)
	{
		instanceLayerOut << ext << "\n";
	}
	instanceLayerOut.close();
		
	PrintMessage(
		MessageType::TYPE_SUCCESS, 
		"Found and saved '" + to_string(instanceLayersSize) + 
		"' instance layers to '" + instanceLayerOutputPath.string() + "'!");
	
	return true;
}