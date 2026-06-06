#include "m28592/m28592.h"
QVector<double> m28592::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
