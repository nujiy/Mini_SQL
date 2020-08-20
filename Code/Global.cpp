#include "Global.h"

std::string IdxToDbf(std::string idx_name)
{
	std::string dbf_name(idx_name);

	int i = idx_name.size() - 1;
	while (idx_name[i] != '.')
		i--;
	if (i < 0) throw SQLError::FILENAME_CONVERT_ERROR();
	idx_name[i + 1] = 'd';
	idx_name[i + 2] = 'b';
	idx_name[i + 3] = 'f';
	idx_name[i + 4] = '\0';

	return dbf_name;
}

std::string DbfToIdx(std::string dbf_name)
{
	std::string idx_name(dbf_name);

	int i = idx_name.size() - 1;
	while (idx_name[i] != '.')
		i--;
	if (i < 0) throw SQLError::FILENAME_CONVERT_ERROR();
	idx_name[i + 1] = 'i';
	idx_name[i + 2] = 'd';
	idx_name[i + 3] = 'x';
	idx_name[i + 4] = '\0';

	return idx_name;
}

int StrToInt(std::string str)
{
	int x = 0;
	for (int i = 0; i < str.size(); i++)
	{
		x = x * 10 + str[i] - '0';
	}
	return x;
}

std::string StrToLower(std::string str)
{
	for (auto &c : str)
		tolower(c);
	return str;
}

std::string IntToStr3(unsigned int x)
{
	std::string str = "000";
	str[2] = (x % 10) + '0';
	str[0] = (x / 100) + '0';
	str[1] = (x % 100) / 10 + '0';
	return str;
}


void SQLTimer::Start()
{
	start_t = steady_clock::now();
}

void SQLTimer::Stop()
{
	stop_t = steady_clock::now();
}
double SQLTimer::TimeSpan()
{
	time_span = duration_cast<duration<double>>(stop_t - start_t);
	return time_span.count();
}

void SQLTimer::PrintTimeSpan()
{
	std::cout << std::setiosflags(std::ios::fixed) << std::setprecision(precision) << TimeSpan() << " seconds ";
}

SQLTimer& GetTimer()
{
	static SQLTimer timer;
	return timer;
}