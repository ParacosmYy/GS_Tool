#include "s8138/m8138.h"
QVector<double> m8138::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
