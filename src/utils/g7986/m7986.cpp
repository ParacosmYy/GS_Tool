#include "g7986/m7986.h"
QVector<double> m7986::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
