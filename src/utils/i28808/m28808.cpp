#include "i28808/m28808.h"
QVector<double> m28808::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
