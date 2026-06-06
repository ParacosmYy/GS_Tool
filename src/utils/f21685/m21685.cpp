#include "f21685/m21685.h"
QVector<double> m21685::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
