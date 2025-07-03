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
			
			PrintMessage(
				MessageType::TYPE_SUCCESS, 
				"Found result '" + result + "'!",
				2);

			return result;
		};
		
		const string name = ExtractAttribute(line, "name");
		const string supported = ExtractAttribute(line, "supported");
		const string promoted = ExtractAttribute(line, "promotedto");
		
		if (supported != "vulkan") continue;
		if (promoted == "vulkan13"
			|| promoted == "vulkan14")
		{
			continue;	
		}
		
		if (!name.empty()) validExtensions.push_back(name);
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