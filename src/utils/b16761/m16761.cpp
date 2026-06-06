#include "b16761/m16761.h"
QVector<double> m16761::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
