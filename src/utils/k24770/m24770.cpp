#include "k24770/m24770.h"
QVector<double> m24770::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
