#include "k24370/m24370.h"
QVector<double> m24370::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
