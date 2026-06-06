#include "m37572/m37572.h"
QVector<double> m37572::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
