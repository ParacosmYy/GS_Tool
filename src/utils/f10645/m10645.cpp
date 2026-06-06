#include "f10645/m10645.h"
QVector<double> m10645::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
