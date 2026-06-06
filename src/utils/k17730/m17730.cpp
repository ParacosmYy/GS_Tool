#include "k17730/m17730.h"
QVector<double> m17730::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
