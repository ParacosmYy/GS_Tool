#include "f8245/m8245.h"
QVector<double> m8245::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
