#include "m31412/m31412.h"
QVector<double> m31412::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
