#include "c18002/m18002.h"
QVector<double> m18002::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
