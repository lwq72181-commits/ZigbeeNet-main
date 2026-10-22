/*
 * TFileWriter.h
 *
 *  Created on: 2019年1月15日
 *      Author: terry
 */

#ifndef TFILEWRITER_H_
#define TFILEWRITER_H_

#include <stdio.h>
#include <string>

namespace comn
{

class FileWriter
{
public:
    explicit FileWriter(const char* filepath = NULL, const char* mode = "wb"):
        m_file()
    {
        open(filepath, mode);

		if (filepath != NULL)
		{
			m_filepath = filepath;
		}
    }

    explicit FileWriter(const std::string& filepath, const char* mode = "wb"):
        m_file()
    {
        open(filepath.c_str(), mode);
		m_filepath = filepath;
    }


    ~FileWriter()
    {
        close();
    }

    void close()
    {
        if (m_file)
        {
            fclose(m_file);
            m_file = NULL;
        }
    }

	bool isOpen() const
	{
		return (m_file != NULL);
	}

	bool open(const std::string& filepath, const char* mode = "wb")
	{
		return open(filepath.c_str(), mode);
	}

    bool open(const char* filepath, const char* mode = "wb")
    {
        close();

		if (!filepath)
		{
			return false;
		}

#ifdef WIN32
		m_file = _fsopen(filepath, mode, _SH_DENYNO);
#else
        m_file = fopen(filepath, mode);
#endif //

        return (m_file != NULL);
    }

    void write(const void* data, int size)
    {
        if (!m_file)
        {
            return;
        }

        fwrite(data, 1, size, m_file);
    }

	const std::string& getFilePath() const
	{
		return m_filepath;
	}

	void flush()
	{
		if (!m_file)
		{
			return;
		}
		fflush(m_file);
	}

protected:
    FILE*   m_file;
	std::string	m_filepath;

};

}


#endif /* TFILEWRITER_H_ */
