#include "a10940/m10940.h"
QVector<double> m10940::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
