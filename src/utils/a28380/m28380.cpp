#include "a28380/m28380.h"
QVector<double> m28380::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
