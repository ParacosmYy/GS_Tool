#include "f28485/m28485.h"
QVector<double> m28485::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
