#include "m18472/m18472.h"
QVector<double> m18472::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
