#include "p7875/m7875.h"
QVector<double> m7875::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
