#include "m28292/m28292.h"
QVector<double> m28292::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
