#include "i21708/m21708.h"
QVector<double> m21708::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
