#include "m28752/m28752.h"
QVector<double> m28752::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
