#include "l8851/m8851.h"
QVector<double> m8851::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
