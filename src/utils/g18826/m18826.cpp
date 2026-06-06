#include "g18826/m18826.h"
QVector<double> m18826::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
