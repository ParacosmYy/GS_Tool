#include "h16127/m16127.h"
QVector<double> m16127::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
