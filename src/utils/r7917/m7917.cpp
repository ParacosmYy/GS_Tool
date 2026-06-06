#include "r7917/m7917.h"
QVector<double> m7917::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
