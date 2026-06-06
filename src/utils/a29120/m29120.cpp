#include "a29120/m29120.h"
QVector<double> m29120::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
