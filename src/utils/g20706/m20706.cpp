#include "g20706/m20706.h"
QVector<double> m20706::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
