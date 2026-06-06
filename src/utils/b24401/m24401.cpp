#include "b24401/m24401.h"
QVector<double> m24401::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
