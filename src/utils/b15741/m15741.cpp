#include "b15741/m15741.h"
QVector<double> m15741::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
