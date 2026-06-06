#include "g24126/m24126.h"
QVector<double> m24126::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
