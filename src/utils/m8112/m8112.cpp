#include "m8112/m8112.h"
QVector<double> m8112::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
