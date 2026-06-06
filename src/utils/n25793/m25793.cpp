#include "n25793/m25793.h"
QVector<double> m25793::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
