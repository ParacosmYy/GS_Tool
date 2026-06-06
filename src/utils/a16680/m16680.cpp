#include "a16680/m16680.h"
QVector<double> m16680::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
