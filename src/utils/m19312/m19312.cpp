#include "m19312/m19312.h"
QVector<double> m19312::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
