#include "m18792/m18792.h"
QVector<double> m18792::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
