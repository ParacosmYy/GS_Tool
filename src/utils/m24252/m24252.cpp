#include "m24252/m24252.h"
QVector<double> m24252::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
