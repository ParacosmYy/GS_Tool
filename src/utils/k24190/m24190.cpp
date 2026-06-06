#include "k24190/m24190.h"
QVector<double> m24190::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
