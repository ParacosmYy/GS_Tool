#include "m25792/m25792.h"
QVector<double> m25792::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
