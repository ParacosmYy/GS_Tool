#include "l18451/m18451.h"
QVector<double> m18451::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
