#include "h16907/m16907.h"
QVector<double> m16907::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
