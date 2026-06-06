#include "g32206/m32206.h"
QVector<double> m32206::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
