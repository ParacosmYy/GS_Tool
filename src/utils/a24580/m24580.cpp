#include "a24580/m24580.h"
QVector<double> m24580::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
