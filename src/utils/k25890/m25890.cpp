#include "k25890/m25890.h"
QVector<double> m25890::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
