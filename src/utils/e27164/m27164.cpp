#include "e27164/m27164.h"
QVector<double> m27164::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
