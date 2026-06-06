#include "f10805/m10805.h"
QVector<double> m10805::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
