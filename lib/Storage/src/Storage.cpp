#include "Storage.h"

// Initialize static variables
Storage::Media Storage::storageMedia = Storage::Media::LittleFS;
FS* Storage::storageSystem = &LittleFS;

/// @brief Mount LittleFS and format if necessary
/// @return True on successful mount of LittleFS
bool Storage::begin() {
	storageSystem = &LittleFS;
	storageMedia = Storage::Media::LittleFS;
	Serial.println("Mounting  LittleFS, this could take a while, please wait...");
	return LittleFS.begin(true, "/sd");
}

/// @brief Mount and initiate the storage for an SD card using SPI. Must be formatted as FAT32
/// @param mi The microcontroller in pin
/// @param mo The microcontroller out pin
/// @param sck The serial clock pin
/// @param cs The chip-select pin
/// @return True on success
bool Storage::begin(int mi, int mo, int sck, int cs) {
	storageSystem = &SD;
	storageMedia = Storage::Media::SD_SPI;
	// Start SPI bus
	SPI.begin(sck, mi, mo);
	bool success = true;
	Serial.println("Mounting storage...");
	if (!SD.begin(cs)) {
		Serial.println("Card mount failed, might need to format card as FAT32 or reduce clock frequency");
		success = false;
	} else {
		uint8_t cardType = SD.cardType();
		if (cardType == CARD_NONE) {
			Serial.println("No SD card attached. Must be formatted as FAT32");
			success = false;
		} else {
			Serial.print("SD card type: ");
			if (cardType == CARD_MMC) {
				Serial.println("MMC");
			} else if (cardType == CARD_SD) {
				Serial.println("SDSC");
			} else if (cardType == CARD_SDHC) {
				Serial.println("SDHC");
			} else {
				Serial.println("UNKNOWN");
			}
			uint64_t cardSize = SD_MMC.cardSize() / 1048576; // 1024 * 1024
			Serial.printf("SD_MMC card size: %lluMB\n", cardSize);
		}
	}
	return success;
}

/// @brief Mount and initiate the storage for an SD card using SDIO (e.g. https://www.adafruit.com/product/4682). Will format if necessary
/// @param clk The clock pin number
/// @param cmd The cmd pin number
/// @param d0 D0 pin number
/// @param d1 D1 pin number
/// @param d2 D2 pin number
/// @param d3 D3 pin number
/// @return True on success
bool Storage::begin(int clk, int cmd, int d0, int d1, int d2, int d3) {
	storageSystem = &SD_MMC;
	storageMedia = Storage::Media::SD_MMC;
	bool success = SD_MMC.setPins(clk, cmd, d0, d1, d2, d3);
	if (success) {
		Serial.println("Mounting storage...");
		if (!SD_MMC.begin("/sd", false, true, 40000)) {
			Serial.println("Card mount failed, might need to reduce sd_mmc frequency to 10000");
			success = false;
		} else {
			uint8_t cardType = SD_MMC.cardType();

			if(cardType == CARD_NONE) {
				Serial.println("No SD_MMC card attached");
				success = false;
			} else {
				Serial.print("SD_MMC card type: ");
				if (cardType == CARD_MMC) {
					Serial.println("MMC");
				} else if(cardType == CARD_SD) {
					Serial.println("SDSC");
				} else if(cardType == CARD_SDHC) {
					Serial.println("SDHC");
				} else {
					Serial.println("UNKNOWN");
				}
				uint64_t cardSize = SD_MMC.cardSize() / 1048576; // 1024 * 1024
				Serial.printf("SD_MMC card size: %lluMB\n", cardSize);
			}
		}
	}
	return success;
}

/// @brief Gets the currently used file system
/// @return A pointer to storage media/file system being used
fs::FS* Storage::getFileSystem() {
	return storageSystem; 
}

/// @brief Gets the currently used media type
/// @return The type of media in use
Storage::Media Storage::getMediaType() {
	return storageMedia;
}

/// @brief List the files in a directory
/// @param dirname The directory path to list
/// @param levels How many levels to recurse into the directory for listing
/// @return A collection of strings of full paths of the files found
std::vector<String> Storage::listFiles(String dirname, uint8_t levels) {
	Serial.println("Listing directory: " + dirname);
	std::vector<String> folderContents;
	File root = storageSystem->open(dirname);
	if(!root){
		Serial.println("Failed to open directory");
		return folderContents;
	}
	if (!root.isDirectory()) {
		Serial.println("Not a directory");
		return folderContents;
	}
	File file = root.openNextFile();
	while(file) {
		if(file.isDirectory()) {
			Serial.print("  DIR : ");
			Serial.println(file.name());
			if(levels) {
				// Recurse and add subdir contents to file list
				for (const auto& f : listFiles(file.path(), levels - 1)) {
					folderContents.push_back(f);
				}
			}
		} else {
			folderContents.push_back(String(file.path()));
		}
		file = root.openNextFile();
	}
	return folderContents;
}

/// @brief List the folders in a directory
/// @param dirname The directory path to list
/// @param levels How many levels to recurse into the directory for listing
/// @return A collection of strings of full paths of the directories
std::vector<String> Storage::listDirs(String dirname, uint8_t levels) {
	Serial.println("Listing directory: " + dirname);
	std::vector<String> folderContents;
	File root = storageSystem->open(dirname);
	if(!root){
		Serial.println("Failed to open directory");
		return folderContents;
	}
	if (!root.isDirectory()) {
		Serial.println("Not a directory");
		return folderContents;
	}
	File file = root.openNextFile();
	while(file) {
		if(file.isDirectory()) {
			folderContents.push_back(String(file.path()));
			Serial.println(file.name());
			if(levels) {
				// Recurse and add subdir contents to file list
				for (const auto& d : listDirs(file.path(), levels - 1)) {
					folderContents.push_back(d);
				}
			}
		}
		file = root.openNextFile();
	}
	return folderContents;
}

/// @brief Checks if a file or directory exists on the storage
/// @param path The path of the file or directory
/// @return True if it exists
bool Storage::fileExists(String path) {
	Serial.println("Checking for file: " + path);
	return storageSystem->exists(path);
}

/// @brief Creates a directory on the storage
/// @param path The path of the directory to create
/// @return True on success
bool Storage::createDir(String path) {
	Serial.println("Creating Dir: " + path);
	return storageSystem->mkdir(path);;
}

/// @brief Removes a directory from the storage
/// @param path The path of the directory to remove
/// @return True on success
bool Storage::removeDir(String path) {
	Serial.println("Removing Dir:" + path);
	return storageSystem->rmdir(path);
}

/// @brief Reads the contents of a file from the storage
/// @param path The path of the file to read
/// @return A String of the file contents, empty string on failure
String Storage::readFile(String path) {
	Serial.println("Reading file: " + path);
	File file = storageSystem->open(path);
	if (!file) {
		Serial.println("Failed to open file for reading");
		return "";
	}
	String output = "";
	while(file.available()){
		output += file.readString();
	}
	file.close();
	return output;
}

/// @brief Writes data to a file, creates or overwrites a file if necessary
/// @param path The path of the file to write
/// @param content The content of the file to write
/// @return True on success
bool Storage::writeFile(String path, String content) {
	Serial.println("Writing file: " + path);
	File file = storageSystem->open(path, FILE_WRITE);
	if (!file) {
		Serial.println("Failed to open file for writing");
		return false;
	}
	return file.print(content) > 0;
}

/// @brief Appends data to a file
/// @param path The path of the file to append
/// @param content The content to append
/// @return True on success
bool Storage::appendToFile(String path, String content) {
	Serial.println("Appending to file: " + path);
	File file = storageSystem->open(path, FILE_APPEND);
	if (!file) {
		Serial.println("Failed to open file for appending");
		return false;
	}
	return file.print(content) > 0;
}

/// @brief Renames/moves a file on the storage
/// @param path1 The original path/name of the file
/// @param path2 The new path/name of the file
/// @return True on success
bool Storage::renameFile(String path1, String path2) {
	Serial.println("Renaming file" + path1 + " to " + path2);
	return storageSystem->rename(path1, path2);
}

/// @brief Deletes a file from the storage
/// @param path The path of the file to delete
/// @return True on success
bool Storage::deleteFile(String path) {
	Serial.println("Deleting file: " + path);
	return storageSystem->remove(path);
}

/// @brief Get free space on filesystem
/// @return The number of free bytes
size_t Storage::freeSpace() {
	switch (storageMedia)
	{
		case Storage::Media::LittleFS:
			return LittleFS.totalBytes() - LittleFS.usedBytes();
			break;
		case Storage::Media::SD_SPI:
			return SD.totalBytes() - SD.usedBytes();
			break;
		case Storage::Media::SD_MMC:
			return SD_MMC.totalBytes() - SD_MMC.usedBytes();
			break;
	}
	return 0;
}