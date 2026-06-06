#include "m26132/m26132.h"
QVector<double> m26132::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
