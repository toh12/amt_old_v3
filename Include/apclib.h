/* 
*	Awesomenauts Patching and Cryptography Library
*	
*	Provides functions to patch the game and and access files in a way that 
*	allows for editing to occur. Editing files without patching the game
*	will cause the game to crash. Patching the game disables matchmaking,
*	but still allows for online custom games.
*/
#pragma once

#include <string>
#include <exception>
#include <filesystem>
#include <set>

//Exceptions to throw to callers

class CryptoException : public virtual std::runtime_error {
public:
	CryptoException(const std::string& msg);
    virtual const char* what() const throw ();
};

class CompressionException : public virtual std::runtime_error {
public:
    CompressionException(const std::string& msg);
    virtual const char* what() const throw ();
};

class BinderException : public virtual std::runtime_error {
public:
	BinderException(const std::string& msg);
	virtual const char* what() const throw ();
};

class FileException : public virtual std::runtime_error {
public:
    FileException(const std::string& msg);
    virtual const char* what() const throw ();
};

//Interface

namespace apcl {
	class BinaryFile {
	public:
		BinaryFile();
		BinaryFile(const std::string& filename, const std::string& extension = "");
		BinaryFile(unsigned int filesize, const std::string& filename, const std::string& extension = "");
		BinaryFile(const unsigned char* begin, const unsigned char* end, const std::string& filename, const std::string& extension = "");
		BinaryFile(const BinaryFile& other);
		BinaryFile(BinaryFile&& other) noexcept;
		BinaryFile& operator=(BinaryFile other);
		friend void swap(BinaryFile& lhs, BinaryFile& rhs) noexcept;
		~BinaryFile();
		friend bool operator==(const BinaryFile& lhs, const BinaryFile& rhs);
		friend bool operator!=(const BinaryFile& lhs, const BinaryFile& rhs);
		friend bool operator< (const BinaryFile& lhs, const BinaryFile& rhs);
		friend bool operator> (const BinaryFile& lhs, const BinaryFile& rhs);
		friend bool operator<=(const BinaryFile& lhs, const BinaryFile& rhs);
		friend bool operator>=(const BinaryFile& lhs, const BinaryFile& rhs);
		const std::string& GetFilename() const;
		const std::string& GetExtension() const;
		unsigned char* begin();
		unsigned char* end();
		const unsigned char* begin() const;
		const unsigned char* end() const;
		unsigned int size() const;
		void resize(unsigned int size);
		void Load(std::filesystem::path directory);
		void Save(std::filesystem::path directory) const;
		void Encrypt(std::filesystem::path directory);
		void Decrypt(std::filesystem::path directory);
		void Compress();
		void Decompress();
	private:
		std::string filename;
		std::string extension;
		unsigned int filesize;
		unsigned char* filedata;
	};

	class BindableFolder : public std::set<BinaryFile> {
	public:
		BindableFolder();
		BindableFolder(const std::string& bindPrefix);
		bool IsBindable() const;
		const std::string& GetBindPrefix() const;
		void SetBindPrefix(const std::string& prefix);
		void Bind();
		void Unbind();
	private:
		std::string bindPrefix;
	};
}