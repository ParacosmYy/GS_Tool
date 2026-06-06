#include "b24021/m24021.h"
QVector<double> m24021::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
