#include "b8401/m8401.h"
QVector<double> m8401::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
