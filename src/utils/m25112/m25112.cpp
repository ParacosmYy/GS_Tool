#include "m25112/m25112.h"
QVector<double> m25112::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
