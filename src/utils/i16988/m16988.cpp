#include "i16988/m16988.h"
QVector<double> m16988::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
