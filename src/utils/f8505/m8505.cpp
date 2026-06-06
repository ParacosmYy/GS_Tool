#include "f8505/m8505.h"
QVector<double> m8505::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
