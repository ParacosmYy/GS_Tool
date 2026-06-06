#include "b21101/m21101.h"
QVector<double> m21101::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
