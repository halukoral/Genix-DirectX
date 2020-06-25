#pragma once
#include <exception>
#include <string>

class GenixException : public std::exception
{
public:
	GenixException(int line, const char* file) noexcept
		:line(line),file(file) {}

			const char*	what()				const noexcept override;
	virtual const char* GetType()			const noexcept { return "Genix Exception"; }
					int GetLine()			const noexcept { return line; }
	const std::string&	GetFile()			const noexcept { return file; }
	
		  std::string	GetOriginString()	const noexcept;

private:
			int line;
	std::string file;

protected:
	mutable std::string whatBuffer;
};