#include "f25085/m25085.h"
QVector<double> m25085::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
