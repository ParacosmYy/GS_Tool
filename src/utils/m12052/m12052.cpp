#include "m12052/m12052.h"
QVector<double> m12052::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
