#include "e25284/m25284.h"
QVector<double> m25284::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
