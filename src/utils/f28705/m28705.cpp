#include "f28705/m28705.h"
QVector<double> m28705::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
