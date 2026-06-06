#include "m37312/m37312.h"
QVector<double> m37312::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
