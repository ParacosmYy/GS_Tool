#include "n7813/m7813.h"
QVector<double> m7813::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
