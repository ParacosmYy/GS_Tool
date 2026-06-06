#include "m28852/m28852.h"
QVector<double> m28852::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
