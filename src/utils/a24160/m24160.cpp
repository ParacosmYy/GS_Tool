#include "a24160/m24160.h"
QVector<double> m24160::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
