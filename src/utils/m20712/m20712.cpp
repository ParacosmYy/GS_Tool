#include "m20712/m20712.h"
QVector<double> m20712::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
