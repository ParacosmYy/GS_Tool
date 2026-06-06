#include "m12132/m12132.h"
QVector<double> m12132::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
