#include "i26708/m26708.h"
QVector<double> m26708::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
