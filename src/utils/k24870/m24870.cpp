#include "k24870/m24870.h"
QVector<double> m24870::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
