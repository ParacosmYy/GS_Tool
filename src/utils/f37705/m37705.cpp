#include "f37705/m37705.h"
QVector<double> m37705::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
