#include "a24280/m24280.h"
QVector<double> m24280::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
