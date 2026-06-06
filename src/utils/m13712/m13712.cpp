#include "m13712/m13712.h"
QVector<double> m13712::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
