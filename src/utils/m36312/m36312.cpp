#include "m36312/m36312.h"
QVector<double> m36312::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
