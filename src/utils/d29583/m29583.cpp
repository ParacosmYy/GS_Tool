#include "d29583/m29583.h"
QVector<double> m29583::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
