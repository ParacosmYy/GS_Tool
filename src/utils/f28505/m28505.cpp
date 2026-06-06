#include "f28505/m28505.h"
QVector<double> m28505::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
