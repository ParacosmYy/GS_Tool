#include "m25472/m25472.h"
QVector<double> m25472::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
