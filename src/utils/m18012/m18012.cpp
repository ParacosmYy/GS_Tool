#include "m18012/m18012.h"
QVector<double> m18012::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
