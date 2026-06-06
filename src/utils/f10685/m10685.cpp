#include "f10685/m10685.h"
QVector<double> m10685::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
