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
static bool CopyVK();
static bool ParseVK();

int main ()
{
	PrintMessage(
		MessageType::TYPE_MESSAGE, 
		"Starting to copy VK file...");
	
	if (CopyVK())
	{
		PrintMessage(
			MessageType::TYPE_SUCCESS,
			"Copied VK!");
	}
	else 
	{
		PrintMessage(
			MessageType::TYPE_ERROR, 
			"Failed to copy VK file!");
		
		cin.get();
		return 0;
	}
	
	PrintMessage(
		MessageType::TYPE_MESSAGE, 
		"Starting to parse VK file...");
	
	if (ParseVK()) 
	{
		PrintMessage(
			MessageType::TYPE_SUCCESS,
			"Parsed VK file!");
	}
	else 
	{
		PrintMessage(
			MessageType::TYPE_ERROR, 
			"Failed to parse VK file!");
			
		cin.get();
		return 0;
	}
	
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
	
	cout << indentStr << typeStr << message << "\n";
}

bool CopyVK()
{
	path source = path(current_path().parent_path().parent_path().parent_path() / "vk.xml");
	if (!exists(source))
	{
		PrintMessage(
			MessageType::TYPE_ERROR, 
			"Failed to find source file '" + source.string() + "'!");
		return false;
	}
	
	path destination = path(current_path() / "vk.xml");
	
	try
	{
		copy_file
		(
			source,
			destination,
			copy_options::overwrite_existing
		);	
	}
	catch (const exception& e)
	{
		PrintMessage(
			MessageType::TYPE_ERROR, 
			"Failed to copy file '" + source.string() + 
			"' to  '" + destination.string() +
			"' because '" + e.what() + "'!");
		return false;
	}
	
	return true;
}

bool ParseVK()
{
	path filePath = path(current_path() / "vk.xml");
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
	
	vector<string> validExtensions{};
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
			
			string result = line.substr(start, end - start);
			return result;
		};
		
		const string name = ExtractAttribute(line, "name");
		const string supported = ExtractAttribute(line, "supported");
		const string promoted = ExtractAttribute(line, "promotedto");
		
		// skip extensions promoted to Vulkan core
		if (promoted == "vulkan12"
			|| promoted == "vulkan13"
			|| promoted == "vulkan14")
		{
			continue;
		}

		// skip disabled, ratified-only, or undefined extensions
		if (supported != "vulkan") continue;
		if (name.empty()) continue;
		if (name.find("VK_VERSION_") == 0) continue;
		if (name.find("_extension_") != string::npos) continue;

		// skip vendor- and OS-specific junk (non-Windows, non-X11)
		if (name.starts_with("VK_AMD_")
			|| name.starts_with("VK_NV_")
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
			|| name.starts_with("VK_MSFT_")
			|| name.starts_with("VK_OHOS_")
			|| name.find("wayland") != string::npos
			|| name.find("directfb") != string::npos
			|| name.find("metal") != string::npos
			|| name.find("drm") != string::npos)
		{
			continue;
		}
		
		if (!name.empty()) validExtensions.push_back(name);
		
		PrintMessage(
			MessageType::TYPE_SUCCESS, 
			"Found result '" + name + "'!",
			2);
	}
	
	file.close();
	
	size_t outputSize = validExtensions.size();
	if (outputSize == 0)
	{
		PrintMessage(
			MessageType::TYPE_ERROR, 
			"Failed to find any opt-in vulkan 1.2 extensions from file '" + filePath.string() + "'!");
		return false;
	}
	
	//write output
	path outputPath = path(current_path() / "vulkan_1_2_opt_in.txt");
	
	ofstream out(outputPath);
	if (!out.is_open())
	{
		PrintMessage(
			MessageType::TYPE_ERROR, 
			"Failed to open file '" + outputPath.string() + "' for editing!");
		return false;
	}
	
	for (const auto& ext : validExtensions)
	{
		out << ext << "\n";
	}
	out.close();
	
	PrintMessage(
		MessageType::TYPE_SUCCESS, 
		"Found and saved '" + to_string(validExtensions.size()) + 
		"' opt-in extensions to '" + outputPath.string() + "'!");
	return true;
}