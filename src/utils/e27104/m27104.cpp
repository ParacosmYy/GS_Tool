#include "e27104/m27104.h"
QVector<double> m27104::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
