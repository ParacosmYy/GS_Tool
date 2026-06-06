#include "b24741/m24741.h"
QVector<double> m24741::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
