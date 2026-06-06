#include "b24701/m24701.h"
QVector<double> m24701::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
