#include "m25132/m25132.h"
QVector<double> m25132::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
