#include "q8516/m8516.h"
QVector<double> m8516::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
