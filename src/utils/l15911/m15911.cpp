#include "l15911/m15911.h"
QVector<double> m15911::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
