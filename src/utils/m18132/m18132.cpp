#include "m18132/m18132.h"
QVector<double> m18132::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
