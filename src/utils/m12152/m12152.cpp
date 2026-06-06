#include "m12152/m12152.h"
QVector<double> m12152::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
