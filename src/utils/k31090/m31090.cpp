#include "k31090/m31090.h"
QVector<double> m31090::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
