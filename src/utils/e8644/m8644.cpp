#include "e8644/m8644.h"
QVector<double> m8644::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
