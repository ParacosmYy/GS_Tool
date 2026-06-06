#include "m18232/m18232.h"
QVector<double> m18232::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
