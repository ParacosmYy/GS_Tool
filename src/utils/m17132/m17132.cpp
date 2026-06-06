#include "m17132/m17132.h"
QVector<double> m17132::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
