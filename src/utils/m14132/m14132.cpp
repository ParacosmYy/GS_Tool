#include "m14132/m14132.h"
QVector<double> m14132::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
