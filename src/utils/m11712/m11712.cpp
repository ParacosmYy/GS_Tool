#include "m11712/m11712.h"
QVector<double> m11712::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
