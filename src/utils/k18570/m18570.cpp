#include "k18570/m18570.h"
QVector<double> m18570::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
