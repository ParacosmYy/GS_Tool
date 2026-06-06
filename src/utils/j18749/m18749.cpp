#include "j18749/m18749.h"
QVector<double> m18749::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
