#include "f7925/m7925.h"
QVector<double> m7925::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
