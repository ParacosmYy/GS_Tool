#include "n8093/m8093.h"
QVector<double> m8093::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
