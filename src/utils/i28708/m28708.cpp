#include "i28708/m28708.h"
QVector<double> m28708::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
