#include "j7829/m7829.h"
QVector<double> m7829::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
