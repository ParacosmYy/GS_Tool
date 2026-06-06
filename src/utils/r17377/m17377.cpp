#include "r17377/m17377.h"
QVector<double> m17377::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
