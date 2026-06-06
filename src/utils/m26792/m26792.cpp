#include "m26792/m26792.h"
QVector<double> m26792::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
