#include "m22712/m22712.h"
QVector<double> m22712::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
