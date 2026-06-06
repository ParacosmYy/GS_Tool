#include "b8061/m8061.h"
QVector<double> m8061::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
