#include "g18686/m18686.h"
QVector<double> m18686::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
